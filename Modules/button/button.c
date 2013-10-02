/* This is a simple template for a character device 
 * driver which can be used as frame for further extension.
 * It creates a device from a custom class and provides the
 * device file /dev/chrdev_button which accepts the commands
 * "set" and "clr" to control the state of a virtual LED.
 *
 * After the module is compiled and copied to your beaglebone
 * install the module using:
 *
 * ~# insmod button.ko
 *
 * You can check the kernel log if everything went well:
 *
 * ~# dmesg | tail
 * [   49.307617] button: Successfully allocated device number Major: 246, Minor: 0
 * [   49.307647] button: Successfully registered driver with kernel.
 * [   49.307739] button: Successfully created custom device class.
 * [   49.313629] button: Successfully created device.
 * [   49.313659] button: Successfully created device file.
 * [   49.313690] button: Successfully allocated memory at address cfb392c0.
 *
 * If all went well tehn the device file should be created in the /dev directory.
 *
 * ~# ls -l | grep chrdev
 * -rw-r--r-- 1 root root      4 Feb 14 22:28 chrdev_button
 * 
 * You can now change the status of the virtual LED by writing into the device file:
 *
 * ~# echo set > /dev/chrdev_button
 * ~# dmesg | tail
 * [  996.621307] button: Switching virtual LED: ON
 *
 * You can then also check what the status of your virtual LED is at the moment by
 * reading from the device:
 *
 * ~# cat /dev/chrdev_button
 * ~# dmesg | tail
 * [ 1128.117309] button: Read device status. Status: set
 *
 * Now you could clear the status again and check the status change. play around with it and 
 * try to understand the code a bit.  
 */ 

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <asm/uaccess.h>
#include <linux/sysfs.h>
#include <linux/proc_fs.h>
#include <linux/device.h>
//from latency module/////////
#include <linux/gpio.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <asm/io.h>
/////////////////////////////

#define DEVICE_NAME "button"
#define CLASS_NAME "chrdev"

#define STATUS_BUFFER_SIZE 128

#define MY_GPIO 60 // Bank 1 = 32 + 28 = 60 // gpio1_28

/* 
 * Get rid of taint message by declaring code as GPL. 
 */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Birkir Oskarsson <biosk08@student.sdu.dk");	/* Who wrote this module? */
MODULE_DESCRIPTION("A chrdev character device driver as a button."); /* What does this module do */

struct button {
	struct cdev cdev;
	struct device* dev;
	struct class* class;
	dev_t dev_t;
	int major;
	int minor;
	char *status;
};
int read = 0;

static struct button button;


static ssize_t button_read( struct file* F, char *buf, size_t count, loff_t *f_pos )
{
	int status = 0;
	//int len = strlen(button.status);

	//if(len < count)
	//	count = len;	
	//printk(KERN_INFO "button: Read device status. Status: %s", button.status);

	char buffer[10];
	int gpio_value = gpio_get_value(MY_GPIO);
	sprintf( buffer, "%d" , gpio_value );
	count = sizeof( buffer );

	if(copy_to_user(buf, buffer, count)) {
		status = -EFAULT;
		goto read_done;
	}
	
	read_done:
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
	ssize_t status = 0;
	int i = 0;
	
	/* Copy data from the userspace (how much is read depends on the return value) */
	if(copy_from_user(button.status, buf, 4)) {
		printk(KERN_ALERT "button: Error copying data from userspace");
		status = -EFAULT;
		goto write_done;	
	}	

	/* Now process the command */
	if(button.status[0] == 's' && button.status[1] == 'e' 
					&& button.status[2] == 't') {
		printk(KERN_INFO "button: Switching virtual LED: ON\n");
		status = 4;
	}
	else if(button.status[0] == 'c' && button.status[1] == 'l' 
						&& button.status[2] == 'r') {
		printk(KERN_INFO "button: Switching virtual LED: OFF\n");
		status = 4;
	}
	else { 
		printk(KERN_WARNING "button: Illegal command!\n");
		status = 1;
	}

	write_done:
		return status;
}

static int button_open( struct inode *inode, struct file *file )
{
	//Here you would set mutex etc. so no one else can access your critical regions.

	return 0;
}

static int button_close( struct inode *inode, struct file *file )
{
	//Here you release your mutex again ... or set the device to sleep etc.
	return 0;
}

static struct file_operations FileOps =
{
	.open         = button_open,
	.read         = button_read,
	.write        = button_write,
	.release      = button_close,
};

