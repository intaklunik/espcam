#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/byteorder/generic.h>
#include "regs.h"
#include "espcam.h"

extern const struct attribute_group *espcam_groups[];

int espcam_spi_register(struct espcam_context *ctx);
void espcam_spi_unregister(struct espcam_context *ctx);

static const struct regmap_range espcam_volatile_ranges[] = {
	regmap_reg_range(APP_ID_REG, CAMERA_STREAM_STATUS_REG),
	regmap_reg_range(CAM_BRIGHTNESS_REG, CAM_SATURATION_REG),
};

static const struct regmap_access_table espcam_volatile_table = {
	.yes_ranges = espcam_volatile_ranges,
	.n_yes_ranges = ARRAY_SIZE(espcam_volatile_ranges),
};

static const struct regmap_range espcam_write_ranges[] = {
	regmap_reg_range(CAMERA_STREAM_STATUS_REG, CAMERA_STREAM_STATUS_REG),
	regmap_reg_range(CAM_BRIGHTNESS_REG, CAM_SATURATION_REG),
};

static const struct regmap_access_table espcam_write_table = {
	.yes_ranges = espcam_write_ranges,
	.n_yes_ranges = ARRAY_SIZE(espcam_write_ranges),
};

static int espcam_reg_read(void *context, unsigned int reg, unsigned int *val)
{
	struct espcam_context *espcam_ctx = context;
	struct i2c_client *client = espcam_ctx->i2c_client;
	struct i2c_msg msgs[2];
	int ret;

	u16 reg_buf = cpu_to_be16(reg);
	u8 val_buf[2];

	msgs[0].addr = client->addr;
	msgs[0].flags = 0;
	msgs[0].buf = (u8 *) &reg_buf;
	msgs[0].len = 2;

	msgs[1].addr = client->addr;
	msgs[1].flags = I2C_M_RD;
	msgs[1].buf = val_buf;
	msgs[1].len = 1;
	
	ret = i2c_transfer(client->adapter, &msgs[0], 1);
	ret = i2c_transfer(client->adapter, &msgs[1], 1);
	
	*val = val_buf[0];

	return 0;
}

static inline bool reg_in_range(unsigned int reg, unsigned int val) 
{
	switch(reg) {
		case CAMERA_STREAM_STATUS_REG:
			return val >= 0 && val <= 1;
		case CAM_BRIGHTNESS_REG:
		case CAM_CONTRAST_REG:
		case CAM_SATURATION_REG:
			return (int) val >= -2 && (int) val <= 2;
		default:
			return false;
	}

	return false;
}

static int espcam_reg_write(void *context, unsigned int reg, unsigned int val)
{
	if (!reg_in_range(reg, val)) {
		//return 0;		
	}

	struct espcam_context *espcam_ctx = context;
	struct i2c_client *client = espcam_ctx->i2c_client;
	struct i2c_msg msg;
	int ret;

	u8 val_buf[4];
	val_buf[0] = (reg >> 8) & 0xff;
	val_buf[1] = reg & 0xff;
	val_buf[2] = val;

	msg.addr = client->addr;
	msg.flags = 0;
	msg.buf = val_buf;
	msg.len = 3;

	ret = i2c_transfer(client->adapter, &msg, 1);
	
	return 0;
}

static const struct regmap_config espcam_regmap_config = {
	.reg_bits = 16,
	.val_bits = 16,
	.wr_table = &espcam_write_table,
	.volatile_table = &espcam_volatile_table,
	.cache_type = REGCACHE_MAPLE,
	.reg_read = espcam_reg_read,
	.reg_write = espcam_reg_write,
};

static int espcam_i2c_probe(struct i2c_client *client)
{
	int ret;
	struct device *dev = &client->dev;
	struct espcam_context *espcam_ctx;

	espcam_ctx = kzalloc(sizeof(struct espcam_context), GFP_KERNEL);
	if (!espcam_ctx) {
		return -ENOMEM;
	}

	espcam_ctx->i2c_client = client;
	dev_set_drvdata(dev, espcam_ctx);

	espcam_ctx->regmap = devm_regmap_init(dev, NULL, espcam_ctx, &espcam_regmap_config);
	if (IS_ERR(espcam_ctx->regmap)) {
		return PTR_ERR(espcam_ctx->regmap);
	}

	ret = espcam_spi_register(espcam_ctx);


	return 0;
}

static void espcam_i2c_remove(struct i2c_client *client)
{
	struct espcam_context *espcam_ctx = dev_get_drvdata(&client->dev);
	espcam_spi_unregister(espcam_ctx);

	kfree(espcam_ctx);
}

static struct of_device_id espcam_i2c_ids[] = {
	{ .compatible = "espcam,espcam0" },
	{  }
};

MODULE_DEVICE_TABLE(of, espcam_i2c_ids);

static struct i2c_device_id espcam_i2c_device_ids[] = {
	{ "espcam", 0 },
	{  }	
};

MODULE_DEVICE_TABLE(i2c, espcam_i2c_device_ids);

static struct i2c_driver espcam_i2c_driver = {
	.driver = {
		.name = "espcam",
		.owner = THIS_MODULE,
		.of_match_table = espcam_i2c_ids,
		.dev_groups = espcam_groups,
	},
	.probe = espcam_i2c_probe,
	.remove = espcam_i2c_remove,
	.id_table = espcam_i2c_device_ids,
};

module_i2c_driver(espcam_i2c_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("intaklunik");
MODULE_DESCRIPTION("espcam");


