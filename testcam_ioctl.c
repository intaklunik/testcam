#include <media/videobuf2-v4l2.h>
#include <media/v4l2-ioctl.h>
#include "testcam.h"

static int testcam_vidioc_querycap(struct file *file, void *priv, struct v4l2_capability *cap)
{
	strscpy(cap->driver, "testcam", sizeof(cap->driver));
	strscpy(cap->card, "testcam card", sizeof(cap->card));
	cap->capabilities = V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_STREAMING | V4L2_CAP_DEVICE_CAPS;
	
	return 0;
}

static int testcam_vidioc_enum_fmt(struct file *file, void *fh, struct v4l2_fmtdesc *fmt)
{
	struct testcam *dev = video_drvdata(file);

	if (fmt->type != dev->fmt.type || fmt->index != 0) {
		return -EINVAL;
	}
	
	memset(fmt, 0, sizeof(*fmt));
	fmt->index = 0;
	fmt->type = dev->fmt.type;
	fmt->flags = 0;
	fmt->pixelformat = dev->fmt.pixelformat;
	strscpy(fmt->description, "RGB565", sizeof(fmt->description));

	return 0;
}

static int testcam_set_format(struct testcam *dev, struct v4l2_format *fmt)
{
	if (fmt->type != dev->fmt.type) {
		return -EINVAL;
	}

	fmt->fmt.pix.pixelformat = dev->fmt.pixelformat;
	fmt->fmt.pix.width = dev->fmt.width;
	fmt->fmt.pix.height = dev->fmt.height;
	fmt->fmt.pix.field = V4L2_FIELD_NONE;
	fmt->fmt.pix.bytesperline = dev->fmt.bytesperline;
	fmt->fmt.pix.sizeimage = dev->fmt.sizeimage;
	fmt->fmt.pix.colorspace = dev->fmt.colorspace;
	fmt->fmt.pix.xfer_func = dev->fmt.xfer_func;
	fmt->fmt.pix.ycbcr_enc = dev->fmt.ycbcr_enc;

	return 0;
}

static int testcam_vidioc_g_fmt(struct file *file, void *fh, struct v4l2_format *fmt)
{
	struct testcam *dev = video_drvdata(file);
	
	return testcam_set_format(dev, fmt);
}

static int testcam_vidioc_s_fmt(struct file *file, void *fh, struct v4l2_format *fmt)
{
	struct testcam *dev = video_drvdata(file);
	
	return testcam_set_format(dev, fmt);
}

static int testcam_vidioc_try_fmt(struct file *file, void *fh, struct v4l2_format *fmt)
{
	struct testcam *dev = video_drvdata(file);
	
	return testcam_set_format(dev, fmt);
}

static int testcam_vidioc_enum_input(struct file *file, void *fh, struct v4l2_input *input)
{
	if (input->index != 0) {
		return -EINVAL;
	}
	memset(input, 0, sizeof(*input));
	input->index = 0;
	strscpy(input->name, "testcam0", sizeof(input->name));
	input->type = V4L2_INPUT_TYPE_CAMERA;
	
	return 0;
}

static int testcam_vidioc_g_input(struct file *file, void *fh, unsigned int *input)
{
	*input = 0;

	return 0;
}

static int testcam_vidioc_g_parm(struct file *file, void *fh, struct v4l2_streamparm *parm)
{
	struct testcam *dev = video_drvdata(file);

	if (parm->type != dev->fmt.type) {
		return -EINVAL;
	}

	memset(parm, 0, sizeof(*parm));
	parm->type = dev->fmt.type;
	parm->parm.capture.capability = 0;
	parm->parm.capture.capturemode = 0;
	parm->parm.capture.timeperframe.numerator = 1;
	parm->parm.capture.timeperframe.denominator = dev->fmt.framerate;
	parm->parm.capture.extendedmode = 0;
	parm->parm.capture.readbuffers = 0;

	return 0;
}

static int testcam_vidioc_s_parm(struct file *file, void *fh, struct v4l2_streamparm *parm)
{
	struct testcam *dev = video_drvdata(file);

	if (parm->type != dev->fmt.type) {
		return -EINVAL;
	}
	
	parm->parm.capture.timeperframe.numerator = 1;
	parm->parm.capture.timeperframe.denominator = dev->fmt.framerate;

	return 0;
}

const struct v4l2_ioctl_ops testcam_ioctl_ops = {
	.vidioc_querycap = testcam_vidioc_querycap,
	.vidioc_enum_fmt_vid_cap = testcam_vidioc_enum_fmt,
	.vidioc_g_fmt_vid_cap = testcam_vidioc_g_fmt,
	.vidioc_s_fmt_vid_cap = testcam_vidioc_s_fmt,
	.vidioc_try_fmt_vid_cap = testcam_vidioc_try_fmt,
	.vidioc_reqbufs = vb2_ioctl_reqbufs,
	.vidioc_querybuf = vb2_ioctl_querybuf,
	.vidioc_qbuf = vb2_ioctl_qbuf,
	.vidioc_dqbuf = vb2_ioctl_dqbuf,
	.vidioc_streamon = vb2_ioctl_streamon,
	.vidioc_streamoff = vb2_ioctl_streamoff,
	.vidioc_enum_input = testcam_vidioc_enum_input,
	.vidioc_g_input = testcam_vidioc_g_input,
	.vidioc_g_parm = testcam_vidioc_g_parm,
	.vidioc_s_parm = testcam_vidioc_s_parm,
};

