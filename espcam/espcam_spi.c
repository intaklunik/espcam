#include "espcam.h"

int espcam_register_video(struct spi_device *client);
void espcam_unregister_video(struct video_device *dev);

static const struct spi_device_id espcam_spi_spi_ids[] = {
	{"vid", 0},
	{ },
};

MODULE_DEVICE_TABLE(spi, espcam_spi_spi_ids);

static const struct of_device_id espcam_spi_ids[] = {
	{ .compatible = "espcam,vid" },
	{  }
};

MODULE_DEVICE_TABLE(of, espcam_spi_ids);

static int espcam_spi_probe(struct spi_device *client)
{
	int ret = 0;
	struct espcam_context *espcam_ctx = container_of(client->dev.driver, struct espcam_context, spi_driver.driver);
	
	espcam_ctx->video_dev = espcam_register_video(client);
	if (IS_ERR(espcam_ctx->video_dev)) {
		return PTR_ERR(espcam_ctx->video_dev);
	}

	espcam_ctx->spi_device = client;
	dev_set_drvdata(&client->dev, espcam_ctx);
	
	return ret;
}

static void espcam_spi_remove(struct spi_device *client)
{
	struct espcam_context *espcam_ctx = dev_get_drvdata(&client->dev);
	espcam_unregister_video(espcam_ctx->video_dev);
}

int espcam_spi_register(struct espcam_context *ctx)
{
	struct spi_driver *spi_driver = &ctx->spi_driver;

	spi_driver->id_table = espcam_spi_spi_ids;
	spi_driver->probe = espcam_spi_probe;
	spi_driver->remove = espcam_spi_remove;
	spi_driver->driver.name = "espcam-spi";
	spi_driver->driver.of_match_table = espcam_spi_ids;

	return spi_register_driver(spi_driver);
}

void espcam_spi_unregister(struct espcam_context *ctx)
{
	spi_unregister_driver(&ctx->spi_driver);
}

