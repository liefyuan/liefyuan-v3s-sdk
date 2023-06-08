#include <linux/init.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>

struct file_operations misc_fops = {
	.owner = THIS_MODULE
};

struct miscdevice misc_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "hello_misc",
	.fops = &misc_fops
};


static int misc_init(void)
{
	int ret ;
	ret = misc_register(&misc_dev);
	if(ret < 0)
	{
		printk("misc registe is error\n");
		return -1;
	}
	printk("misc registe is ok\n");
	return 0;
}

static void misc_exit(void)
{
	misc_deregister(&misc_dev);
	printk("Bye Misc\n");
}

module_init(misc_init);
module_exit(misc_exit);

/* 模块声明 */
MODULE_LICENSE("GPL");
