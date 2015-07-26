
#include <bsp.h>
#include <rtems/rtems_bsdnet.h>

extern void rtems_bsdnet_loopattach();
static struct rtems_bsdnet_ifconfig loopback_config = {
    "lo0",                          /* name */
    (int (*)(struct rtems_bsdnet_ifconfig *, int))rtems_bsdnet_loopattach, /* attach function */
    NULL,                           /* link to next interface */
    "127.0.0.1",                    /* IP address */
    "255.0.0.0",                    /* IP net mask */
};
#define NET_PREV &loopback_config

static struct rtems_bsdnet_ifconfig bsp_driver_config = {
    RTEMS_BSP_NETWORK_DRIVER_NAME,      /* name */
    RTEMS_BSP_NETWORK_DRIVER_ATTACH,    /* attach function */
    NET_PREV,                   /* link to next interface */
};
#undef NET_PREV
#define NET_PREV &bsp_driver_config

#ifdef BSP_pc386

int
rtems_ne_driver_attach (struct rtems_bsdnet_ifconfig *config, int attach);
static struct rtems_bsdnet_ifconfig ne_driver_config = {
    "ne1",      /* name */
    rtems_ne_driver_attach,    /* attach function */
    NET_PREV,                   /* link to next interface */
};
#undef NET_PREV
#define NET_PREV &ne_driver_config

#endif

#define RTEMS_NETWORK_CONFIG_MBUF_SPACE 180
#define RTEMS_NETWORK_CONFIG_CLUSTER_SPACE 350

struct rtems_bsdnet_config rtems_bsdnet_config = {
    NET_PREV,
    rtems_bsdnet_do_bootp,
    10,
    RTEMS_NETWORK_CONFIG_MBUF_SPACE*1024,
    RTEMS_NETWORK_CONFIG_CLUSTER_SPACE*1024,
    NULL,
    "local",
};
