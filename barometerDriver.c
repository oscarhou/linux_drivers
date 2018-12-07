#include <linux/init.h>
#include <linux/module.h>
#include <linux/sysfs.h>
#include <linux/i2c.h>
#include <linux/cdev.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Oscar Hou");
MODULE_DESCRIPTION("Device driver for the MPL115A2");

#define DEVICE_NAME "barometer"
#define CLASS_NAME "mpl"
// Address is 0x60
// Write is 0xC0, read is 0xC1

// Register Mapping of MPL115A2
#define PRES_ADC_MSB_REG 0x00
#define PRES_ADC_LSB_REG 0x01
#define TEMP_ADC_MSB_REG 0x02
#define TEMP_ADC_LSB_REG 0x03
#define A0_COEFF_MSB 0x04
#define A0_COEFF_LSB 0x05
#define B1_COEFF_MSB 0x06
#define B1_COEFF_LSB 0x07
#define B2_COEFF_MSB 0x08
#define B2_COEFF_LSB 0x09
#define C12_COEFF_MSB 0x0A
#define C12_COEFF_LSB 0x0B
#define CONVERT 0x12

struct mpl_i2c {
	dev_t mpl_dev;
	struct cdev cdev;
	struct class* charClass;
	struct device* charDevice;
	struct i2c_client* client;

	int32_t a0_msb;
	int32_t a0_lsb;
	int32_t b1_msb;
	int32_t b1_lsb;
	int32_t c12_msb;
	int32_t c12_lsb;
};

static int mpl_open(struct inode *inodep, struct file *filep)
{
	struct mpl_i2c* data;
	data = container_of(inodep->i_cdev, struct mpl_i2c, cdev);
	printk("Opening mpl data\n");
	printk("file open A0:MSB=%x,LSB=%x, B0:MSB=%x,LSB=%x, C12:MSB=%x,LSB,%x\n",
		data->a0_msb, data->a0_lsb, data->b1_msb, data->b1_lsb, data->c12_msb, data->c12_lsb);

	filep->private_data = data;
	
	return 0;
}

static ssize_t mpl_read(struct file *filep, char *buffer, size_t len, loff_t *offset)
{
	struct mpl_i2c *data;
	int rVal = 0;
	int32_t presMsb, presLsb, tempMsb, tempLsb;

	data = filep->private_data;

	tempMsb = i2c_smbus_read_byte_data(data->client, TEMP_ADC_MSB_REG);
	if (tempMsb < 0)
	{
		printk("Error reading Temperature MSB\n");
	}

	tempLsb = i2c_smbus_read_byte_data(data->client, TEMP_ADC_LSB_REG);
	if (tempLsb < 0)
	{
		printk("Error reading Temperature LSB\n");
	}

	presMsb = i2c_smbus_read_byte_data(data->client, PRES_ADC_MSB_REG);
	if (presMsb < 0)
	{
		printk("Error reading Pressure MSB\n");
	}

	presLsb = i2c_smbus_read_byte_data(data->client, PRES_ADC_LSB_REG);
	if (presLsb < 0)
	{
		printk("Error reading Pressure LSB\n");
	}

	printk("PresMsb %d, PresLsb %d, TempMSB %d, TempLsb %d\n",
			presMsb, presLsb, tempMsb, tempLsb);
	
	return 0;
}

static ssize_t mpl_write(
	struct file *filep,
	const char *buf,
	size_t len,
	loff_t *offset)
{
	struct mpl_i2c *data;
	int rVal = 0;
	data = filep->private_data;

	printk("Reading mpl data\n");
	rVal = i2c_smbus_write_byte_data(data->client, CONVERT, 0x00);
	if (rVal)
	{
		printk("Error writing to CONVERT register\n");
	}

	// Gotta return the number of characters written
	// or echo flips the F out.
	return len;
}

static int mpl_close(struct inode *inodep, struct file *filep)
{
	printk("Opening mpl data\n");
	return 0;
}
static const struct of_device_id mpl_of_match[] = {
	// TODO: Find out when this is used
	{ .compatible = "nxp,mpl115a2" },
	{}
};

static ssize_t temp_show(
	struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
	struct mpl_i2c* data = dev_get_drvdata(dev);
	printk("Showing Temperature\n");

	return 0;
}

static ssize_t pressure_show(
	struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
	struct mpl_i2c* data;

	data = dev_get_drvdata(dev);
	printk("Showing Pressure\n");
	return 0;
}

// Macro to create read only attributes
// dev_attr_XXX
static DEVICE_ATTR_RO(temp);
static DEVICE_ATTR_RO(pressure);

static struct file_operations mpl_fops =
{
	read: mpl_read,
	write: mpl_write,
	open: mpl_open,
	release: mpl_close
};

static int mpl_probe(
	struct i2c_client *client,
	const struct i2c_device_id * id)
{
	struct mpl_i2c* data;
	data = (struct mpl_i2c*) kzalloc(sizeof(struct mpl_i2c), GFP_KERNEL);
	if (!data)
	{
		printk("Failed to allocate space for driver data\n");
		return -1;
	}

	printk("Starting mpl Module\n");

	if ((alloc_chrdev_region(&data->mpl_dev, 1, 1, DEVICE_NAME)))
	{
		printk("Error allocating device major number\n");
	}

	cdev_init(&data->cdev, &mpl_fops);
	data->cdev.owner = THIS_MODULE;

	if ((cdev_add(&data->cdev, data->mpl_dev, 1)))
	{
		printk("Failed to add MPL driver");
	}
	
	// create a directory under /sys/
	data->charClass = class_create(THIS_MODULE, CLASS_NAME);
	if (IS_ERR(data->charClass))
	{
		printk("Failed to register class\n");
	}

	data->charDevice = device_create(data->charClass, NULL, data->mpl_dev, NULL, DEVICE_NAME);
	if (IS_ERR(data->charDevice))
	{
		class_destroy(data->charClass);
		printk("Failed to create device\n");
	}

	// Initialize mpl_i2c structure
	data->a0_msb = i2c_smbus_read_byte_data(client, A0_COEFF_MSB);
	data->a0_lsb = i2c_smbus_read_byte_data(client, A0_COEFF_LSB);
	data->b1_msb = i2c_smbus_read_byte_data(client, B1_COEFF_MSB);
	data->b1_lsb = i2c_smbus_read_byte_data(client, B1_COEFF_LSB);
	data->c12_msb = i2c_smbus_read_byte_data(client, C12_COEFF_MSB);
	data->c12_lsb = i2c_smbus_read_byte_data(client, C12_COEFF_LSB);

	printk("A0:MSB=%x,LSB=%x, B0:MSB=%x,LSB=%x, C12:MSB=%x,LSB,%x\n",
		data->a0_msb, data->a0_lsb, data->b1_msb, data->b1_lsb, data->c12_msb, data->c12_lsb);

	i2c_set_clientdata(client, data);
	dev_set_drvdata(data->charDevice, data);
	data->client = client;
	return 0;
}

static int mpl_remove(struct i2c_client *pClient)
{
	struct mpl_i2c* data;
	data = (struct mpl_i2c*) i2c_get_clientdata(pClient);
	printk("Removing MPL driver\n");
	device_destroy(data->charClass, data->mpl_dev);
	class_unregister(data->charClass);
	class_destroy(data->charClass);
	cdev_del(&data->cdev);
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

// Doing this prevents having to declare init/exit stuff
module_i2c_driver(mpl_driver);

