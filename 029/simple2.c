#include <linux/module.h>
#include <linux/kernel.h>

void simple_export(void);

int init_module(void)
{
    printk(KERN_INFO "Simple 2 init\n");
    simple_export();
    return 0;
}

void cleanup_module(void)
{
    printk(KERN_INFO "Simple 2 exit\n");
}