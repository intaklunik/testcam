#ifndef TESTCAM_H
#define TESTCAM_H

#include <media/v4l2-device.h>
#include <media/videobuf2-core.h>

#define TESTCAM_LOG(msg) pr_debug("testcam" msg)

struct testcam_format {
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

struct testcam_buffer {
	struct vb2_buffer vb;
	struct list_head list;
};

struct testcam {
	struct video_device video_device;
	struct v4l2_device v4l2_dev;
	struct vb2_queue vb_queue;
	struct mutex mtx;
	struct testcam_format fmt;
	struct list_head buf_queue;
	spinlock_t qlock;
	struct task_struct *streaming_thread;
};

#endif //TESTCAM_H
