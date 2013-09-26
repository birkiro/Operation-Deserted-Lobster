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
	{ 0xa60b9ee, "module_layout" },
	{ 0x86c8ad2e, "malloc_sizes" },
	{ 0x65d6d0f0, "gpio_direction_input" },
	{ 0x47229b5c, "gpio_request" },
	{ 0x9bf67b84, "kmem_cache_alloc" },
	{ 0xe680bae6, "device_create_file" },
	{ 0x2c28632b, "device_create" },
	{ 0x44e7643d, "__class_create" },
	{ 0x3afdbb64, "cdev_add" },
	{ 0x12bee511, "cdev_init" },
	{ 0x29537c9e, "alloc_chrdev_region" },
	{ 0x7485e15e, "unregister_chrdev_region" },
	{ 0x532bdb3a, "cdev_del" },
	{ 0xc2a80510, "class_destroy" },
	{ 0xe56368a0, "device_destroy" },
	{ 0x25353f, "device_remove_file" },
	{ 0xfe990052, "gpio_free" },
	{ 0x67c2fa54, "__copy_to_user" },
	{ 0x91715312, "sprintf" },
	{ 0x6c8d5ae8, "__gpio_get_value" },
	{ 0x27e1a049, "printk" },
	{ 0xfa2a45e, "__memzero" },
	{ 0xfbc74f64, "__copy_from_user" },
	{ 0xefd6cf06, "__aeabi_unwind_cpp_pr0" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "083E787B22AFEAA2D42F051");
