#include <linux/spi/spi.h>
#include "espcam_video.h"

#define ESPCAM_MAX_VIDEO_BUFFERS 32

int espcam_streaming_task(void *arg);
static void espcam_buf_queue_return_buffers(struct espcam_video *dev, enum vb2_buffer_state state);

static int espcam_queue_setup(struct vb2_queue *vq, unsigned int *nbuffers, unsigned int *nplanes, unsigned int sizes[], struct device *alloc_devs[])
{
	struct espcam_video *dev = vb2_get_drv_priv(vq);
	if (*nplanes) {
		return *nplanes != 1 || sizes[0] < dev->fmt.sizeimage ? -EINVAL : 0;
	}
	
	*nplanes = 1;
	sizes[0] = dev->fmt.sizeimage;
	
	if (*nbuffers > ESPCAM_MAX_VIDEO_BUFFERS) {
		*nbuffers = ESPCAM_MAX_VIDEO_BUFFERS;
	}

	return 0;
}

static int espcam_buffer_prepare(struct vb2_buffer *vb)
{
	struct espcam_video *dev = vb2_get_drv_priv(vb->vb2_queue);
	
	if (vb2_plane_size(vb, 0) < dev->fmt.sizeimage) {
		ESPCAM_LOG("Buffer too small");
		return -EINVAL;
	}

	return 0;
}

static void espcam_buffer_queue(struct vb2_buffer *vb)
{
	struct espcam_video *dev = vb2_get_drv_priv(vb->vb2_queue);
	struct espcam_buffer *buf = container_of(vb, struct espcam_buffer, vb);

	spin_lock(&dev->qlock);
	list_add_tail(&buf->list, &dev->buf_queue);
	spin_unlock(&dev->qlock);
}


static int espcam_start_streaming(struct vb2_queue *vq, unsigned int count)
{
	struct espcam_video *dev = vb2_get_drv_priv(vq);
	int err = 0;


	dev->streaming_thread =  kthread_run(espcam_streaming_task, dev, "espcam_streaming_thread");

	if (IS_ERR(dev->streaming_thread)) {
		err = PTR_ERR(dev->streaming_thread);
		dev->streaming_thread = NULL;
		espcam_buf_queue_return_buffers(dev, VB2_BUF_STATE_QUEUED);
		return err;
	}

	return 0;
}

static void espcam_stop_streaming(struct vb2_queue *vq)
{
	struct espcam_video *dev = vb2_get_drv_priv(vq);
	kthread_stop(dev->streaming_thread);
	espcam_buf_queue_return_buffers(dev, VB2_BUF_STATE_ERROR);
}

int espcam_streaming_task(void *arg)
{
	struct espcam_video *dev = arg;
	struct list_head *q = &dev->buf_queue;
	struct espcam_buffer *cur_buf;
	struct spi_device *client = dev->client;
	int ret;
	unsigned char buf[1] = {0};

	while (!kthread_should_stop()) {	
		spin_lock(&dev->qlock);
		if (!list_empty(q)) {
			cur_buf = list_first_entry(q, struct espcam_buffer, list);
			list_del(&cur_buf->list);
		}
		spin_unlock(&dev->qlock);
		void *vaddr = vb2_plane_vaddr(&cur_buf->vb, 0);
		if (vaddr) {
			memset(vaddr, 0xaa, dev->fmt.sizeimage);
			vb2_set_plane_payload(&cur_buf->vb, 0, dev->fmt.sizeimage);
			vb2_buffer_done(&cur_buf->vb, VB2_BUF_STATE_DONE);
			ret = spi_read(client, &buf[0], 1);
			if (ret) {
				pr_info("spi_read failed");
			}
		}

		wait_queue_head_t wait;
		init_waitqueue_head(&wait);
		wait_event_interruptible_timeout(wait, kthread_should_stop(), 100);	
	}

	return 0;
}

static void espcam_buf_queue_return_buffers(struct espcam_video *dev, enum vb2_buffer_state state)
{
	struct list_head *q = &dev->buf_queue;
	struct espcam_buffer *pos, *n;

	spin_lock(&dev->qlock);
	list_for_each_entry_safe(pos, n, q, list) {
		list_del(&pos->list);
		vb2_buffer_done(&pos->vb, state);
	}
	spin_unlock(&dev->qlock);
}

const struct vb2_ops espcam_queue_qops = {
	.queue_setup = espcam_queue_setup,
	.buf_prepare = espcam_buffer_prepare,
	.buf_queue = espcam_buffer_queue,
	.start_streaming = espcam_start_streaming,
	.stop_streaming = espcam_stop_streaming,
};

