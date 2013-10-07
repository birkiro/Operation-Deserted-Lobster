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
#include <linux/interrupt.h>
 
#define GPIO_NUMBER    60     	// 60 = gpio1_28 (1 * 32 + 28 = 60)
 
static dev_t first;     	// Global variable for the first device number
static struct cdev c_dev;	// Global variable for the character device structure
static struct class *cl;	// Global variable for the device class
 
static int init_result;
static int button_irq = 0;
static int button_count = 0; // irq count

static irqreturn_t button_handler(int irq, void *dev_id)
{
	button_count++;
	printk(KERN_INFO "button: I got an interrupt.\n");
	return IRQ_HANDLED;
}

static ssize_t button_read( struct file* F, char *buf, size_t count, loff_t *f_pos )
{
	printk(KERN_INFO "button irq counts: %d\n", button_count);

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
 
static ssize_t button_write( struct file* F, const char *buf, size_t count, loff_t *f_pos )
{
	printk(KERN_INFO "button: Executing WRITE.\n");
 
	switch( buf[0] )
	{
	case '0':
		gpio_set_value(GPIO_NUMBER, 0);
		button_count = 0;
	break;
	case '1':
		gpio_set_value(GPIO_NUMBER, 1);
	break;
	 
	default:
		printk("button: Wrong option.\n");
	break;
	}
 
	return count;
}
 
static int button_open( struct inode *inode, struct file *file )
{
	return 0;
}
 
static int button_close( struct inode *inode, struct file *file )
{
	return 0;
}
 
static struct file_operations FileOps =
{
	.owner        = THIS_MODULE,
	.open         = button_open,
	.read         = button_read,
	.write        = button_write,
	.release      = button_close,
};
 
static int __init init_button(void)
{
	//init_result = register_chrdev( 0, "gpio", &FileOps );
	 
	init_result = alloc_chrdev_region( &first, 0, 1, "button_driver" );
	 
	if( 0 > init_result )
	{
		printk( KERN_ALERT "button: Device Registration failed\n" );
		return -1;
	}
	else
	{
	    printk( KERN_ALERT "button: Major number is: %d\n",init_result );
	    //return 0;
	}
	 
	if ( (cl = class_create( THIS_MODULE, "chardev" ) ) == NULL )
	{
		printk( KERN_ALERT "button: Class creation failed\n" );
		unregister_chrdev_region( first, 1 );
		return -1;
	}

	if( device_create( cl, NULL, first, NULL, "button_driver" ) == NULL )
	{
		printk( KERN_ALERT "button: Device creation failed\n" );
		class_destroy(cl);
		unregister_chrdev_region( first, 1 );
		return -1;
	}
	 
	cdev_init( &c_dev, &FileOps );
	 
	if( cdev_add( &c_dev, first, 1 ) == -1)
	{
		printk( KERN_ALERT "Device addition failed\n" );
		device_destroy( cl, first );
		class_destroy( cl );
		unregister_chrdev_region( first, 1 );
		return -1;
	}

	if(gpio_request(GPIO_NUMBER, "button_1"))
	{
		printk( KERN_ALERT "gpio request failed\n" );
		device_destroy( cl, first );
		class_destroy( cl );
		unregister_chrdev_region( first, 1 );
		return -1;
	}

	if((button_irq = gpio_to_irq(GPIO_NUMBER)) < 0)
	{
		printk( KERN_ALERT "gpio to irq failed\n" );
		device_destroy( cl, first );
		class_destroy( cl );
		unregister_chrdev_region( first, 1 );
		return -1;
	}

	if(request_irq(button_irq, button_handler, IRQF_TRIGGER_RISING | IRQF_DISABLED, "gpiomod#button", NULL ) == -1)
	{
		printk( KERN_ALERT "Device interrupt handle failed\n" );
		device_destroy( cl, first );
		class_destroy( cl );
		unregister_chrdev_region( first, 1 );

		return -1;
	}
	else
	{
		printk( KERN_ALERT "button: Device irq number is %d\n", button_irq );
	}

	 
	return 0; 
}
 
static void __exit cleanup_button(void)
{
	//unregister_chrdev( init_result, "gpio" );
	 
	cdev_del( &c_dev );
	device_destroy( cl, first );
	class_destroy( cl );
	unregister_chrdev_region( first, 1 );
	 
	printk(KERN_ALERT "button: Device unregistered\n");
 
	free_irq(button_irq, NULL);
	gpio_free(GPIO_NUMBER);
}

module_init(init_button);
module_exit(cleanup_button);
 
MODULE_AUTHOR("Birkir Oskarsson & Bjorn Smith");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Beaglebone Black gpio1_28 irq driver");