static ssize_t sculld_show_dev(struct device *ddev,
                                struct device_attribute *attr,
                               char *buf)
 {
         /* no longer a good example. should use attr parameter. */
        return 0;
}
 
static DEVICE_ATTR(button, S_IRUGO, sculld_show_dev, NULL);


static void exit_button(void) { 
	// Free the button
	gpio_free(MY_GPIO);

	/* unregister fs (node) in /dev */
	device_remove_file(button.dev, &dev_attr_button);

	/* destroy device */
	device_destroy(button.class, button.dev_t);

	/* destroy the class */
	class_destroy(button.class);

	/* Get rid of our char dev entries */
	cdev_del(&button.cdev);

	/* cleanup_module is never called if registering failed */
	unregister_chrdev_region(button.dev_t, 1);	

	printk(KERN_WARNING "button: Device unregistered\n");
}

static int init_button(void)
{
	int error;
	
	button.dev_t = MKDEV(0,0);
	/* Allocate a device number (major and minor). The kernel happily do this 
	 * for you using the alloc_chrdev_region function.
	 *
	 * https://www.kernel.org/doc/htmldocs/kernel-api/API-alloc-chrdev-region.html
   	 *
  	 */

	error = alloc_chrdev_region(&button.dev_t, button.minor, 0, DEVICE_NAME);		
	if (error < 0) {
		printk(KERN_WARNING "button: Can't get device number %d\n", button.minor);
		return error;
	}
	button.major = MAJOR(button.dev_t);
	
	printk(KERN_INFO "button: Successfully allocated device number Major: %d, Minor: %d\n", button.major, button.minor);
	
	/* The kernel uses structures of type struct cdev to represent char devices internally. 	 
	 * Before the kernel invokes our device's operations, we must allocate and register one or 		 
	 * more of these structures. To do so, the code must include <linux/cdev.h>, where the 		 
	 * structure and its associated helper functions are defined.
	 * 
	 * In our example here we want to embed the cdev structure within a device-specific structure 		 
	 * of your own. In this case we initialize the structure that we have already allocated.  
	 */
	cdev_init(&button.cdev, &FileOps);
	/*struct cdev has an owner field that should be set to THIS_MODULE.*/
	button.cdev.owner = THIS_MODULE;
	/*Set your specific fiel operations defined in the above FileOps struct*/	
	button.cdev.ops = &FileOps;

	/* Once the cdev structure is set up, the final step is to tell the kernel about it with a 	    
	 * call to:
	 * https://www.kernel.org/doc/htmldocs/kernel-api/API-cdev-add.html
	 * Fail gracefully if need be 
	 */
	if (cdev_add (&button.cdev, button.dev_t, 1)) {
		printk(KERN_WARNING "button: Error adding device.\n");
		goto fail;	
	}
	printk(KERN_INFO "button: Successfully registered driver with kernel.\n");


	/* 
	 * Now we need to create the device node for user interaction in /dev
	 * 
	 *
 	 * We can either tie our device to a bus (existing, or one that we create)
 	 * or use a "virtual" device class. For this example, we choose the latter.
	 *
  	 */
 	button.class = class_create(THIS_MODULE, CLASS_NAME);
  	if (button.class < 0) {
  		printk(KERN_WARNING "button: Failed to register device class.\n");
  		goto fail;
 	}
	printk(KERN_INFO "button: Successfully created custom device class.\n");

	/* With a class, the easiest way to instantiate a device is to call device_create() */
 	button.dev = device_create(button.class, NULL, button.dev_t, NULL, DEVICE_NAME /*"_" CLASS_NAME*/);
	if (button.dev < 0) {
		printk(KERN_WARNING "button: Failed to create device.\n");
  		goto fail;
	}
	printk(KERN_INFO "button: Successfully created device.\n");

	error = device_create_file(button.dev, &dev_attr_button);
	if(error < 0) {
		printk(KERN_WARNING "button: Error creating device file.\n");
	}
	printk(KERN_INFO "button: Successfully created device file.\n");
		

	//AND allocate some memory for our status buffer:
	
	if(!button.status) {
		button.status = kmalloc(STATUS_BUFFER_SIZE, GFP_KERNEL);
		if(!button.status) {
			printk(KERN_ALERT "button: Failed to alloacte memory.\n");
			return -ENOMEM;
		}	
	}
	printk(KERN_ALERT "button: Successfully allocated memory at address %08x.\n",	
			button.status);

	return 0; /* succeed */

  	fail:
		exit_button();
		return error;
}
module_init(init_button);
module_exit(exit_button);
