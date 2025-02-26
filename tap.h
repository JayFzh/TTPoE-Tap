#ifndef TAP_H
#define TAP_H

#include <linux/if_tun.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>

int tap_init(void);
void tap_exit(void);

extern struct net_device *tap_dev;

#endif 