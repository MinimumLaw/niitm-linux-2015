#include <linux/module.h>
#include <linux/kernel.h>

void simple_export(void)
{
    printk(KERN_INFO "Simple 1 exported\n");
}
EXPORT_SYMBOL(simple_export);

int init_module(void)
{
    printk(KERN_INFO "Simple 1 init\n");
    return 0;
}

void cleanup_module(void)
{
    printk(KERN_INFO "Simple 1 exit\n");
}