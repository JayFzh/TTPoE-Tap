int copy_skb_data(struct sk_buff *skb,int offset) {

    // 确保偏移量合法
    if (offset >= skb->len) {
        printk(KERN_ERR "Offset is beyond skb data length\n");
        return -EINVAL;
    }

    int len = skb->len - offset; // 需要复制的数据长度

    // 创建新的 skb
    struct sk_buff *skb_new = alloc_skb(len+2, GFP_KERNEL);
    if (!skb_new) {
        printk(KERN_ERR "Failed to allocate new skb\n");
        return -ENOMEM;
    }

    // 从原始 skb 复制数据到临时缓冲区
    char *buffer = kmalloc(len+2, GFP_KERNEL);
    if (!buffer) {
        printk(KERN_ERR "Failed to allocate memory\n");
        kfree_skb(skb_new);
        return -ENOMEM;
    }

    int ret = skb_copy_bits(skb, offset, buffer, len);
    if (ret < 0) {
        printk(KERN_ERR "Failed to copy skb data\n");
        kfree(buffer);
        kfree_skb(skb_new);
        return ret;
    }

    // 将复制的数据放入新的 skb
    skb_put_data(skb_new, buffer, len);
    skb_new->dev = tap_dev;
    skb_new->protocol = eth_type_trans(skb_new, tap_dev);
    skb_reset_network_header(skb_new);

    print_skb_raw(skb_new);
    ret = netif_rx(skb_new);
    if (ret != NET_XMIT_SUCCESS) {
        printk(KERN_INFO "Transmission failed: %d\n", ret);
        // 处理失败情况，如释放skb等
    }
    else printk(KERN_INFO "Transmission success\n");


    // 释放临时缓冲区
    // kfree(buffer);

    // skb_new 可以在此之后进行处理

    return 0;
}


// 收包逻辑
int ttp_skb_dequ (void)
{
    struct sk_buff *skb;
    struct ttp_fsm_event *ev;
    struct ttp_pkt_info pif;
    struct ttp_frame_hdr frh;
    u8 mac[ETH_ALEN];

    if (!(skb = skb_dequeue (&ttp_global_root_head.skb_head))) {
        return 0;
    }

    if (tap_dev) {
        if(skb->len > 64) { 
            int offset = 60;
            copy_skb_data(skb,offset);
        } 
    }


    memset (&frh, 0, sizeof (frh));
    memset (&pif, 0, sizeof (pif));

    ttpoe_parse_print (skb, TTP_RX);

    ttp_skb_pars (skb, &frh, &pif);

    if (!TTP_OPCODE_IS_VALID (frh.ttp->conn_opcode)) {
        TTP_LOG ("pre_parser: INVALID opcode:%d\n", frh.ttp->conn_opcode);
        ttp_skb_drop (skb);
        return 0;
    }

    if (!TTP_VC_ID__IS_VALID (frh.ttp->conn_vc)) {
        TTP_LOG ("pre_parser: INVALID vc-id:%d\n", frh.ttp->conn_vc);
        ttp_skb_drop (skb);
        return 0;
    }

    if (!ttp_evt_pget (&ev)) {
        ttp_skb_drop (skb);
        return 0;
    }

    atomic_inc (&ttp_stats.frm_ct);
    atomic_inc (&ttp_stats.skb_ct);

    ev->rsk = skb;
    ev->psi = pif;
    ev->evt = TTP_OPCODE_TO_EVENT (frh.ttp->conn_opcode);

    if (frh.tth->l3gw) {
        if (!is_valid_ether_addr (ttp_debug_gwmac.mac)) {
            ether_addr_copy (ttp_debug_gwmac.mac, frh.eth->h_source); /* learn gwmac */
        }

        ttp_mac_from_shim (mac, frh.tsh->src_node);
        ev->kid = ttp_tag_key_make (mac, frh.ttp->conn_vc, 1);
    }
    else {
        ev->kid = ttp_tag_key_make (frh.eth->h_source, frh.ttp->conn_vc, 0);
    }

    ttp_evt_enqu (ev);
    TTP_EVLOG (ev, TTP_LG__PKT_RX, frh.ttp->conn_opcode);

    return 1;
}