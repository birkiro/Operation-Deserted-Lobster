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
	{ 0xf20dabd8, "free_irq" },
	{ 0xd5f2172f, "del_timer_sync" },
	{ 0xac8f37b2, "outer_cache" },
	{ 0xfe990052, "gpio_free" },
	{ 0xbe2c0274, "add_timer" },
	{ 0x593a99b, "init_timer_key" },
	{ 0x1c132024, "request_any_context_irq" },
	{ 0x11f447ce, "__gpio_to_irq" },
	{ 0x403f9529, "gpio_request_one" },
	{ 0xc2165d85, "__arm_iounmap" },
	{ 0x40a6f522, "__arm_ioremap" },
	{ 0x7d11c268, "jiffies" },
	{ 0x3ce4ca6f, "disable_irq" },
	{ 0x8834396c, "mod_timer" },
	{ 0x27e1a049, "printk" },
	{ 0xe6da44a, "set_normalized_timespec" },
	{ 0xefd6cf06, "__aeabi_unwind_cpp_pr0" },
	{ 0x432fd7f6, "__gpio_set_value" },
	{ 0x46608fa0, "getnstimeofday" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "35D53D6FBE4698EE8497889");
