#include <linux/kernel.h>
#include <linux/module.h>
#include <media/videobuf2-vmalloc.h>
#include <media/v4l2-ioctl.h>
#include <linux/spi/spi.h>
#include "espcam.h"
#include "espcam_video.h"


extern const struct v4l2_ioctl_ops espcam_ioctl_ops;
extern const struct vb2_ops espcam_queue_qops;

int espcam_create_controls(struct espcam_video *dev);
void espcam_free_controls(struct espcam_video *dev);

static const struct v4l2_file_operations espcam_file_ops = {
	.owner = THIS_MODULE,
	.open = v4l2_fh_open,
	.release = vb2_fop_release,
	.unlocked_ioctl = video_ioctl2,
	.mmap = vb2_fop_mmap,
	.poll = vb2_fop_poll,
};

static void espcam_init_format(struct espcam_video *dev)
{
	dev->fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	dev->fmt.pixelformat = V4L2_PIX_FMT_RGB565;
	dev->fmt.width = 120;
	dev->fmt.height = 160;
	dev->fmt.bytesperline = dev->fmt.width * 2;
	dev->fmt.sizeimage = dev->fmt.bytesperline * dev->fmt.height;
	dev->fmt.colorspace = V4L2_COLORSPACE_SRGB;
	dev->fmt.ycbcr_enc = V4L2_YCBCR_ENC_DEFAULT;
	dev->fmt.xfer_func = V4L2_XFER_FUNC_SRGB;
	dev->fmt.framerate = 10;
}

static int espcam_init_vbq(struct espcam_video *dev)
{
	struct vb2_queue *q = &dev->vb_queue;
	memset(q, 0, sizeof(struct vb2_queue));
	q->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	q->io_modes = VB2_MMAP;
	q->drv_priv = dev;
	q->ops = &espcam_queue_qops;
	q->mem_ops = &vb2_vmalloc_memops;
	q->buf_struct_size = sizeof(struct espcam_buffer);
	q->lock = &dev->mtx;
	q->timestamp_flags = V4L2_BUF_FLAG_TIMESTAMP_MONOTONIC | V4L2_BUF_FLAG_TSTAMP_SRC_EOF;
	mutex_init(&dev->mtx);

	return vb2_queue_init(q);
}

struct video_device * espcam_register_video(struct spi_device *spi)
{
	int ret = 0;
	
	struct espcam_video *espcam_dev = kzalloc(sizeof(struct espcam_video), GFP_KERNEL);
	if (!espcam_dev) {
		return ERR_PTR(-ENOMEM);
	}

	espcam_init_format(espcam_dev);
	
	strscpy(espcam_dev->v4l2_dev.name, "espcam", sizeof(espcam_dev->v4l2_dev.name));
	ret = v4l2_device_register(NULL, &espcam_dev->v4l2_dev);
	if (ret) {
		goto free_espcam;
	}

	ret = espcam_init_vbq(espcam_dev);
	if (ret) {
		goto unreg_v4l2;
	}

	INIT_LIST_HEAD(&espcam_dev->buf_queue);	
	spin_lock_init(&espcam_dev->qlock);
	
	struct video_device *vdev = &espcam_dev->video_device;
	strscpy(vdev->name, "espcamvdev", sizeof(vdev->name));
	vdev->v4l2_dev = &espcam_dev->v4l2_dev;
	vdev->fops = &espcam_file_ops;
	vdev->ioctl_ops = &espcam_ioctl_ops;
	vdev->release = video_device_release_empty;
	vdev->lock = &espcam_dev->mtx;
	vdev->queue = &espcam_dev->vb_queue;
	vdev->device_caps = V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_STREAMING;
	video_set_drvdata(vdev, espcam_dev);

	ret = espcam_create_controls(espcam_dev);
	if (ret) {
		goto free_vbq;
	}

	espcam_dev->client = spi;

	ret = video_register_device(vdev, VFL_TYPE_VIDEO, -1);
	if (ret < 0) {
		goto free_ctrls;
	}


	return &espcam_dev->video_device;

free_ctrls:
	espcam_free_controls(espcam_dev);	
free_vbq:
	vb2_queue_release(&espcam_dev->vb_queue);
unreg_v4l2:
	v4l2_device_unregister(&espcam_dev->v4l2_dev);
free_espcam:
	kfree(espcam_dev);

	return ERR_PTR(ret);
}

void espcam_unregister_video(struct video_device *video_dev)
{
	struct espcam_video *espcam_dev = container_of(video_dev, struct espcam_video, video_device);

	espcam_free_controls(espcam_dev);
	mutex_lock(&espcam_dev->mtx);
	vb2_queue_release(&espcam_dev->vb_queue);
	mutex_unlock(&espcam_dev->mtx);
	video_unregister_device(&espcam_dev->video_device);
	v4l2_device_unregister(&espcam_dev->v4l2_dev);
	kfree(espcam_dev);
}


