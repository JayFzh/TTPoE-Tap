#include <linux/if_tun.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/string.h> 

#include "tap.h"
#include "ttpoe.h"
#include "fsm.h"
#include "tags.h"
#include "noc.h"
#include "print.h"

struct net_device *tap_dev;

static const struct net_device_ops tap_netdev_ops;


static netdev_tx_t my_tap_start_xmit(struct sk_buff *skb, struct net_device *dev) {
    // 处理接收到的数据包
    printk(KERN_INFO "Packet received: %d bytes\n", skb->len);
    if(skb->len != 70){
        int ret;
        unsigned int payload_len;
        struct ttp_frame_hdr frh;
        struct sk_buff *new_skb;
        u8 *buf;
        payload_len = skb->len;
        buf = ttp_skb_aloc (&new_skb, payload_len);
        // size_t buf_len = sizeof(buf);
        // printk(KERN_INFO "Buffer length: %zu bytes\n", buf_len);
        ttp_skb_pars (new_skb, &frh, NULL);
        memcpy(frh.noc,skb->data,skb->len);

        // frh.noc

        
        ret = ttpoe_noc_debug_tx (buf, new_skb, payload_len, TTP_EV__TXQ__TTP_PAYLOAD,
                                    &ttp_debug_target);

        dev_kfree_skb(skb); // 释放内核中的数据包
    }

    return NETDEV_TX_OK;
}

static int tap_set_mac_address(struct net_device *dev, void *addr) {
    struct sockaddr *sock_addr = addr;

    if (!is_valid_ether_addr(sock_addr->sa_data))
        return -EADDRNOTAVAIL;

    memcpy((u8 *)dev->dev_addr, sock_addr->sa_data, dev->addr_len);
    return 0;
}


static void tap_setup(struct net_device *dev) {
    dev->netdev_ops = &tap_netdev_ops;  // 设置网络设备操作函数集
    ether_setup(dev);                   // 以太网设备初始化
    dev->flags |= IFF_NOARP;
    // dev->features |= NETIF_F_IP_CSUM;
}

int tap_init(void) {
    tap_dev = alloc_netdev(0, "tap%d", NET_NAME_UNKNOWN, tap_setup);
    if (!tap_dev)
        return -ENOMEM;

    if (register_netdev(tap_dev)) {
        free_netdev(tap_dev);
        return -ENODEV;
    }

    return 0;
}

void tap_exit(void) {
    unregister_netdev(tap_dev);
    free_netdev(tap_dev);
}

static const struct net_device_ops tap_netdev_ops = {
    // .ndo_open = my_tap_open,
    // .ndo_stop = my_tap_stop,
    .ndo_start_xmit = my_tap_start_xmit,
    .ndo_set_mac_address = tap_set_mac_address,
};

// module_init(tap_init);
// module_exit(tap_exit);