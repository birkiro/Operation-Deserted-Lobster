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

#define DEVICE_NAME "driver"
#define CLASS_NAME "button"
#define GPIO1_28 60
#define STATUS_BUFFER_SIZE 128

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Birkir Oskarsson <birkiro@gmail.com>");	/* Who wrote this module? */
MODULE_DESCRIPTION("A character device that can controls one or more buttons, button1, button2, etc.");

/*
The structure of this device driver should be:
	Module with initialization
	Initialize hardware resources (get GPIOS)
	Create the character device
	include write function for serving requests
	include read function for providing information
*/

// File operation structure
static const struct file_operations button_fops = {
	.owner		= THIS_MODULE,
	.write		= button_write,
	.read 		= button_read,
};

static int __init init_button(void)
{
	int status = 0;
	bool valid = false;
	int value = 0;
	int ret = 0;
	int major;
	int minor;
	dev_t dev_t;

	// Set mux recieve enable bit.
	muxReceiveEnable();

	// Check GPIO valid or not.
	valid = gpio_is_valid ( GPIO1_28 );

	if ( valid == 0 )
	{
		printk (KERN_ALERT "Valid GPIO pin!!! \n");
	}
	else
	{
		printk (KERN_ALERT "InValid GPIO pin!!! \n");
	}

	// Request a GPIO.
	status = gpio_request ( GPIO1_28, "GPIO1_28" );

	if ( status < 0 )
	{
		printk (KERN_ALERT "GPIO request Failed!!! \n");
	}
	else
	{
		printk (KERN_ALERT "GPIO request successful!!!  \n");
	}

	// Make GPIO pin as input pin.
	gpio_direction_input ( GPIO1_28 );

	// Read the value in Pin 38
	value = gpio_get_value ( GPIO1_28 );

	printk (KERN_ALERT "GPIO value is %d \n", value);

	// Get the IRQ value
	ret = gpio_to_irq ( GPIO1_28 );

	// Install interrupt Handler. Interrupt for BOTH edge.
	ret = request_irq ( ret, gpio_irq_handler, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, DEVICE, NULL );

	if ( ret != 0 )
	{
		printk (KERN_INFO "Error: request_irq returned %d\n", ret);
		return -EIO;
	}

	return 0;
}

static void __exit exit_button(void)
{
		printk (KERN_ALERT "Exiting from GPIO Interrupt module!!! \n");

		// free the GPIO pin.
		gpio_free ( GPIO1_28 );

		// Free the interrupt request.
		free_irq ( gpio_to_irq ( GPIO1_28 ), NULL );
}

static ssize_t button_write
(struct file *filp, const char __user *buff, size_t count, loff_t *f_pos)
{
	// Remember to return a negative number to indicate if there is an error.
}
module_init(init_button);
module_exit(exit_button);
