#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <asm/io.h>       //含有 ioremap 函数 iounmap 函数
#include <linux/uaccess.h>  //含有 copy_from_user 函数和含有 copy_to_user 函数
#include <linux/device.h> //含有类相关的设备函数
#include <linux/cdev.h>

#define GPIOG_CFG0 (0x01C208D8)
#define GPIOG_CFG1 (0x01C208DC)
#define GPIOG_DATA (0x01C208E8)
#define GPIOG_PUL0 (0x01C208F4)

static dev_t led_dev_num;       //定义一个设备号
static struct cdev *led_dev;    //定义一个设备管理结构体指针
static struct class *led_class; //定义一个设备类
static struct device *led0;     //定义一个设备

size_t *gpiog_cfg0; //存储虚拟地址到物理地址映射
size_t *gpiog_cfg1; //存储虚拟地址到物理地址映射
size_t *gpiog_data; //存储虚拟地址到物理地址映射
size_t *gpiog_pul0; //存储虚拟地址到物理地址映射

static int led_open(struct inode *inode, struct file *file)
{
	/* GPIOG 配置 */
	// *((volatile size_t*)gpioe_cfg1) &= ~(7<<16); //清除配置寄存器
	// *((volatile size_t*)gpioe_cfg1) |= (1<<16);  //配置 GPIOE12 为输出模式
	// *((volatile size_t*)gpioe_pul0) &= ~(3<<16); //清除上/下拉寄存器
	// *((volatile size_t*)gpioe_pul0) |= (1<<12);  //配置 GPIOE12 为上拉模式

	*((volatile size_t*)gpiog_cfg1) &= ~(7<<16); //清除配置寄存器
	*((volatile size_t*)gpiog_cfg1) |= (1<<1);  //配置 GPIOG1 为输出模式
	*((volatile size_t*)gpiog_pul0) &= ~(3<<16); //清除上/下拉寄存器
	*((volatile size_t*)gpiog_pul0) |= (1<<1);  //配置 GPIOG1 为上拉模式

	printk(KERN_DEBUG"open led!!!\n");
	return 0;
}

static int led_close(struct inode *inode, struct file *filp)
{
	printk(KERN_DEBUG"close led!!!\n");
	return 0;
}

static int led_read(struct file *filp, char __user *buff, size_t count, loff_t *offp)
{
	int ret;
	size_t status = ((*((volatile size_t*)gpiog_data))>>1)&0x01;//获取 GPIOG1 状态
	ret = copy_to_user(buff,&status,4); //将内核空间拷贝到用户空间 buff
	if(ret < 0)
		printk(KERN_DEBUG"read error!!!\n"); //输出信息linux 驱动开发指南 | 李山文 著
	else
		printk(KERN_DEBUG"read led ok!!!\n"); //输出信息
	return 0;
}

static int led_write(struct file *filp, const char __user *buff, size_t count, loff_t *offp)
{
	int ret;
	size_t status;
	ret = copy_from_user(&status,buff,4); //将用户空间拷贝到内核空间的 status
	if(ret < 0)
		printk(KERN_DEBUG"write error!!!\n"); //输出信息
	else
		printk(KERN_DEBUG"write led ok!!!\n"); //输出信息
		*((volatile size_t*)gpiog_data) &= ~(1<<1) ;//清除 GPIOG1 状态
	if(status)
		*((volatile size_t*)gpiog_data) |= (1<<1);//设置 GPIOG1 状态 1
	return 0;
}

static struct file_operations led_ops = {
	.owner = THIS_MODULE,
	.open = led_open,
	.read = led_read,
	.write = led_write,
	.release = led_close,
};

static int __init led_init(void)
{
	int ret;
	led_dev = cdev_alloc(); //动态申请一个设备结构体
	if(led_dev == NULL)
	{
		printk(KERN_WARNING"cdev_alloc failed!\n");
		return -1;
	}
	ret = alloc_chrdev_region(&led_dev_num,0,1,"led"); //动态申请一个设备号
	if(ret !=0)
	{
		printk(KERN_WARNING"alloc_chrdev_region failed!\n");
		return -1;
	}
	led_dev->owner = THIS_MODULE; //初始化设备管理结构体的 owner 为 THIS_MODULE
	led_dev->ops = &led_ops; //初始化设备操作函数指针为 led_ops 函数
	cdev_add(led_dev,led_dev_num,1); //将设备添加到内核中
	led_class = class_create(THIS_MODULE, "led_class"); //创建一个名为 led_class 的类linux 驱动开发指南 | 李山文 著

	if(led_class == NULL)
	{
		printk(KERN_WARNING"led_class failed!\n");
		return -1;
	}
	led0 = device_create(led_class,NULL,led_dev_num,NULL,"led0");//创建一个设备名为 led0
	if(IS_ERR(led0))
	{
		printk(KERN_WARNING"device_create failed!\n");
		return -1;
	}
	gpiog_cfg0 = ioremap(GPIOG_CFG0,4); //将 GPIOG_CFG0 物理地址映射为虚拟地址
	gpiog_cfg1 = ioremap(GPIOG_CFG1,4); //将 GPIOG_CFG1 物理地址映射为虚拟地址
	gpiog_data = ioremap(GPIOG_DATA,4); //将 GPIOG_DATA 物理地址映射为虚拟地址
	gpiog_pul0 = ioremap(GPIOG_PUL0,4); //将 GPIOG_PUL0 物理地址映射为虚拟地址
	return 0;
}

static void __exit led_exit(void)
{
	cdev_del(led_dev); //从内核中删除设备管理结构体
	unregister_chrdev_region(led_dev_num,1); //注销设备号
	device_destroy(led_class,led_dev_num); //删除设备节点
	class_destroy(led_class); //删除设备类
	iounmap(gpiog_cfg0); //取消 GPIOG_CFG0 映射
	iounmap(gpiog_cfg1); //取消 GPIOG_CFG1 映射
	iounmap(gpiog_data); //取消 GPIOG_DATA 映射
	iounmap(gpiog_pul0); //取消 GPIOG_PUL0 映射
}

module_init(led_init);
module_exit(led_exit);

MODULE_LICENSE("GPL"); //不加的话加载会有错误提醒
MODULE_AUTHOR("liefyuan@qq.com"); //作者
MODULE_VERSION("0.1"); //版本
MODULE_DESCRIPTION("led_dev"); //简单的描述

