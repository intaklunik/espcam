#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include "espcam_sysfs.h"

static int __init espcam_init(void)
{
	return espcam_add_groups();
}

static void __exit espcam_exit(void)
{
	espcam_remove_groups();
}

module_init(espcam_init);
module_exit(espcam_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("intaklunik");
MODULE_DESCRIPTION("espcam");


