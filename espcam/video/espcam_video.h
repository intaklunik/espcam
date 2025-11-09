#ifndef ESPCAM_VIDEO_H
#define ESPCAM_VIDEO_H

#include <media/v4l2-device.h>
#include <media/videobuf2-core.h>
#include <media/v4l2-ctrls.h>

#define ESPCAM_LOG(msg) pr_debug("espcam" msg)

struct espcam_format {
	int width;
	int height;
	int sizeimage;
	enum v4l2_buf_type type;
	unsigned int pixelformat;
	unsigned int bytesperline;
	enum v4l2_colorspace colorspace;
	enum v4l2_ycbcr_encoding ycbcr_enc;
	enum v4l2_xfer_func xfer_func;
	unsigned int framerate;
};

struct espcam_buffer {
	struct vb2_buffer vb;
	struct list_head list;
};

struct espcam_video {
	struct video_device video_device;
	struct v4l2_device v4l2_dev;
	struct v4l2_ctrl_handler ctrl_handler;
	struct vb2_queue vb_queue;
	struct mutex mtx;
	struct espcam_format fmt;
	struct list_head buf_queue;
	spinlock_t qlock;
	struct task_struct *streaming_thread;
	struct spi_device *client;
};


int espcam_create_controls(struct espcam_video *dev);
void espcam_free_controls(struct espcam_video *dev);

#endif //ESPCAM_VIDEO_H
