#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/poll.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/pid.h>
#include <linux/interrupt.h>
#include <linux/uaccess.h>

#include "kbuf_ioctl.h"

#define DEV_NAME	"kbuf"
#define NUM_DEVICES	5
#define BUF_SZ		128
/* dinamic major allocate */
#define MAJOR_BASE	0
/* minors from MINOR_BASE to MINOR_BASE + NUM_DEVICES */
#define MINOR_BASE	0

#define MONITOR_IRQ_NUMBER	19

int dev_major;
volatile uint32_t dev_irq = 0;

typedef struct {
    int opened;
    int closed;
    int readed;
    int writen;
    int lseek;
    int poll;
    int ioctl;
    /* file buffer */
    char buff[BUF_SZ];
} kbuf_stat;

kbuf_stat *dev = NULL;

void show_proc_info(int p)
{
    struct task_struct *task = NULL;
    struct pid *pid = NULL;

    pid = find_vpid(p);
    if(!pid)
	goto err;

    task = get_pid_task(pid, PIDTYPE_PID);
    if(!task)
	goto err;

    printk(KERN_INFO "Show info about pid=%d\n", p);
    printk(KERN_INFO "\tname: %s\n", task->comm);
    return;

err:
	printk(KERN_ERR "Not found pid=%d\n", p);

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
	print_hex_dump(KERN_INFO, "kbuf: ", DUMP_PREFIX_OFFSET,
		16, 1, stat->buff, BUF_SZ, true);
    } else {
	printk(KERN_ERR "Sorry, no statistics present.\n");
    }
}

int kbuf_open(struct inode *i, struct file *f)
{
    kbuf_stat *stat;
    int idev = iminor(i) - MINOR_BASE;

    if(idev < NUM_DEVICES && dev && idev > -1) {
	stat = &dev[idev];
    } else
	return -ENODEV;

    stat->opened++;
    f->f_pos = 0; /* set file pointer to 0 on open */

    printk(KERN_INFO "kbuf %d open\n", idev);
    return 0;
}

