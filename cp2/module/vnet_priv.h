#ifndef _INCLUDE_KBUF_IOCTL_H_
#define _INCLUDE_KBUF_IOCTL_H_

#define IOKBUF_STAT	0x1001
#define IOKBUF_PROCSTAT	0x1002
#define IOKBUF_DEVSTAT	0x1003

#define IOVNET_DEVSTAT	0x2003

struct vnet_priv {
    struct net_device *dev;
    /*
     * Private block
     */
    unsigned long open;
    unsigned long stop;
    unsigned long xmit;
    unsigned long count;
    unsigned long timeouts;
    unsigned char last_bytes[256];
};

#endif