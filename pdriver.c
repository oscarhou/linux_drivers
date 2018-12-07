#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/jiffies.h>
#include <linux/hrtimer.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Oscar Hou");
MODULE_DESCRIPTION("A module to read GPIO pin that connects to the particle sensor");
#define DEVICE_NAME "particle"
#define CLASS_NAME "part"


static int majorNum;
static struct class* particleCharClass = NULL;
static struct device* particleCharDevice = NULL;
static uint32_t k_dataGpio = 18;
static uint32_t k_ledGpio = 21;
static int irqNumber;
static unsigned long currJiffie;
static struct hrtimer htimer;
static ktime_t kt_period;


static ssize_t data_read(struct file *filep, char *buffer, size_t len, loff_t *offset)
{
	printk("Reading from device %lu\n",jiffies);
	
	

	return 0;
}

static int dev_open(struct inode *inodep, struct file *filep)
{
	return 0;
}

static int dev_close(struct inode *inodep, struct file *filep)
{
	return 0;
}

static void __exit sensor_exit(void)
{
	
}

static struct file_operations fops = {
	read: data_read,
	open: dev_open,
	release: dev_close
};

static irq_handler_t particleHandler(unsigned int irq, void *dev_id, struct pt_regs *regs)
{
	int particleOn = gpio_get_value(k_dataGpio);

	gpio_set_value(k_ledGpio, particleOn);

	return (irq_handler_t) IRQ_HANDLED;
}

static int __init sensor_init(void)
{
	majorNum = register_chrdev(0, DEVICE_NAME, &fops);
	if (majorNum < 0)
	{
		printk("Faild to register character device\n");
	}

	particleCharClass = class_create(THIS_MODULE, CLASS_NAME);
	if (IS_ERR(particleCharClass))
	{
		unregister_chrdev(majorNum, DEVICE_NAME);
		printk("failed to register class\n");
	}

	particleCharDevice = device_create(particleCharClass, NULL, MKDEV(majorNum, 0), NULL, DEVICE_NAME);
	if (IS_ERR(particleCharDevice))
	{
		class_destroy(particleCharClass);
		unregister_chrdev(majorNum, DEVICE_NAME);
		printk("failed to create device\n");
	}

	if (!gpio_is_valid(k_dataGpio))
	{
		printk("Invalid GPIO %d\n", k_dataGpio);
	}

	gpio_request(k_dataGpio, "sysfs");
	gpio_direction_input(k_dataGpio);
	gpio_request(k_ledGpio, "sysfs");
	gpio_direction_output(k_ledGpio, 1);

	irqNumber = gpio_to_irq(k_dataGpio);

	request_irq(irqNumber,
				(irq_handler_t) particleHandler,
				IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,
				"particleIrqHandler",
				NULL);
	return 0;
}

module_init(sensor_init);
module_exit(sensor_exit);
