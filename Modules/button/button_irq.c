#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <asm/uaccess.h>
#include <linux/version.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/moduleparam.h>


#define GPIO_NUMBER    60 // on beaglebone black gpio1_28
#define MAX_BUTTON_DEV 2

static int button_major = 0;
module_param(button_major, int, 0);

MODULE_AUTHOR("Birkir Oskarsson");
MODULE_LICENSE("Dual BSD/GPL");

// .open
static int button_open(struct inode *inode, struct file *filp)
{
	return 0;
}

// .release
static int button_release(struct inode *inode, struct file *filp)
{
	return 0;
}

// .read	from Bjorn
static ssize_t button_read( struct file* F, char *buf, size_t count, loff_t *f_pos )
{
	char buffer[10];
	 
	int temp = gpio_get_value(GPIO_NUMBER);
	 
	sprintf( buffer, "%1d" , temp );
	 
	count = sizeof( buffer );
	 
	if( copy_to_user( buf, buffer, count ) )
	{
		return -EFAULT;
	}
	 
	if( *f_pos == 0 )
	{
		*f_pos += 1;
		return 1;
	}
	else
	{
		return 0;
	}
}

// .write
static ssize_t button_write( struct file* F, const char *buf, size_t count, loff_t *f_pos )
{
	printk(KERN_INFO "Executing WRITE.\n");
 
	switch( buf[0] )
	{
	case '0':
		gpio_set_value(GPIO_NUMBER, 0);
	break;
	 
	case '1':
		gpio_set_value(GPIO_NUMBER, 1);
	break;
	 
	default:
		printk("Wrong option.\n");
	break;
	}
 
	return count;
}

// Setup of a cdev structure for a device.
static void button_setup_cdev(struct cdev *dev, int minor, struct file_operations *fops)
{
	int err, devno = MKDEV(button_major, minor);
    
	cdev_init(dev, fops);
	dev->owner = THIS_MODULE;
	dev->ops = fops;
	err = cdev_add (dev, devno, 1);
	/* Fail gracefully if need be */
	if (err) printk(KERN_NOTICE "Error %d adding button%d", err, minor);
	
}

static struct file_operations FileOps =
{
	.owner        = THIS_MODULE,
	.open         = button_open,
	.read         = button_read,
	.write        = button_write,
	.release      = button_release,
};


static struct cdev ButtonDevs[MAX_BUTTON_DEV];

static int button_init(void)
{
	int result;
	dev_t dev = MKDEV(button_major, 0);

	/* Figure out our device number. */
	if (button_major)
		result = register_chrdev_region(dev, 2, "button_drv");
	else {
		result = alloc_chrdev_region(&dev, 0, 2, "button_drv");
		button_major = MAJOR(dev);
	}
	if (result < 0) {
		printk(KERN_WARNING "button: unable to get major %d\n", button_major);
		return result;
	}
	if (button_major == 0)
		button_major = result;

	// Set up 2 cdevs using the setup function
	button_setup_cdev(ButtonDevs, 0, &FileOps);
	button_setup_cdev(ButtonDevs + 1, 1, &FileOps);
	return 0;
}


static void button_cleanup(void)
{
	cdev_del(ButtonDevs);
	cdev_del(ButtonDevs + 1);
	unregister_chrdev_region(MKDEV(button_major, 0), 2);
}

module_init(button_init);
module_exit(button_cleanup);
