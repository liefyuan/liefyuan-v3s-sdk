#include <linux/init.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

int misc_open(struct inode* inode, struct file* file)
{
	printk("hello misc_open\n");
	return 0;
}

int misc_release(struct inode* inode, struct file* file)
{
        printk("misc_release bye\n");
        return 0;
}

ssize_t misc_read(struct file *file, char __user *ubuf, \
                        size_t size, loff_t *loff_t)
{
        char kbuf[64] = "liefyuan1994";
        if(copy_to_user(ubuf, kbuf, strlen(kbuf)) != 0)
        {
                printk("copy_to_user error!\n");
                return -1;
        }
        printk("misc_read\n");
        return 0;
}

ssize_t misc_write(struct file *file, char __user *ubuf, \
                        size_t size, loff_t *loff_t)
{
        char kbuf[64] = {0};
        if(copy_from_user(kbuf, ubuf, size) != 0)
        {
                printk("copy_from_user error!\n");
                return -1;
        }
        printk("kbuf is :%s\n", kbuf);
        printk("misc_write\n");
        return 0;
}

struct file_operations misc_fops = {
        .owner = THIS_MODULE,
        .open = misc_open,
        .release = misc_release,
        .read = misc_read,
        .write = misc_write
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