int kbuf_release(struct inode *i, struct file *f)
{
    kbuf_stat *stat;
    int idev = iminor(i) - MINOR_BASE;

    if(idev < NUM_DEVICES && dev && idev > -1) {
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
    int idev = iminor(f->f_inode) - MINOR_BASE;
    size_t real_rd = 0;

    if(idev < NUM_DEVICES && dev && idev > -1) {
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
	((*ofs + f->f_pos + sz) > BUF_SZ) ? (BUF_SZ - f->f_pos - *ofs) : sz;

    if(copy_to_user(ubuff, (void *)(stat->buff + *ofs), real_rd)) {
	printk(KERN_ERR "kbuf %d copy_to_user() failed\n", idev);
	return -EFAULT;
    }

    printk(KERN_INFO "kbuf %d Module read return %zd\n", idev, real_rd);
    f->f_pos += real_rd; /* update fp */

    return real_rd;
}

ssize_t kbuf_write(struct file *f, const char __user *ubuff, size_t sz, loff_t *ofs)
{
    kbuf_stat *stat;
    int idev = iminor(f->f_inode) - MINOR_BASE;
    size_t real_wr = 0;

    if(idev < NUM_DEVICES && dev && idev > -1) {
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
	((*ofs + f->f_pos + sz) > BUF_SZ) ? (BUF_SZ - f->f_pos - *ofs) : sz;

    /* real copy */
    if(copy_from_user((void *)(stat->buff + *ofs),ubuff, real_wr)) {
	printk(KERN_ERR "kbuf %d copy_from_user() failed\n", idev);
	return -EFAULT;
    }

    printk(KERN_INFO "kbuf %d Module write return %zd\n", idev, real_wr);
    f->f_pos += real_wr;

    return real_wr;
}

long kbuf_ioctl(struct file *f, unsigned int cmd, unsigned long arg)
{
    kbuf_stat *stat;
    int idev = iminor(f->f_inode) - MINOR_BASE;
    int ret = -1;
    int pid;

    if(idev < NUM_DEVICES && dev && idev > -1) {
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
    case IOKBUF_DEVSTAT:
	ret = copy_to_user((void __user *)arg, (void *)&dev_irq, sizeof(uint32_t));
	printk(KERN_INFO "DEVSTAT = %d (%d bytes)\n", dev_irq, ret);
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
    int idev = iminor(f->f_inode) - MINOR_BASE;
    int ret = 0;

    if(idev < NUM_DEVICES && dev && idev > -1) {
	stat = &dev[idev];
    } else
	return -ENODEV;

    stat->lseek++;

    switch(cmd){
    case SEEK_SET: /* absolute */
	if(ofs > (BUF_SZ-1) || ofs < 0) {
	    printk(KERN_ERR "kbuf %d SEEK_SET : Invalid offset!\n", idev);
	    ret = -EOVERFLOW;
	} else {
	    printk(KERN_ERR "kbuf %d SEEK_SET seek ok!\n", idev);
	    f->f_pos = ofs;
	}
	break;
    case SEEK_CUR: /* relative */
	ofs += f->f_pos;
	if (ofs < 0 || ofs > (BUF_SZ-1)) {
	    printk(KERN_ERR "kbuf %d SEEK_CUR : Invalid offset!\n", idev);
	    ret = -EOVERFLOW;
	} else {
	    printk(KERN_ERR "kbuf %d SEEK_CUR seek ok!\n", idev);
	    f->f_pos = ofs;
	}
	break;
    case SEEK_END:
	if(ofs > 0 || ofs < (0-BUF_SZ+1)) {
	    printk(KERN_ERR "kbuf %d SEEK_END : Invalid offset!\n", idev);
	    ret = -EOVERFLOW;
	} else {
	    f->f_pos = ofs + BUF_SZ - 1;
	}
	break;
    default:
	ret = -EINVAL;
	printk(KERN_ERR "kbuf %d Unsupported lseek whence!\n", idev);
    }

    ret = (ret < 0) ? ret : f->f_pos;
    printk(KERN_INFO "kbuf %d lseek return %d\n", idev, ret);
    return ret;
}


unsigned int kbuf_poll(struct file *f, struct poll_table_struct *pt)
{
    int ret;
    kbuf_stat *stat;
    int idev = iminor(f->f_inode) - MINOR_BASE;

    if(idev < NUM_DEVICES && dev && idev > -1) {
	stat = &dev[idev];
    } else
	return -ENODEV;

    stat->poll++;

    ret = 0;
    /* if fp != EOF can read and write */
    if(f->f_pos < (BUF_SZ -1))
	ret = (POLLIN | POLLOUT);

    printk(KERN_INFO "kbuf %d poll return %d(fp at %lld)\n",
	idev, ret, f->f_pos);

    return ret;
}

irqreturn_t handler(int nr, void* count)
{
    WARN_ON(count != &dev_irq);

    (*(volatile uint32_t*)count)++;
    return IRQ_NONE;
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

int probe_kbuf_device(void)
{
    /* Allocate device private data and buffers */
    dev = (kbuf_stat*)kzalloc(sizeof(kbuf_stat) * NUM_DEVICES, GFP_KERNEL);
    if(dev == NULL) {
	printk(KERN_ERR "Allocate KBUF devices private data failed!\n");
	return -ENOMEM;
    }

    /* Register devices */
    dev_major = register_chrdev(MAJOR_BASE,DEV_NAME,&kbuf_fops);
    if( dev_major < 0 ){
	printk(KERN_ERR "Register chardev failed(%d)\n", dev_major);
	return dev_major;
    }

    /* irq */
    if(request_irq(MONITOR_IRQ_NUMBER, handler,
		IRQF_SHARED, "name", (void *)&dev_irq)) {
	printk(KERN_ERR "request irq failed\n");
	return -EBADF;
    }
    printk(KERN_INFO "IRQ registered\n");

    /* MAJOR_BASE not 0 - no dinamic major allocate */
    if(dev_major == 0) dev_major = MAJOR_BASE;

    printk(KERN_INFO "kbuf devices registered (major = %d, minor = %d..%d)\n",
	dev_major, MINOR_BASE, MINOR_BASE + NUM_DEVICES);
    return 0;
}

void remove_kbuf_device(void)
{
    /* irq */
    synchronize_irq(MONITOR_IRQ_NUMBER);
    free_irq(MONITOR_IRQ_NUMBER, (void *)&dev_irq);

    /* dev */
    unregister_chrdev(dev_major, DEV_NAME);

    /* private */
    kfree(dev);
    printk(KERN_INFO "kbuf devices unregistered\n");
}

int init_module(void)
{
    printk(KERN_INFO "kbuf module init\n");
    return probe_kbuf_device();
}

void cleanup_module(void)
{
    remove_kbuf_device();
    printk(KERN_INFO "kbuf module exit\n");
}

MODULE_LICENSE("GPL");
