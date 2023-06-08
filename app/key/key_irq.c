#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/init.h>
#include <linux/module.h>

#include <linux/errno.h>
#include <linux/gpio.h>
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#include <linux/cdev.h>

#include <linux/device.h>
#include <linux/of.h>
#include <linux/of_address.h>

#include <linux/of_gpio.h>
#include <linux/of_irq.h>




/**
 * file name：key-irq
 * date: 2022-10-24  23:17
 * version：1.0
 * author:liefyuan
 * describe：irq device drive
 */


#define GPIOKEY_CNT  1     /* 设备号个数 */
#define GPIOKEY_NAME        "irq"      /* 设备名*/

/* 定义按键值 */
#define KEY0VALUE  0x01   /* 按键值 */
#define INVAKEY     0x00 /* 无效的按键值 */
#define KEY_NUM 1   /* 按键数量 */


/* 中断IO描述结构体 */
struct irq_keydesc {
    int gpio;                                               /* gpio */
    int irqnum;                                         /* 中断号*/
    unsigned char value;                    /* 按键对应的键值 */
    char name[10];                                           /* 名字 */
    irqreturn_t (*handler)(int , void *);  /* 中断服务函数 */
};

/* 设备结构体 自定义 */
struct gpiokey_dev{
    dev_t devid;     /*设备号  */
    struct cdev cdev;  /* cdev */
    struct class *class;  /* 类*/
    struct device *device;  /* 设备 */
    int major;   /* 主设备号 */
    int minor;  /* 次设备号 */

    struct device_node *nd;  /* 设备节点 */


    atomic_t keyvalue;       /* 有效的按键键值 */
    atomic_t releasekey;  /* 标记是否完成一次完成的按键 包括按下和释放 */

    struct irq_keydesc irqkeydesc[KEY_NUM];  /* 按键描述数组 */
    unsigned char curkeynum;  /* 当前的按键号 */
 };

/* 定义一个设备结构体 */
struct gpiokey_dev gpiokey;   /* key 设备 */

/* 定义定时器和回调函数 */
static void timer_function(struct timer_list *);
static DEFINE_TIMER(Timer, timer_function);  // Timer：是一个定时器对象  timer_function:回调函数


/* 中断服务函数 开启定时器 延时 10ms  定时器用于按键消抖*/
static irqreturn_t key0_handler(int irq, void *dev_id)
{
    struct gpiokey_dev *dev  = (struct gpiokey_dev *)dev_id;

    dev->curkeynum = 0;  // 当前按键号 

    mod_timer(&Timer,  jiffies + msecs_to_jiffies(10));  /* 10 ms定时 */

    return IRQ_RETVAL(IRQ_HANDLED);
}

/* 定时器服务函数 用于按键*/
void timer_function(struct timer_list * arg)
{
    unsigned char value;  // 按键值
    unsigned char num;  // 当前按键号 
    struct irq_keydesc *keydesc;  /* 中断描述 */


    num = gpiokey.curkeynum;  // 当前按键号 
    keydesc = &gpiokey.irqkeydesc[num];  // 当前按键号的中断描述

    value = gpio_get_value(keydesc->gpio);  // 读取按键值
    if(value == 0){  // 按键按下 
        atomic_set(&gpiokey.keyvalue, keydesc->value);  // 设置按键的值
    }else{  // 按键松开
        atomic_set(&gpiokey.keyvalue, 0x80 | keydesc->value);
        atomic_set(&gpiokey.releasekey, 1);  /* 标记松开按键  即完成一次完整的按键过程 */
    }
}

/* 打开设备 */
static int key_open(struct inode *inode, struct file *filp)
{

    filp->private_data = &gpiokey;  /* 设置私有数据 */
    printk("key open!\r\n");
    return 0;
}

