#include "espcam_video.h"

static int espcam_s_ctrl(struct v4l2_ctrl *ctrl)
{
	switch(ctrl->id) {
		case V4L2_CID_BRIGHTNESS:
		case V4L2_CID_CONTRAST:
		case V4L2_CID_SATURATION:

	}

	return 0;
}

static const struct v4l2_ctrl_ops espcam_ctrl_ops = {
	.s_ctrl = espcam_s_ctrl,
};

int espcam_create_controls(struct espcam_video *dev)
{
	int ret = 0;
	struct v4l2_ctrl_handler *ctrl_handler = &dev->ctrl_handler;

	v4l2_ctrl_handler_init(ctrl_handler, 3);
	v4l2_ctrl_new_std(ctrl_handler, &espcam_ctrl_ops, V4L2_CID_BRIGHTNESS,0, 2, 1, 1);
	v4l2_ctrl_new_std(ctrl_handler, &espcam_ctrl_ops, V4L2_CID_CONTRAST, 0, 2, 1, 1);
	v4l2_ctrl_new_std(ctrl_handler, &espcam_ctrl_ops, V4L2_CID_SATURATION, 0, 2, 1, 1);
	
	ret = ctrl_handler->error;
	if (ret) {
		v4l2_ctrl_handler_free(ctrl_handler);
	}

	return ret;
}

void espcam_free_controls(struct espcam_video *dev)
{
	v4l2_ctrl_handler_free(&dev->ctrl_handler);
}



