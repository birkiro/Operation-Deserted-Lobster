
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/i2c.h>

#include "i2cdev.h"

#define USER_BUFF_SIZE 6

struct dac_dev {
	dev_t devt;
	struct cdev cdev;
	struct semaphore sem;
	struct class *class;
	char user_buff[USER_BUFF_SIZE];
};

static struct dac_dev dac_dev;
unsigned char data_string[50];
//static int cnt = 0;
int ret;

//I2C specific fields:
static struct i2c_client *dac_i2c_client;
static struct i2c_board_info dac_board_info = {I2C_BOARD_INFO(DRIVER_NAME, DAC_ADDRESS)};

static ssize_t dac_read(struct file *filp, char *buf, size_t count, loff_t *offp)
{
	int status = 0;
	unsigned char i = 0;

	int valprobe = 0;

	if(i2c_master_recv(dac_i2c_client, data_string, 6))
	{
		for(i=0; i<6; i++)
		{
			printk(KERN_INFO "reading from dac buffer[%d] 0x%x \n", i, data_string[i]);	
		}
		valprobe = ((data_string[1] << 4) & 0xFF0) | ((data_string[2] >> 4) & 0xF);
		printk(KERN_INFO "DAC register value is : %d", valprobe);
		valprobe = valprobe*3300/4096;
		printk(KERN_INFO "dac: current output voltage is %d.%d%d%dv.", valprobe/1000, valprobe%1000/100, 										       valprobe%100/10, valprobe%10);
		if(copy_to_user(buf, data_string, 6)) 
		{
			status = -EFAULT;
			goto read_done;
		}	
	}
    	read_done:
		return status;
}

static ssize_t dac_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos )
{
	int status = 1;
	unsigned char i,j;
	int _temp_buff[4]; //adapt the userspace input into dac raw value;
	int valraw = 0, val = 0;
	unsigned char buff[2];
	int calival = 0;

	if(copy_from_user(dac_dev.user_buff, buf, count)) 
	{
		printk(KERN_ALERT "Error copying data from userspace");
		status = -EFAULT;
		goto write_done;	
	}
	
	if((dac_dev.user_buff[0] >= 0x30 && dac_dev.user_buff[0] <= 0x33) && 
	   dac_dev.user_buff[1] == '.' &&
	   (dac_dev.user_buff[2] >= 0x30 && dac_dev.user_buff[2] <= 0x39) && 
	   (dac_dev.user_buff[3] >= 0x30 && dac_dev.user_buff[3] <= 0x39) && 
	   (dac_dev.user_buff[4] >= 0x30 && dac_dev.user_buff[4] <= 0x39) && 
	   dac_dev.user_buff[5] == 'v')
	{
		for(i=1; i<4; i++)
		{
			dac_dev.user_buff[i] = dac_dev.user_buff[i+1];	
		}
		for(j=0; j<4; j++)
		{
			_temp_buff[j] = dac_dev.user_buff[j] - '0';
			printk(KERN_INFO "User send %d: %d\n", j+1, _temp_buff[j]);
		}
		valraw = _temp_buff[0]*1000+_temp_buff[1]*100+_temp_buff[2]*10+_temp_buff[3];
		printk(KERN_INFO "user set voltage value: %d\n", valraw);
		val = valraw*4096/33;
		printk(KERN_INFO "processed middle value: %d\n", val);

		if ((val%10) > 5)
			val = val/10 + 1;
		else
			val = val/10;
		if ((val%10) > 5)
			val = val/10 + 1;
		else
			val = val/10;
		printk(KERN_INFO "unprocessed DAC register value: %d\n", val);

		//buff[0] = 0x60;
		buff[0] = (val >> 8) & 0xF;
		buff[1] = (val & 0xFF);

		//Here comes some calibration
		printk(KERN_INFO "calibration begins.\n");
		calival = (buff[0] << 8) | buff[1];
		printk(KERN_INFO "calibration DAC buffer: 0x%x\n", calival);
		calival = calival*330000/4096;
		if ((calival%10) > 5)
			calival = calival/10 + 1;
		else
			calival = calival/10;
		if ((calival%10) > 5)
			calival = calival/10 + 1;
		else
			calival = calival/10;
		printk(KERN_INFO "calibration DAC register value: %d\n", calival);

		while(calival != valraw)
		{
			if (calival > valraw)
				buff[1]--;
			else
				buff[1]++;
			printk("DAC register value has been adapted to: 0x%x 0x%x\n", buff[0], buff[1]);

			calival = (buff[0] << 8) | buff[1];
			calival = calival*330000/4096;
			if ((calival%10) > 5)
				calival = calival/10 + 1;
			else
				calival = calival/10;
			if ((calival%10) > 5)
				calival = calival/10 + 1;
			else
				calival = calival/10;
		}
		printk(KERN_INFO "calibration done.\n");
		//calibration done
	
		printk("dac: Sending 0x%x 0x%x\n", buff[0], buff[1]);
		i2c_master_send(dac_i2c_client, buff,2);
		status = 4;
	}
	/*else
	{
		printk("dac: Illegal command!\n");
		status = 1;
	}*/
	write_done:
		return status;
}

static int dac_open(struct inode *inode, struct file *filp)
{	
	//only allow one process to open this device by using a semaphore as mutual exclusive lock- mutex
	if(down_interruptible(&dac_dev.sem) != 0)
	{
		printk(KERN_ALERT "dac: could not lock device during open");
		return -1;
	}
	printk(KERN_INFO "dac: opened device");
	return 0;
}

static int dac_close(struct inode *inode, struct file *filp)
{	
	//by calling up , which is opposite of down for semaphore, we release the mutex that we obtained at device open
	//this has the effect of allowing other process to use the device now
	up(&dac_dev.sem);
	printk(KERN_INFO "dac: closed device");
	return 0;
}

