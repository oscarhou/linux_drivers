#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/i2c.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Oscar Hou");
MODULE_DESCRIPTION("Device driver for the MPL115A2");

#define DEVICE_NAME "barometer"
#define CLASS_NAME "mpl"

static int majorNum;
static struct class* charClass = NULL;
static struct device* charDevice = NULL;

static const struct of_device_id mpl_of_match[] = {
	// TODO: Find out when this is used
	{ .compatible = "nxp,mpl115a2" },
	{}
};

static int mpl_probe(
	struct i2c_client *client,
	const struct i2c_device_id * id)
{
	return 0;

}

static int mpl_remove(struct i2c_client *client)
{
	return 0;
}

// Links the platform driver to the device driver
static struct i2c_driver mpl_driver =
{
	.probe = mpl_probe,
	.remove = mpl_remove,
	.driver = {
		.name = "nxp,mpl",
		.of_match_table = of_match_ptr(mpl_of_match)
	},
};



static int dev_open(struct inode *inodep, struct file *filep)
{
	return 0;
}

static int dev_close(struct inode *inodep, struct file *filep)
{
	return 0;
}

// Required for driver file ops
static struct file_operations fops = {
	open: dev_open,
	release : dev_close
};

static int __init init_mpl(void)
{
	printk("Starting MPL Module\n");
	// Register with platform driver
	i2c_add_driver(&mpl_driver);

	// Get Major and Minor driver numbers
	majorNum = register_chrdev(0, DEVICE_NAME, &fops);
	if (majorNum < 0)
		printk("Failed to register MPL driver major number\n");

	charClass = class_create(THIS_MODULE, CLASS_NAME);
	if (IS_ERR(charClass))
	{
		unregister_chrdev(majorNum, DEVICE_NAME);
		printk("Failed to register class for MPL\n");
	}

	charDevice = device_create(
		charClass,
		NULL,
		MKDEV(majorNum, 0),
		NULL,
		DEVICE_NAME);

	if (IS_ERR(charDevice))
	{
		class_destroy(charClass);
		unregister_chrdev(majorNum, DEVICE_NAME);
		printk("Failed to create device for MPL\n");
	}
	
	return 0;
}

static void __exit exit_mpl(void)
{
	printk("Stopping MPL Module\n");
	i2c_del_driver(&mpl_driver);
}

// TODO: Enable when using DTB?
// module_i2c_driver(mpl_driver);

module_init(init_mpl);
module_exit(exit_mpl);
