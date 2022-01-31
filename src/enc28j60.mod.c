#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0xbd4a4d5c, "module_layout" },
	{ 0xac875933, "driver_unregister" },
	{ 0xb14ebdf6, "__spi_register_driver" },
	{ 0xcbc0ea8e, "spi_sync" },
	{ 0x86332725, "__stack_chk_fail" },
	{ 0xade12dec, "devm_kmalloc" },
	{ 0xa8d01b8f, "_dev_err" },
	{ 0xffb0c88d, "spi_setup" },
	{ 0x5f754e5a, "memset" },
	{ 0xc5850110, "printk" },
	{ 0xe471ab95, "_dev_info" },
	{ 0x85fb927f, "of_property_read_variable_u32_array" },
	{ 0x217d7f37, "of_find_property" },
	{ 0x8f678b07, "__stack_chk_guard" },
	{ 0x19832790, "netif_tx_wake_queue" },
	{ 0xc38c83b8, "mod_timer" },
	{ 0x526c3a6c, "jiffies" },
	{ 0x3ebfa27e, "netif_carrier_off" },
	{ 0xb1ad28e0, "__gnu_mcount_nc" },
};

MODULE_INFO(depends, "");

MODULE_ALIAS("spi:encj2860");

MODULE_INFO(srcversion, "1FE68BD2E5204D744D81F0B");
