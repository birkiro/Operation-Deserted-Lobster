/*
 * A simple echo device driver.
 *
 * Usage:
 * # echo This is an echo message > /dev/echodevice
 * # cat /dev/echodevice
 */

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
#include <linux/time.h>

#define BUFFER_SIZE 25

static dev_t devicenumber;		// Global variable for the device number
static struct cdev c_dev;		// Global variable for the character device structure
static struct class *cl;		// Global variable for the device class
 
static char msg[BUFFER_SIZE];	// The buffer we use to store a string in .read and .write

/*
 * .read
 */
static ssize_t echodevice_read( struct file* F, char *buf, size_t count, loff_t *f_pos )
{
	// If the buffer is full, we finish reading.
	if(*f_pos >= BUFFER_SIZE)
	{
		return 0;
	}

	// Overload protection
	if(*f_pos + count > BUFFER_SIZE)
	{
		count = BUFFER_SIZE - *f_pos;
	}

	// Hand what the buffer contains to user space
	if( copy_to_user( buf, &msg + *f_pos, count ))
	{
		return -EFAULT;
	}

	// increment counter so we can take in what comes next
	*f_pos += count;
	printk(KERN_INFO "echodevice: .read\n");
	return count;
}

/*
 * .write
 */
static ssize_t echodevice_write( struct file* F, const char *buf, size_t count, loff_t *f_pos )
{
	// If the buffer is full, we return a kernel message indicating a buffer overload.
	if(*f_pos >= BUFFER_SIZE)
	{
		printk( KERN_ALERT "echodevice: BUFFER OVERLOAD!\n" );
		return -1;
	}

	// Overload protection
	if(*f_pos + count > BUFFER_SIZE)
	{
		count = BUFFER_SIZE - *f_pos;
	}

	/*
	 *  when you do # echo STRING > /dev/echodevice
	 *  this function fills in the buffer
	 */
	if( copy_from_user( &msg + *f_pos, buf, count ))
	{
		return -EFAULT;
	}

	// increment counter so we can take in what comes next
	*f_pos += count;
	printk(KERN_INFO "echodevice: .write\n");
	return count;
}

/*
 * / .open
 */
static int echodevice_open( struct inode *inode, struct file *file )
{
	return 0;
}

/*
 * .close
 */
static int echodevice_close( struct inode *inode, struct file *file )
{
	return 0;
}
 
static struct file_operations FileOps =
{
	.owner        = THIS_MODULE,
	.open         = echodevice_open,
	.read         = echodevice_read,
	.write        = echodevice_write,
	.release      = echodevice_close,
};

/*
 * Module init function.
 */
static int __init init_echodevice(void)
{
	if( 0 > alloc_chrdev_region( &devicenumber, 0, 1, "echodevice" ) )
	{
		printk( KERN_ALERT "echodevice: Device Registration failed\n" );
		return -1;
	}
	 
	if ( (cl = class_create( THIS_MODULE, "chardev" ) ) == NULL )
	{
		printk( KERN_ALERT "echodevice: Class creation failed\n" );
		unregister_chrdev_region( devicenumber, 1 );
		return -1;
	}

	if( device_create( cl, NULL, devicenumber, NULL, "echodevice" ) == NULL )
	{
		printk( KERN_ALERT "echodevice: Device creation failed\n" );
		class_destroy(cl);
		unregister_chrdev_region( devicenumber, 1 );
		return -1;
	}
	 
	cdev_init( &c_dev, &FileOps );
	 
	if( cdev_add( &c_dev, devicenumber, 1 ) == -1)
	{
		printk( KERN_ALERT "echodevice: Device addition failed\n" );
		device_destroy( cl, devicenumber );
		class_destroy( cl );
		unregister_chrdev_region( devicenumber, 1 );
		return -1;
	}
	printk(KERN_ALERT "echodevice: Device Registered <major, minor> = <%d, %d>\n", MAJOR(devicenumber), MINOR(devicenumber));
	return 0; 
}

/*
 * Module exit function
 */
static void __exit cleanup_echodevice(void)
{
	cdev_del( &c_dev );
	device_destroy( cl, devicenumber );
	class_destroy( cl );
	unregister_chrdev_region( devicenumber, 1 );
	 
	printk(KERN_ALERT "echodevice: Device unregistered\n");
}

module_init(init_echodevice);
module_exit(cleanup_echodevice);
 
MODULE_AUTHOR("Birkir Oskarsson & Bjorn Smith");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Beaglebone Black echo device");
