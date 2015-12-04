#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#include "kbuf_ioctl.h"

#define DEV_NAME	"kbuf"
#define BUF_SZ		1024
#define NUM_DEVICES	5

int dev_major;

typedef struct {
    int opened;
    int closed;
    int readed;
    int writen;
    int lseek;
    int poll;
    int ioctl;
    /* file pointer and buffer */
    int fp;
    char buff[BUF_SZ];
} kbuf_stat;

kbuf_stat *dev = NULL;

void show_proc_info(int pid)
{
	printk(KERN_INFO "ToDo: show info about pid=%d\n", pid);
}

void show_stat(kbuf_stat *stat, int dev_minor)
{
    if(stat) {
	printk(KERN_INFO "STATISTICS for kbuf %d %d:\n", 
	    dev_major, dev_minor);
	printk(KERN_INFO "opened: %d\n", stat->opened);
	printk(KERN_INFO "closed: %d\n", stat->closed);
	printk(KERN_INFO "readed: %d\n", stat->readed);
	printk(KERN_INFO "writen: %d\n", stat->writen);
	printk(KERN_INFO "lseek:  %d\n", stat->lseek);
	printk(KERN_INFO "poll:   %d\n", stat->poll);
	printk(KERN_INFO "ioctl:  %d\n", stat->ioctl);
	printk(KERN_INFO "FP at:  %d\n", stat->fp);
    } else {
	printk(KERN_ERR "Sorry, no statistics present.\n");
    }
}

int kbuf_open(struct inode *i, struct file *f)
{
    kbuf_stat *stat;
    int idev = iminor(i);

    if(idev < NUM_DEVICES && dev) {
	stat = &dev[idev];
    } else
	return -ENODEV;

    stat->opened++;
    stat->fp = 0; /* set file pointer to 0 on open */

    printk(KERN_INFO "kbuf %d open\n", idev);
    return 0;
}

int kbuf_release(struct inode *i, struct file *f)
{
    kbuf_stat *stat;
    int idev = iminor(i);

    if(idev < NUM_DEVICES && dev) {
	stat = &dev[idev];
    } else
	return -ENODEV;

    stat->closed++;

    printk(KERN_INFO "kbuf %d closed\n", idev);
    return 0;
}

ssize_t kbuf_read(struct file *f, char __user *ubuff, size_t sz, loff_t *ofs)
{
    kbuf_stat *stat;
    int idev = iminor(f->f_inode);
    size_t real_rd = 0;

    if(idev < NUM_DEVICES && dev) {
	stat = &dev[idev];
    } else
	return -ENODEV;

    stat->readed++;

    if(*ofs > BUF_SZ) {
	printk(KERN_ERR "kbuf %d Offset to long for read!\n", idev);
	return -EINVAL;
    };

    /* calculate real read size */
    real_rd = 
	((*ofs + stat->fp + sz) > BUF_SZ) ? (BUF_SZ - stat->fp - *ofs) : sz;

    if(copy_to_user(ubuff, (void *)(stat->buff + *ofs), real_rd)) {
	printk(KERN_ERR "kbuf %d copy_to_user() failed\n", idev);
	return -EFAULT;
    }

    printk(KERN_INFO "kbuf %d Module read return %zd\n", idev, real_rd);
    stat->fp += real_rd; /* update fp */

    return real_rd;
}

ssize_t kbuf_write(struct file *f, const char __user *ubuff, size_t sz, loff_t *ofs)
{
    kbuf_stat *stat;
    int idev = iminor(f->f_inode);
    size_t real_wr = 0;

    if(idev < NUM_DEVICES && dev) {
	stat = &dev[idev];
    } else
	return -ENODEV;

    stat->writen++;

    if(*ofs > BUF_SZ) {
	printk(KERN_ERR "kbuf %d Offset to long for write!\n", idev);
	return -EINVAL;
    };

    /* calculate real write size */
    real_wr =
	((*ofs + stat->fp + sz) > BUF_SZ) ? (BUF_SZ - stat->fp - *ofs) : sz;

    /* real copy */
    if(copy_from_user((void *)(stat->buff + *ofs),ubuff, real_wr)) {
	printk(KERN_ERR "kbuf %d copy_from_user() failed\n", idev);
	return -EFAULT;
    }

    printk(KERN_INFO "kbuf %d Module write return %zd\n", idev, real_wr);
    stat->fp += real_wr;

    return real_wr;
}

