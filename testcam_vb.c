#include "testcam_draw.h"
#include "testcam.h"

#define TESTCAM_MAX_VIDEO_BUFFERS 32

int testcam_streaming_task(void *arg);
static void testcam_buf_queue_return_buffers(struct testcam *dev, enum vb2_buffer_state state);

static int testcam_queue_setup(struct vb2_queue *vq, unsigned int *nbuffers, unsigned int *nplanes, unsigned int sizes[], struct device *alloc_devs[])
{
	struct testcam *dev = vb2_get_drv_priv(vq);
	if (*nplanes) {
		return *nplanes != 1 || sizes[0] < dev->fmt.sizeimage ? -EINVAL : 0;
	}
	
	*nplanes = 1;
	sizes[0] = dev->fmt.sizeimage;
	
	if (*nbuffers > TESTCAM_MAX_VIDEO_BUFFERS) {
		*nbuffers = TESTCAM_MAX_VIDEO_BUFFERS;
	}

	return 0;
}

static int testcam_buffer_prepare(struct vb2_buffer *vb)
{
	struct testcam *dev = vb2_get_drv_priv(vb->vb2_queue);
	
	if (vb2_plane_size(vb, 0) < dev->fmt.sizeimage) {
		TESTCAM_LOG("Buffer too small");
		return -EINVAL;
	}

	return 0;
}

static void testcam_buffer_queue(struct vb2_buffer *vb)
{
	struct testcam *dev = vb2_get_drv_priv(vb->vb2_queue);
	struct testcam_buffer *buf = container_of(vb, struct testcam_buffer, vb);

	spin_lock(&dev->qlock);
	list_add_tail(&buf->list, &dev->buf_queue);
	spin_unlock(&dev->qlock);
}


static int testcam_start_streaming(struct vb2_queue *vq, unsigned int count)
{
	struct testcam *dev = vb2_get_drv_priv(vq);
	int err = 0;

	err = testcam_init_draw("VGA8x8", dev->fmt.pixelformat);
	if (err) {
		return err;
	}

	dev->streaming_thread =  kthread_run(testcam_streaming_task, dev, "testcam_streaming_thread");

	if (IS_ERR(dev->streaming_thread)) {
		err = PTR_ERR(dev->streaming_thread);
		dev->streaming_thread = NULL;
		testcam_buf_queue_return_buffers(dev, VB2_BUF_STATE_QUEUED);
		return err;
	}

	return 0;
}

static void testcam_stop_streaming(struct vb2_queue *vq)
{
	struct testcam *dev = vb2_get_drv_priv(vq);
	kthread_stop(dev->streaming_thread);
	testcam_buf_queue_return_buffers(dev, VB2_BUF_STATE_ERROR);
}

int testcam_streaming_task(void *arg)
{
	struct testcam *dev = arg;
	struct list_head *q = &dev->buf_queue;
	struct testcam_buffer *cur_buf;

	while (!kthread_should_stop()) {	
		spin_lock(&dev->qlock);
		if (!list_empty(q)) {
			cur_buf = list_first_entry(q, struct testcam_buffer, list);
			list_del(&cur_buf->list);
		}
		spin_unlock(&dev->qlock);
		void *vaddr = vb2_plane_vaddr(&cur_buf->vb, 0);
		if (vaddr) {
			testcam_draw_bg(dev->fmt.sizeimage, vaddr);
			testcam_draw_string("TESTCAM", vaddr, dev->fmt.width);
			vb2_set_plane_payload(&cur_buf->vb, 0, dev->fmt.sizeimage);
			vb2_buffer_done(&cur_buf->vb, VB2_BUF_STATE_DONE);
		}

		wait_queue_head_t wait;
		init_waitqueue_head(&wait);
		wait_event_interruptible_timeout(wait, kthread_should_stop(), 100);	
	}

	return 0;
}

static void testcam_buf_queue_return_buffers(struct testcam *dev, enum vb2_buffer_state state)
{
	struct list_head *q = &dev->buf_queue;
	struct testcam_buffer *pos, *n;

	spin_lock(&dev->qlock);
	list_for_each_entry_safe(pos, n, q, list) {
		list_del(&pos->list);
		vb2_buffer_done(&pos->vb, state);
	}
	spin_unlock(&dev->qlock);
}

const struct vb2_ops testcam_queue_qops = {
	.queue_setup = testcam_queue_setup,
	.buf_prepare = testcam_buffer_prepare,
	.buf_queue = testcam_buffer_queue,
	.start_streaming = testcam_start_streaming,
	.stop_streaming = testcam_stop_streaming,
};

