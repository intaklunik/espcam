#include <linux/sysfs.h>
#include <linux/kobject.h>
#include <linux/device.h>
#include "regs.h"

static ssize_t espcam_sysfs_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	
	struct dev_ext_attribute *ea;
	struct espcam_context *espcam_ctx = dev_get_drvdata(dev);

	ea = container_of(attr, struct dev_ext_attribute, attr);
	unsigned int reg = ea->var;
	
	return sysfs_emit(buf, "%u\n", reg);
}


static ssize_t espcam_sysfs_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	
	struct dev_ext_attribute *ea;
	struct espcam_context *espcam_ctx = dev_get_drvdata(dev);
	ea = container_of(attr, struct dev_ext_attribute, attr);
	unsigned int reg = ea->var;
	unsigned int reg_val;
	
	if (kstrtoint(buf, 0, &reg_val) < 0) {
		return 0;
	}
	

	pr_info("reg_val: %u", reg_val);

	return count;
}

#define ESPCAM_ATTR(name, mode, show, store, reg) \
	static struct dev_ext_attribute espcam_dev_attr_##name = { \	
		.attr = __ATTR(name, mode, show, store), \
	  	.var = (void *) reg, \
	} 

#define ESPCAM_ATTR_RO(name, func, reg) \
	ESPCAM_ATTR(name, 0444, func##_show, NULL, reg)

#define ESPCAM_ATTR_RW(name, func, reg) \
	ESPCAM_ATTR(name, 0644, func##_show, func##_store, reg)


ESPCAM_ATTR_RO(id, espcam_sysfs, APP_ID_REG);
ESPCAM_ATTR_RO(version, espcam_sysfs, APP_VERSION_REG);
ESPCAM_ATTR_RW(camera_stream_status, espcam_sysfs, CAMERA_STREAM_STATUS_REG);
ESPCAM_ATTR_RW(brightness, espcam_sysfs, CAM_BRIGHTNESS_REG);
ESPCAM_ATTR_RW(contrast, espcam_sysfs, CAM_CONTRAST_REG);
ESPCAM_ATTR_RW(saturation, espcam_sysfs, CAM_SATURATION_REG);

static struct attribute *espcam_attributes[] = {
	&espcam_dev_attr_id.attr.attr,
	&espcam_dev_attr_version.attr.attr,
	&espcam_dev_attr_camera_stream_status.attr.attr,
	NULL
};

static struct attribute *espcam_camera_attributes[] = {
	&espcam_dev_attr_brightness.attr.attr,
	&espcam_dev_attr_contrast.attr.attr,
	&espcam_dev_attr_saturation.attr.attr,
	NULL
};

static const struct attribute_group espcam_attribute_group = {
	.name = "app",
	.attrs = espcam_attributes,
};

static const struct attribute_group espcam_camera_attribute_group = {
	.name = "camera",
	.attrs = espcam_camera_attributes,
};

static const struct attribute_group *espcam_groups[] = {
	&espcam_attribute_group,
	&espcam_camera_attribute_group,
	NULL
};

static struct kobject *kobj_ref;

int espcam_add_groups(void)
{
	kobj_ref = kobject_create_and_add("espcam", kernel_kobj);
	return sysfs_create_groups(kobj_ref, espcam_groups);
}

void espcam_remove_groups(void)
{
	sysfs_remove_groups(kobj_ref, espcam_groups);
	kobject_put(kobj_ref);
}

