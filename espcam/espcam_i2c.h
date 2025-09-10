#ifndef ESPCAM_I2C_H
#define ESPCAM_I2C_H

struct espcam_context {
	struct i2c_client *i2c_client;
	struct regmap *regmap;
};

#endif // ESPCAM_I2C_H
