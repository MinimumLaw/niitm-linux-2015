#include <linux/module.h>
#include <linux/kernel.h>

#include <linux/slab.h>
#include <linux/fs.h>

#include <linux/uaccess.h>

#define DEV_NAME	"kbuf"
#define BUF_SZ		1024

int dev_major;
char* buff = NULL;

typedef struct {
    int opened;
    int closed;
    int readed;
    int	writen;
    int	poll;
    int ioctl;
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
	printk(KERN_INFO "poll:   %d\n", stat->poll);
	printk(KERN_INFO "ioctl:  %d\n", stat->ioctl);
    } else {
	printk(KERN_ERR "Sorry, no statistics present.\n");
    }
}

int mod_open(struct inode *i, struct file *f)
{
    if(stat) stat->opened++;
    printk(KERN_INFO "Module open\n");
    return 0;
}

int mod_release(struct inode *i, struct file *f)
{
    if(stat) stat->closed++;
    printk(KERN_INFO "Module release\n");
    return 0;
}

ssize_t mod_read(struct file *f, char __user *ubuff, size_t sz, loff_t *ofs)
{
    size_t real_rd = 0;

    if(stat) stat->readed++;

    if(!buff) return -EINVAL;

    printk(KERN_INFO "Module read return %d\n", real_rd);
    return real_rd;
}

ssize_t mod_write(struct file *f, const char __user *ubuff, size_t sz, loff_t *ofs)
{
    size_t real_wr = 0;

    if(stat) stat->writen++;

    if(!buff) return -EINVAL;

    if(*ofs > BUF_SZ) {
	printk(KERN_ERR "Offset to long!\n");
	return -EINVAL;
    };

    /* calculate real write size */
    real_wr = ((*ofs + sz) > BUF_SZ) ? (BUF_SZ - *ofs) : sz;

    if(real_wr != sz) {
	printk(KERN_INFO "Write: at %d request %d, support %d (SZ=%d)\n", 
		*ofs, sz, real_wr, BUF_SZ);
    }

    /* real copy */
    real_wr = copy_from_user((void *)(buff + *ofs),ubuff, real_wr);

    printk(KERN_INFO "Module write return %d\n", real_wr);
    return real_wr;
}

long mod_ioctl(struct file *f, unsigned int cmd, unsigned long arg)
{
    if(stat) stat->ioctl++;
    printk(KERN_INFO "Module ioctl\n");
    return 0;
}

unsigned int mod_poll(struct file *f, struct poll_table_struct *pt)
{
    if(stat) stat->poll++;
    printk(KERN_INFO "Module poll\n");
    return 0;
}

struct file_operations kbuf_fops = {
    .owner = THIS_MODULE,
    .open = mod_open,
    .release = mod_release,
    .read = mod_read,
    .write = mod_write,
    .compat_ioctl = mod_ioctl,
    .poll = mod_poll,
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