long kbuf_ioctl(struct file *f, unsigned int cmd, unsigned long arg)
{
    kbuf_stat *stat;
    int idev = iminor(f->f_inode);
    int ret = -1;
    int pid;

    if(idev < NUM_DEVICES && dev) {
	stat = &dev[idev];
    } else
	return -ENODEV;

    stat->ioctl++;

    switch (cmd) {
    case IOKBUF_STAT:
	show_stat(stat, idev);
	ret = 0;
	break;
    case IOKBUF_PROCSTAT:
	pid = arg;
	show_proc_info(pid);
	ret = 0;
	break;
    default:
	printk(KERN_ERR "kbuf %d Unsupported IOCTL cmd %d\n", idev, cmd);
    }

    return ret;
}

loff_t kbuf_llseek(struct file *f, loff_t ofs, int cmd)
{
    kbuf_stat *stat;
    int idev = iminor(f->f_inode);
    int ret = 0;

    if(idev < NUM_DEVICES && dev) {
	stat = &dev[idev];
    } else
	return -ENODEV;

    stat->lseek++;

    switch(cmd){
    case SEEK_SET: /* absolute */
	if(ofs > BUF_SZ || ofs > 0) {
	    printk(KERN_ERR "kbuf %d To large offset!\n", idev);
	    ret = -EOVERFLOW;
	} else {
	    printk(KERN_ERR "kbuf %d seek ok!\n", idev);
	    stat->fp = ofs;
	}
	break;
    case SEEK_CUR: /* relative */
	ofs += stat->fp;
	if (ofs < 0 || ofs > BUF_SZ) {
	    printk(KERN_ERR "kbuf %d To large offset!\n", idev);
	    ret = -EOVERFLOW;
	} else {
	    printk(KERN_ERR "kbuf %d seek ok!\n", idev);
	    stat->fp = ofs;
	}
	break;
    case SEEK_END: /* EOF + ofs - not supported */
	printk(KERN_ERR "kbuf %d SEEK_END unsupported!\n", idev);
	ret = -EOVERFLOW;
	break;
    default:
	ret = -EINVAL;
	printk(KERN_ERR "kbuf %d Unsupported lseek params!\n", idev);
    }

    ret = (ret < 0) ? ret : stat->fp;
    printk(KERN_INFO "kbuf %d lseek return %d\n", idev, ret);
    return ret;
}


unsigned int kbuf_poll(struct file *f, struct poll_table_struct *pt)
{
    kbuf_stat *stat;
    int idev = iminor(f->f_inode);

    if(idev < NUM_DEVICES && dev) {
	stat = &dev[idev];
    } else
	return -ENODEV;

    stat->poll++;

    printk(KERN_INFO "kbuf %d poll\n", idev);
    return 0;
}

struct file_operations kbuf_fops = {
    .owner = THIS_MODULE,
    .open = kbuf_open,
    .release = kbuf_release,
    .read = kbuf_read,
    .write = kbuf_write,
    .unlocked_ioctl = kbuf_ioctl,
    .poll = kbuf_poll,
    .llseek = kbuf_llseek,
};

int register_kbuf_device(void)
{
    /* Allocate device private data and buffers */
    dev = (kbuf_stat*)kzalloc(sizeof(kbuf_stat) * NUM_DEVICES, GFP_KERNEL);
    if(dev == NULL) {
	printk(KERN_ERR "Allocate KBUF devices private data failed!\n");
	return -ENOMEM;
    }

    /* Register devices */
    dev_major = register_chrdev(0,DEV_NAME,&kbuf_fops);
    if( dev_major < 0 ){
	printk(KERN_ERR "Register chardev failed(%d)\n", dev_major);
	return dev_major;
    }

    printk(KERN_INFO "device registered (major = %d, minor = 0..%d)\n",
	dev_major, NUM_DEVICES);
    return 0;
}

void unregister_kbuf_device(void)
{
    kfree(dev);

    unregister_chrdev(dev_major, DEV_NAME);
    printk(KERN_INFO "device unregistered\n");
}

int init_module(void)
{
    printk(KERN_INFO "Module init\n");
    return register_kbuf_device();
}

void cleanup_module(void)
{
    unregister_kbuf_device();
    printk(KERN_INFO "Module exit\n");
}
