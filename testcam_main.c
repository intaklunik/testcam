#include <linux/kernel.h>
#include <linux/module.h>
#include <media/videobuf2-vmalloc.h>
#include <media/v4l2-ioctl.h>
#include "testcam.h"

static struct testcam * testcam_dev;

extern const struct v4l2_ioctl_ops testcam_ioctl_ops;
extern const struct vb2_ops testcam_queue_qops;

static const struct v4l2_file_operations testcam_file_ops = {
	.owner = THIS_MODULE,
	.open = v4l2_fh_open,
	.release = vb2_fop_release,
	.unlocked_ioctl = video_ioctl2,
	.mmap = vb2_fop_mmap,
	.poll = vb2_fop_poll,
};

static void testcam_init_format(struct testcam *dev)
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

static int testcam_init_vbq(struct testcam *dev)
{
	struct vb2_queue *q = &dev->vb_queue;
	memset(q, 0, sizeof(struct vb2_queue));
	q->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	q->io_modes = VB2_MMAP;
	q->drv_priv = testcam_dev;
	q->ops = &testcam_queue_qops;
	q->mem_ops = &vb2_vmalloc_memops;
	q->buf_struct_size = sizeof(struct testcam_buffer);
	q->lock = &testcam_dev->mtx;
	q->timestamp_flags = V4L2_BUF_FLAG_TIMESTAMP_MONOTONIC | V4L2_BUF_FLAG_TSTAMP_SRC_EOF;
	mutex_init(&dev->mtx);

	return vb2_queue_init(q);
}

static int __init testcam_init(void)
{
	int ret = 0;
	
	testcam_dev = kzalloc(sizeof(struct testcam), GFP_KERNEL);
	if (!testcam_dev) {
		return -ENOMEM;
	}

	testcam_init_format(testcam_dev);
	
	strscpy(testcam_dev->v4l2_dev.name, "testcam", sizeof(testcam_dev->v4l2_dev.name));
	ret = v4l2_device_register(NULL, &testcam_dev->v4l2_dev);
	if (ret) {
		goto free_testcam;
	}

	ret = testcam_init_vbq(testcam_dev);
	if (ret) {
		goto unreg_v4l2;
	}

	INIT_LIST_HEAD(&testcam_dev->buf_queue);	
	spin_lock_init(&testcam_dev->qlock);
	
	struct video_device *vdev = &testcam_dev->video_device;
	strscpy(vdev->name, "testcamvdev", sizeof(vdev->name));
	vdev->v4l2_dev = &testcam_dev->v4l2_dev;
	vdev->fops = &testcam_file_ops;
	vdev->ioctl_ops = &testcam_ioctl_ops;
	vdev->release = video_device_release_empty;
	vdev->lock = &testcam_dev->mtx;
	vdev->queue = &testcam_dev->vb_queue;
	vdev->device_caps = V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_STREAMING;
	video_set_drvdata(vdev, testcam_dev);

	ret = video_register_device(vdev, VFL_TYPE_VIDEO, -1);
	if (ret < 0) {
		goto unreg_v4l2;
	}

	return 0;

unreg_v4l2:
	v4l2_device_unregister(&testcam_dev->v4l2_dev);
free_testcam:
	kfree(testcam_dev);
	return ret;
}

static void __exit testcam_exit(void)
{
	video_unregister_device(&testcam_dev->video_device);
	v4l2_device_unregister(&testcam_dev->v4l2_dev);
	kfree(testcam_dev);
}

module_init(testcam_init);
module_exit(testcam_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("intaklunik");
MODULE_DESCRIPTION("TestCam virtual video driver");
MODULE_VERSION("1.0");