static const struct file_operations dac_fops = {
	.owner = THIS_MODULE,
	.open =	dac_open,
	.release = dac_close,	
	.read =	dac_read,
	.write = dac_write,
};

static struct i2c_driver dac_i2c_driver = {
	.driver = {
		.name	= DRIVER_NAME,
		.owner	= THIS_MODULE,
	},
};


//********************************************************************************************************************************//
#define I2C_BUS		3
int __init dac_init_i2c(void)
{
	int ret;
	unsigned char _buff[2];
	struct i2c_adapter *adapter;

	/* register our driver */
	ret = i2c_add_driver(&dac_i2c_driver);
	if (ret)
	{
		printk(KERN_ALERT "Error registering i2c driver\n");
		return ret;
	}
	/* add our device */
	adapter = i2c_get_adapter(I2C_BUS);

	if (!adapter) 
	{
		printk(KERN_ALERT "i2c_get_adapter(%d) failed\n", I2C_BUS);
		return -1;
	}

	dac_i2c_client = i2c_new_device(adapter, &dac_board_info); 

			
	if (!dac_i2c_client) 
	{
		printk(KERN_ALERT "i2c_new_device failed\n");
		return -1;
	}

	i2c_put_adapter(adapter);	

	printk("Succesfully registered i2c driver for dac\n\r");

	printk("Initialising DAC ...\n");
	_buff[0] = 0x00;
 
	i2c_master_send(dac_i2c_client, _buff,1);
	if(i2c_master_recv(dac_i2c_client, _buff, 1))		
		ret = _buff[0];
	else	
		ret = -1;
	printk("DAC: Status %x\n",ret);
	printk("Done\n\r");

	return 0;
}

void __exit dac_cleanup_i2c(void)
{
	i2c_unregister_device(dac_i2c_client);
	dac_i2c_client = NULL;

	i2c_del_driver(&dac_i2c_driver);
}
//********************************************************************************************************************************//
//*** DRIVER HANDLING FUNCTIONS ***//
static int __init dac_init_cdev(void)
{
	int error;

	//dac_dev.devt = MKDEV(0, 0);

	error = alloc_chrdev_region(&dac_dev.devt, 0, 1, DRIVER_NAME); //dynamic allocation to assign the device

	if (error < 0) 
	{
		printk(KERN_ALERT "allocation for dac failed: error = %d \n", error);
		return -1;
	}

	cdev_init(&dac_dev.cdev, &dac_fops); //initialize cdev structure we allocated
	dac_dev.cdev.owner = THIS_MODULE; //struct has an owner

	error = cdev_add(&dac_dev.cdev, dac_dev.devt, 1); //create cdev, add it into kernel
	if (error) 
	{
		printk(KERN_ALERT "create cdev failed: error = %d\n", error);
		unregister_chrdev_region(dac_dev.devt, 1);
		return -1;
	}	

	return 0;
}

static int __init dac_init_class(void)
{
	dac_dev.class = class_create(THIS_MODULE, DRIVER_NAME);

	if (!dac_dev.class)
 	{
		printk(KERN_ALERT "class_create(i2c) failed\n");
		return -1;
	}

	if (!device_create(dac_dev.class, NULL, dac_dev.devt, NULL, DRIVER_NAME)) 
	{
		class_destroy(dac_dev.class);
		return -1;
	}

	return 0;
}

static int __init dac_init(void)
{
	printk(KERN_INFO "*******************************************************\n");
	printk(KERN_INFO "* I2C - Example Driver EPRO1 2013 v0.0.1       	*\n");
	printk(KERN_INFO "* Mads Clausen Institute - Sydansk Universitet 2013   *\n");
	printk(KERN_INFO "* Author: Robert Brehm                                *\n");
	printk(KERN_INFO "* Modified by Weinan Ji                               *\n");
	printk(KERN_INFO "* All rights reserved!                                *\n");
	printk(KERN_INFO "*******************************************************\n");
	printk(KERN_INFO "\n");


	//void *memset(void *s, int c, size_t n);
	//fills the first n bytes of the memory area pointed to by s with the constant byte c
	memset(&dac_dev, 0, sizeof(struct dac_dev)); 

	//initialize the semaphore
	sema_init(&dac_dev.sem, 1);

	if (dac_init_cdev() < 0)
		goto init_fail_1;

	if (dac_init_class() < 0)
		goto init_fail_2;

	if (dac_init_i2c() < 0)
		goto init_fail_3;


	return 0;

init_fail_3:
	device_destroy(dac_dev.class, dac_dev.devt);
	class_destroy(dac_dev.class);

init_fail_2:
	cdev_del(&dac_dev.cdev);
	unregister_chrdev_region(dac_dev.devt, 1);

init_fail_1:

	return -1;
}
module_init(dac_init);

static void __exit dac_exit(void)
{
	printk(KERN_INFO "i2cdev_exit()\n");

	dac_cleanup_i2c();
	printk(KERN_INFO "clean up successfully\n");

	device_destroy(dac_dev.class, dac_dev.devt);
  	class_destroy(dac_dev.class);
	printk(KERN_INFO "destroy device and class successfully\n");

	cdev_del(&dac_dev.cdev);
	printk(KERN_INFO "remove character device successfully\n");

	unregister_chrdev_region(dac_dev.devt, 1);
	printk(KERN_INFO "unregister done\n");

	/*if (dac_dev.user_buff)
	{
		kfree(dac_dev.user_buff);
		printk(KERN_INFO "memory freed\n");
	}*/
		
}
module_exit(dac_exit);



MODULE_AUTHOR("Robert Brehm");
MODULE_DESCRIPTION("I2C Example Driver");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_VERSION("0.1");