/* 从设备读取数据 */
static ssize_t key_read(struct file *filp, char __user *buf, size_t cnt, loff_t *offt)
{
    int ret = 0;
    unsigned char keyvalue = 0, releasekey = 0; // 按键值和按键释放标志

    struct gpiokey_dev *dev = filp->private_data;

    keyvalue = atomic_read(&dev->keyvalue); // 读取按键值
    releasekey = atomic_read(&dev->releasekey); // 读取按键释放值

    if(releasekey){  // 有按键按下 
        if(keyvalue & 0x80){
            keyvalue &= ~0x80;
            ret = copy_to_user(buf, &keyvalue, sizeof(keyvalue));  // 拷贝到用户空间 
        }else{
            printk("data error!\r\n");
            return -EINVAL;;  // 数据错误
        }
       atomic_set(&dev->releasekey, 0); /* 按下标志清零 */
    }else{  // 没有按键按下 
        return -EINVAL;
    }

    //printk("key read !\r\n");
    return 0;
}

/* 往设备写数据 */
static ssize_t key_write(struct file *filp, const char __user *buf, size_t cnt, loff_t *offt)
{
    printk("key write !\r\n");
    return 0;
}

/* 释放设备 */
static int key_release(struct inode *inode, struct file *filp)
{
    //printk("key release!\r\n");
    return 0;
}


/* 设备操作函数结构体  */
static struct file_operations gpiokey_fops = {
    .owner = THIS_MODULE,
    .open = key_open,
    .read = key_read,
    .write = key_write,
    .release = key_release,
};

