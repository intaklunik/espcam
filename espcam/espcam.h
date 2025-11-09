#ifndef ESPCAM_H
#define ESPCAM_H

#include <linux/spi/spi.h>
#include <linux/i2c.h>
#include <linux/regmap.h>
#include <media/v4l2-dev.h>

struct espcam_context {
	struct i2c_client *i2c_client;
	struct regmap *regmap;
	struct spi_driver spi_driver;
	struct spi_device *spi_device;
	struct video_device *video_dev;
};

#endif // ESPCAM_H
