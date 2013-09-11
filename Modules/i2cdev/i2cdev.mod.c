#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x17634745, "module_layout" },
	{ 0x95d52825, "__class_create" },
	{ 0x7485e15e, "unregister_chrdev_region" },
	{ 0x77ecd5e7, "cdev_del" },
	{ 0x2f729c13, "class_destroy" },
	{ 0xd6be847f, "device_destroy" },
	{ 0xd9717d24, "device_create" },
	{ 0x21e4a50b, "cdev_add" },
	{ 0x39320ce4, "cdev_init" },
	{ 0x29537c9e, "alloc_chrdev_region" },
	{ 0x42191ea9, "i2c_del_driver" },
	{ 0x831c2fde, "i2c_unregister_device" },
	{ 0xda5b29be, "i2c_put_adapter" },
	{ 0x965e8def, "i2c_new_device" },
	{ 0xf521f770, "i2c_get_adapter" },
	{ 0x4a10ba4c, "i2c_register_driver" },
	{ 0x67c2fa54, "__copy_to_user" },
	{ 0x48a24285, "i2c_master_recv" },
	{ 0x25c819b6, "i2c_master_send" },
	{ 0xfa2a45e, "__memzero" },
	{ 0xfbc74f64, "__copy_from_user" },
	{ 0xff178f6, "__aeabi_idivmod" },
	{ 0x2196324, "__aeabi_idiv" },
	{ 0xefd6cf06, "__aeabi_unwind_cpp_pr0" },
	{ 0x695b2cfd, "down_interruptible" },
	{ 0x2e5810c6, "__aeabi_unwind_cpp_pr1" },
	{ 0x27e1a049, "printk" },
	{ 0xc9f5df35, "up" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "37F8D2BA1D6B480589194CD");