/* 驱动入口函数 */
static int __init mkey_init(void)
{
    int ret,i = 0;  // 返回值

    /* 初始化原子变量 */
    atomic_set(&gpiokey.keyvalue, INVAKEY);  // 原子变量初始值为无效的按键值 
    atomic_set(&gpiokey.releasekey, 0);  // 原子变量初始值为无效的按键值 


    /* 获取设备数中的属性数据  */
    /* 1. 获取设备节点 /key*/
    gpiokey.nd = of_find_node_by_path("/key");  // 通过绝对路径查找设备节点
    if(gpiokey.nd == NULL){
        printk("key node no find!\r\n");
        return -EINVAL;  /* 无效参数 不知道这个返回值是啥意思，我觉得返回一个负数就可以，这个值是23，不知道有没有处理*/
    }

    /* 2. 获取设备树中的gpio属性 得到key所使用的gpio编号 */
    /* 可能有多个按键  这里统一处理 兼容性更好 */
    for(i = 0; i< KEY_NUM; i++){
        gpiokey.irqkeydesc[i].gpio = of_get_named_gpio(gpiokey.nd, "key-gpio", i);
        if(gpiokey.irqkeydesc[i].gpio < 0 ){
            printk("can't get key-gpio%d\r\n",i);
            return -EINVAL;  /* 无效参数 不知道这个返回值是啥意思，我觉得返回一个负数就可以，这个值是23，不知道有没有处理*/
        }
        printk("key-gpio%d num = %d \r\n", i,gpiokey.irqkeydesc[i].gpio);  // 打印获取的key-gpio属性值
    }

    /* 初始化key所使用的IO 并且设置为中断模式 */
    for(i = 0; i< KEY_NUM; i++) {
        memset(gpiokey.irqkeydesc[i].name, 0, sizeof(gpiokey.irqkeydesc[i].name)); // 缓冲区清零
        sprintf(gpiokey.irqkeydesc[i].name, "key%d", i); // 组合名字

        gpio_request(gpiokey.irqkeydesc[i].gpio, gpiokey.irqkeydesc[i].name); // 申请IO
        gpio_direction_input(gpiokey.irqkeydesc[i].gpio);  // 设置为输入模式

        gpiokey.irqkeydesc[i].irqnum = irq_of_parse_and_map(gpiokey.nd, i);  // 获取中断号 
       
        #if 0
		imx6uirq.irqkeydesc[i].irqnum = gpio_to_irq(imx6uirq.irqkeydesc[i].gpio);
       #endif

        /* 打印按键信息 */ 
        printk("key%d:gpio=%d, irqnum=%d\r\n", i, gpiokey.irqkeydesc[i].gpio, gpiokey.irqkeydesc[i].irqnum);
    }
 
    /* 申请中断 */
    gpiokey.irqkeydesc[0].handler = key0_handler;  // 中断处理函数 
    gpiokey.irqkeydesc[0].value = KEY0VALUE;  // 按键值 

    for(i = 0 ; i < KEY_NUM; i++){
        ret = request_irq(gpiokey.irqkeydesc[i].irqnum, gpiokey.irqkeydesc[i].handler,
                 IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING, gpiokey.irqkeydesc[i].name, &gpiokey); 
        if(ret < 0){
            printk("irq %d request failed!\r\n", gpiokey.irqkeydesc[i].irqnum); // 中断号申请失败 
            return -EFAULT; 
        }
    }


    /* 注册字符设备驱动 */

    /* 1. 创建设备号 */
    if(gpiokey.major){  // 定义了设备号 
        gpiokey.devid = MKDEV(gpiokey.major, 0 );  // 根据主设备号和次设备号合成设备号 
        register_chrdev_region(gpiokey.devid, GPIOKEY_CNT, GPIOKEY_NAME);  // 注册设备号 
    }else{  // 没有定义设备号 动态生成 
        alloc_chrdev_region(&gpiokey.devid,0,GPIOKEY_CNT, GPIOKEY_NAME ); // 申请设备号
        gpiokey.major = MAJOR(gpiokey.devid);  // 获取主设备号
        gpiokey.minor = MINOR(gpiokey.devid);  // 获取次设备号
    }
    printk("gpiokey major = %d,minor = %d\r\n",gpiokey.major, gpiokey.minor);  // 打印主设备号和次设备号

    /* 2. 初始化 cdev */
    gpiokey.cdev.owner = THIS_MODULE;  
    cdev_init(&gpiokey.cdev, &gpiokey_fops);  // 初始化cdev
    /* 3. 添加cdev */
    cdev_add(&gpiokey.cdev, gpiokey.devid, GPIOKEY_CNT ); // 向linux系统添加cdev

     /* 自动创建设备节点文件 */
    /* 4. 创建类 */
    gpiokey.class = class_create(THIS_MODULE, GPIOKEY_NAME);  // 创建类 
    if(IS_ERR(gpiokey.class)){
        return PTR_ERR(gpiokey.class);
    }
    /* 创建设备 */
    gpiokey.device = device_create(gpiokey.class, NULL, gpiokey.devid, NULL, GPIOKEY_NAME);
      if(IS_ERR(gpiokey.device)){
        return PTR_ERR(gpiokey.device);
    }

    return 0;
}


/* 驱动出口函数 */
static void __exit mkey_exit(void)
{
   unsigned char i = 0;
    /* 释放IO */
    for(i = 0; i < KEY_NUM; i++){
        gpio_free(gpiokey.irqkeydesc[i].gpio);
    }

    /* 删除定时器 */
    del_timer_sync(&Timer);

    /* 释放中断 */
    for(i = 0; i < KEY_NUM; i++){
        free_irq(gpiokey.irqkeydesc[i].irqnum, &gpiokey);
    }

    /*  注销字符设备驱动 */
    cdev_del(&gpiokey.cdev);  /* 删除 cdev */
    unregister_chrdev_region(gpiokey.devid, GPIOKEY_CNT ); /* 注销设备号 */

    device_destroy(gpiokey.class, gpiokey.devid);  /* 注销设备  */
    class_destroy(gpiokey.class);  /* 注销类 */


    printk("key drive unregsister ok !\r\n");
}

/* 加载驱动入口和出口函数 */
module_init(mkey_init);
module_exit(mkey_exit);

/* LICENSE 和 AUTHOR 信息*/
MODULE_LICENSE("GPL");
MODULE_AUTHOR("liefyuan");


