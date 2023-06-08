/***************************************************************************
 * DHT11 linux5.2内核驱动
 * 
 * 2022-10-29
 * 
***************************************************************************/
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/input.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>

#include <linux/of_gpio.h>

#define DEVICE_NAME	"dht11_ctl"//设备名称
#define DRIVER_NAME "dht11_ctl"

uint32_t dth11_pin = 1;//定义dht11的管脚属性

struct dht11_data
{
    u_int16_t temp;//温度
    u_int16_t hmi;//湿度
};


#define delay_us(x)		udelay(x)
#define delay_ms(x)		msleep(x)

static u8 dht11_read_io(void)    
{
	gpio_direction_input(dth11_pin);
	return gpio_get_value(dth11_pin);
}

//复位DHT11
static void dht11_rst(void)   
{                 
	gpio_direction_output(dth11_pin, 0);
	msleep (20);  //拉低至少18ms
	gpio_direction_output(dth11_pin, 1); //DQ=1 
	delay_us (30);  //主机拉高20~40us
}

//等待DHT11的回应
//返回1:未检测到DHT11的存在
//返回0:存在
static u8 dht11_check(void)    
{   
	u8 retry=0;//定义临时变量
	gpio_direction_input(dth11_pin);//设置io口为输入模式
	while ((dht11_read_io()==1)&&retry<100)//DHT11会拉低40~80us
	{
		retry++;
		delay_us(1);
	}; 
	if(retry>=100)return 1;
	else retry=0;
	while ((dht11_read_io()==0)&&retry<100)//DHT11拉低后会再次拉高40~80us
	{
		retry++;
		delay_us(1);
	};
	if(retry>=100)return 1;    
	return 0;
}

//从DHT11读取一个位
//返回值：1/0
static u8 dht11_read_bit(void)  
{
	u8 retry=0;
	while((dht11_read_io()==1)&&retry<100)//等待变为低电平
	{
		retry++;
		delay_us(1);
	}
	retry=0;
	while((dht11_read_io()==0)&&retry<100)//等待变高电平
	{
		retry++;
		delay_us(1);
	}
	delay_us(40);//等待40us
	if(dht11_read_io()==1)
	return 1;
	else 
	return 0;   
}
/*读取一个byte*/
static u8 dht11_read_byte(void)    
{        
    u8 i,dat;
    dat=0;
	for (i=0;i<8;i++) 
	{
		dat<<=1; 
		dat|=dht11_read_bit();//读取一个位
	}    
	return dat;
}
 
/*从DHT11中取出温度和湿度数据*/
static u8 dht11_read_data(u_int16_t *temp,u_int16_t *humi)    
{        
	u8 buf[5];
	u8 i;
	dht11_rst();
	if(dht11_check()==0)
	{
		for(i=0;i<5;i++)//读取40位数据
		{
			buf[i]=dht11_read_byte();//读取一个数据
		}
		if((buf[0]+buf[1]+buf[2]+buf[3])==buf[4])
		{
			*humi=buf[0]<<8|buf[1];
			*temp=buf[2]<<8|buf[3];
			printk("readout data:buf=%d,%d,%d,%d,%d\n",buf[0],buf[1],buf[2],buf[3],buf[4]);
		}
	}else return 1;
	return 0;    
}


/*检测dht11所连接的io口的状态*/
static void dht11_init(void)
{     
	dht11_rst();  //复位DHT11
	dht11_check();//等待DHT11的回应
}



/*dht11读函数*/
ssize_t dht11_read(struct file *file, char __user *user, size_t bytesize, loff_t *this_loff_t)
{
    struct dht11_data this_data;//得到当前的温度和湿度信息
	if(dht11_read_data(&this_data.temp,&this_data.hmi) == 0);//读取温湿度值 
	{
        //将温度和湿度信息拷贝到user指针中
		if ( copy_to_user(user,&this_data,sizeof(this_data)) )
		{
			return EFAULT ;
		}
	}
}


/*dht11设备节点打开函数*/
int dht11_open(struct inode *inode, struct file *file)
{
    printk("--------------%s--------------\n",__FUNCTION__);
	return 0;
}

/*设备节点关闭*/
int dht11_close(struct inode *inode, struct file *file)
{
    return 0;//返回0
}
/*ioctl读取函数*/
long dht11_ioctl(struct file *file, unsigned int item, unsigned long long_item)
{
    return 0;
}

/*dht11文件结构体*/
static struct file_operations dht11_ops = {
	.owner			= THIS_MODULE,
	.open			= dht11_open,
	.release		= dht11_close, 
	.unlocked_ioctl	= dht11_ioctl,
	.read			= dht11_read
};

/*将DHT11注册为杂项*/
static struct miscdevice dht11_misc_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = DEVICE_NAME,
	.fops = &dht11_ops,
};


/*probe 函数*/
static int dht11_probe(struct platform_device *pdev)
{
	int ret;
    enum of_gpio_flags flag;//(flag == OF_GPIO_ACTIVE_LOW) ?
    struct device_node *dht11_gpio_node = pdev->dev.of_node; 
	
	dth11_pin = of_get_named_gpio_flags(dht11_gpio_node->child, "gpios", 0, &flag); 

	if (!gpio_is_valid(dth11_pin)) 
	{
    	printk("dht11-gpio: %d is invalid\n", dth11_pin); 
		return -ENODEV;
    }
	else
		printk("dht11-gpio: %d is valid!\n", dth11_pin);
	if (gpio_request(dth11_pin, "dht11-gpio")) 
	{ 
        printk("gpio %d request failed!\n", dth11_pin); 
        gpio_free(dth11_pin); 
        return -ENODEV;
    }
	else
		printk("gpio %d request success!\n", dth11_pin);

    /**/
    ret = misc_register(&dht11_misc_dev);//注册设备为杂项设备
    msleep(100);//延时100毫秒
    printk(DEVICE_NAME "\tinitialized\n");
    dht11_init();//初始化一次系统
	return 0;
}


static int dht11_remove (struct platform_device *pdev)
{
	misc_deregister(&dht11_misc_dev);
	gpio_free(dth11_pin);	

	return 0;
}

static int dht11_suspend (struct platform_device *pdev, pm_message_t state)
{
	printk("dht11 suspend off\n");
	return 0;
}

static int dht11_resume (struct platform_device *pdev)
{
	printk("dht11 resume on!\r\n");
	return 0;
}

/*匹配设备树驱动*/
static const struct of_device_id dht11_of_match[] = {
        { .compatible = "dht-11" },
        { /* sentinel */ }
};
/**/
MODULE_DEVICE_TABLE(of, dht11_of_match);

/*注册平台驱动*/
static struct platform_driver dht11_driver = {
	.probe = dht11_probe,
	.remove = dht11_remove,
	.suspend = dht11_suspend,
	.resume = dht11_resume,
	.driver = {
		.name = DRIVER_NAME,
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(dht11_of_match),
	},
};

/*设备驱动挂载函数*/
static int __init dht11_dev_init(void) {
    printk("dht 11 driver config successfully\r\n");
	return platform_driver_register(&dht11_driver);
}

/*设备驱动退出函数*/
static void __exit dht11_dev_exit(void) {
	platform_driver_unregister(&dht11_driver);
}

module_init(dht11_dev_init);
module_exit(dht11_dev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("liefyuan");
