#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#include "kbuf_ioctl.h"

#define DEV_NAME	"kbuf"
#define BUF_SZ		1024

int dev_major;
char* buff = NULL;

typedef struct {
    int opened;
    int closed;
    int readed;
    int writen;
    int lseek;
    int poll;
    int ioctl;
    int fp; /* file pointer */
} kbuf_stat;

kbuf_stat *stat = NULL;

void show_stat(kbuf_stat *stat)
{
    if(stat) {
	printk(KERN_INFO "STATISTICS:\n");
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
    if(stat) stat->opened++;

    stat->fp = 0; /* set file pointer to 0 on open */
    printk(KERN_INFO "kbuf open\n");
    return 0;
}

int kbuf_release(struct inode *i, struct file *f)
{
    if(stat) stat->closed++;

    printk(KERN_INFO "kbuf close\n");
    return 0;
}

ssize_t kbuf_read(struct file *f, char __user *ubuff, size_t sz, loff_t *ofs)
{
    size_t real_rd = 0;

    if(stat) stat->readed++;

    if(!buff) return -EINVAL;

    if(*ofs > BUF_SZ) {
	printk(KERN_ERR "Offset to long for read!\n");
	return -EINVAL;
    };

    /* calculate real read size */
    real_rd = 
	((*ofs + stat->fp + sz) > BUF_SZ) ? (BUF_SZ - stat->fp - *ofs) : sz;

    if(copy_to_user(ubuff, (void *)(buff + *ofs), real_rd)) {
	printk(KERN_ERR "copy_to_user() failed\n");
	return -EFAULT;
    }

    printk(KERN_INFO "Module read return %zd\n", real_rd);
    stat->fp += real_rd; /* update fp */

    return real_rd;
}

ssize_t kbuf_write(struct file *f, const char __user *ubuff, size_t sz, loff_t *ofs)
{
    size_t real_wr = 0;

    if(stat) stat->writen++;

    if(!buff) return -EINVAL;

    if(*ofs > BUF_SZ) {
	printk(KERN_ERR "Offset to long for write!\n");
	return -EINVAL;
    };

    /* calculate real write size */
    real_wr =
	((*ofs + stat->fp + sz) > BUF_SZ) ? (BUF_SZ - stat->fp - *ofs) : sz;

    /* real copy */
    if(copy_from_user((void *)(buff + *ofs),ubuff, real_wr)) {
	printk(KERN_ERR "copy_from_user() failed\n");
	return -EFAULT;
    }

    printk(KERN_INFO "Module write return %zd\n", real_wr);
    stat->fp += real_wr;

    return real_wr;
}

long kbuf_ioctl(struct file *f, unsigned int cmd, unsigned long arg)
{
    int ret = -1;
    int pid;

    if(stat) stat->ioctl++;
    printk(KERN_INFO "Module ioctl\n");

    switch (cmd) {
    case IOKBUF_STAT:
	show_stat(stat);
	ret = 0;
	break;
    case IOKBUF_PROCSTAT:
	pid = arg;
	printk(KERN_INFO "ToDo: show info about pid=%d\n", pid);
	ret = 0;
	break;
    default:
	printk(KERN_ERR "Unsupported IOCTL cmd %d\n", cmd);
    }

    return ret;
}

loff_t kbuf_llseek(struct file *f, loff_t ofs, int cmd)
{
    int ret = 0;

    if(stat) stat->lseek++;

    printk(KERN_INFO "kbuf lseek\n");

    switch(cmd){
    case SEEK_SET: /* absolute */
	if(ofs > BUF_SZ || ofs > 0) {
	    printk(KERN_ERR "To large offset!\n");
	    ret = -EOVERFLOW;
	} else {
	    printk(KERN_ERR "seek ok!\n");
	    stat->fp = ofs;
	}
	break;
    case SEEK_CUR: /* relative */
	ofs += stat->fp;
	if (ofs < 0 || ofs > BUF_SZ) {
	    printk(KERN_ERR "To large offset!\n");
	    ret = -EOVERFLOW;
	} else {
	    printk(KERN_ERR "seek ok!\n");
	    stat->fp = ofs;
	}
	break;
    case SEEK_END: /* EOF + ofs - not supported */
	printk(KERN_ERR "SEEK_END unsupported!\n");
	ret = -EOVERFLOW;
	break;
    default:
	ret = -EINVAL;
	printk(KERN_ERR "Unsupported lseek params!\n");
    }

    printk(KERN_INFO "kbuf lseek return %d\n", ret);
    return (ret < 0) ? ret : stat->fp;
}


unsigned int kbuf_poll(struct file *f, struct poll_table_struct *pt)
{
    if(stat) stat->poll++;
    printk(KERN_INFO "Module poll\n");
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
    buff = kzalloc(BUF_SZ, GFP_KERNEL);
    if(buff == NULL) {
	printk(KERN_ERR "Buffer allocate failed!\n");
	return -ENOMEM;
    }

    stat = (kbuf_stat*)kzalloc(sizeof(kbuf_stat), GFP_KERNEL);
    if(stat == NULL) {
	printk(KERN_ERR "Statistics allocate failed!\n");
	return -ENOMEM;
    }

    stat->fp = 0; /* initial set file pointer */

    dev_major = register_chrdev(0,DEV_NAME,&kbuf_fops);
    if( dev_major < 0 ){
	printk(KERN_ERR "Register chardev failed(%d)\n", dev_major);
	return dev_major;
    }
    printk(KERN_INFO "device registered (major = %d)\n", dev_major);
    return 0;
}

void unregister_kbuf_device(void)
{
    show_stat(stat);

    kfree(stat);
    kfree(buff);

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
