/***************************************************************************
 *  V4L2Device.cpp - Low level acquisition class for Video4Linux2 API
 *
 *  Thu Mar 6 10:06:23 2002
 *  Copyright  2002  Christophe Seyve - SISELL
 *  christophe.seyve@sisell.com
 ****************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "V4L2Device.h"
#include "videocapture.h"
#include "VirtualDevice.h"

// additionnal libraries for memory/device control
#include <assert.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <pthread.h>
#include <errno.h>

u8 g_debug_V4L2Device = 0;

extern "C" {
#include "v4l2uvc.h"
}

#define g_debug_V4L2Device	0
#define V4L2_printf(...)	{fprintf(stderr,"[V4L2 '%s']::%s:%d",videodevice,__func__,__LINE__);fprintf(stderr,__VA_ARGS__);fflush(stderr);}

#define CLEAR(x) memset (&(x), 0, sizeof (x))
static int xioctl(int fd, int request, void * arg)
{
	int r;
	
	do {
		r = ioctl(fd, request, arg);
	} while (-1 == r && EINTR == errno);
	
	return r;
}
static void
errno_exit                      (const char *           s)
{
	fprintf (stderr, "%s error %d, %s\n",
			 s, errno, strerror (errno));

	exit (EXIT_FAILURE);
}


V4L2Device::V4L2Device(int dev) :
	m_idx_device(dev)
{
	fprintf(stderr, "[V4L2]::%s:%d : constructor V4L2 device %d\n",
			__func__, __LINE__, dev); fflush(stderr);
	
	init();

	if(dev>=0) {
		videodevice = new char [32];
		sprintf(videodevice, "V4L2-%d", dev);
	}
}

V4L2Device::~V4L2Device() 
{
	if(initialised) {
		VDclose();
	}
	if(videodevice) {
		delete [] videodevice;
		videodevice = NULL;
	}

	swReleaseImage(&m_iplImageRGB32);
	swReleaseImage(&m_iplImageY);
}


void V4L2Device::init() 
{
	initialised = false;
	m_imageSize.width = m_imageSize.height = 0;
	m_nChannels = 4;

	memset(&m_video_properties, 0, sizeof(t_video_properties));

	rawframebuffer = NULL;

	m_isPWC = false;
	// vd.fd = -1 until is done correctly...
	memset(&vd, 0, sizeof(v4l2device));
	
	vd.fd = -1;
	vd.frame = 0;
	
	videodevice = NULL;
	// params defaults
	norm = VIDEO_MODE_PAL;
	channel = 0;
	m_capture_palette = -1;
	frequency_table = 0;
	TVchannel = 0;

	buffers = NULL;
	m_iplImageY = m_iplImageRGB32 = NULL;

	tBoxSize newSize;
	newSize.width = 640;
	newSize.height = 480;

	char device[32];
	sprintf(device, "/dev/video%d", m_idx_device);


	/*  Opens device with size
		device device name (ex.: "/dev/video0")
		newSize acquisition size
	*/
	int ret = VDopen(device, &newSize);
	if(ret < 0) {
		fprintf(stderr, "V4L2::%s:%d : VDOpen(%s, %dx%d) failed.\n",
				__func__, __LINE__,  videodevice,
				(int)newSize.width, (int)newSize.height);
		return;
	}

	fprintf(stderr, "V4L2::%s:%d : VDOpen(%s, %dx%d) successed.\n",
			__func__, __LINE__,
			device,
			(int)newSize.width, (int)newSize.height);

	videodevice = new char [strlen(device)+1];
	strcpy(videodevice, device);
}











/**************************************************************************
  Virtual interface Specific section
  *************************************************************************/

/** \brief Use sequential mode
	If true, grabbing is done when a new image is requested.
	Else a thread is started to grab */
void V4L2Device::setSequentialMode(bool on)
{

}


/** \brief Start acquisition */
int V4L2Device::startAcquisition()
{
	int retval = start_capturing();

	updateVideoProperties();

	return retval;
}

/* Return image size */
CvSize V4L2Device::getImageSize()
{
	return m_imageSize;
}


int V4L2Device::grab()
{
	// Returns frame buffer adress
	rawframebuffer = getFrameBuffer();

	if(!rawframebuffer)
	{
		fprintf(stderr, "[V4L2]::%s:%d : no acq !\n", __func__, __LINE__);
		return -1;
	}

	//fprintf(stderr, "[V4L2]::%s:%d : acq ok %p !\n", __func__, __LINE__, rawframebuffer);
	return 0;

}

/** @brief Update and return video properties */
t_video_properties V4L2Device::updateVideoProperties()
{
	if(!initialised) {
		memset(&m_video_properties, 0, sizeof(t_video_properties));
		return m_video_properties;
	}

	video_picture pic;
	getpicture(&pic);
#define VIDEOPICTURE_SCALE	1.
	m_video_properties.brightness =	pic.brightness / VIDEOPICTURE_SCALE;
	m_video_properties.contrast = pic.contrast / VIDEOPICTURE_SCALE;
	m_video_properties.saturation = pic.colour / VIDEOPICTURE_SCALE;
	m_video_properties.hue = pic.hue / VIDEOPICTURE_SCALE;
	m_video_properties.white_balance = pic.whiteness / VIDEOPICTURE_SCALE;

#if 0 /// \bug FIXME : implement this
		//	m_video_properties. = cvGetCaptureProperty(m_capture, );
	m_video_properties.pos_msec =		cvGetCaptureProperty(m_capture, CV_CAP_PROP_POS_MSEC );/*Film current position in milliseconds or video capture timestamp */
	m_video_properties.pos_frames =		cvGetCaptureProperty(m_capture, CV_CAP_PROP_POS_FRAMES );/*0-based index of the frame to be decoded/captured next */
	m_video_properties.pos_avi_ratio =	cvGetCaptureProperty(m_capture, CV_CAP_PROP_POS_AVI_RATIO );/*Relative position of the video file (0 - start of the film, 1 - end of the film) */
	m_video_properties.frame_width =	cvGetCaptureProperty(m_capture, CV_CAP_PROP_FRAME_WIDTH );/*Width of the frames in the video stream */
	m_video_properties.frame_height =	cvGetCaptureProperty(m_capture, CV_CAP_PROP_FRAME_HEIGHT );/*Height of the frames in the video stream */
	m_video_properties.fps =			cvGetCaptureProperty(m_capture, CV_CAP_PROP_FPS );/*Frame rate */
	m_video_properties.fourcc_dble =	cvGetCaptureProperty(m_capture, CV_CAP_PROP_FOURCC );/*4-character code of codec */
//		char   fourcc[5] =	cvGetCaptureProperty(m_capture, FOURCC coding CV_CAP_PROP_FOURCC 4-character code of codec */
//		char   norm[8] =		cvGetCaptureProperty(m_capture, Norm: pal, ntsc, secam */
	m_video_properties.frame_count =	cvGetCaptureProperty(m_capture, CV_CAP_PROP_FRAME_COUNT );/*Number of frames in the video file */
	m_video_properties.format =			cvGetCaptureProperty(m_capture, CV_CAP_PROP_FORMAT );/*The format of the Mat objects returned by retrieve() */
	m_video_properties.mode =			cvGetCaptureProperty(m_capture, CV_CAP_PROP_MODE );/*A backend-specific value indicating the current capture mode */
	m_video_properties.brightness =		cvGetCaptureProperty(m_capture, CV_CAP_PROP_BRIGHTNESS );/*Brightness of the image (only for cameras) */
	m_video_properties.contrast =		cvGetCaptureProperty(m_capture, CV_CAP_PROP_CONTRAST );/*Contrast of the image (only for cameras) */
	m_video_properties.saturation =		cvGetCaptureProperty(m_capture, CV_CAP_PROP_SATURATION );/*Saturation of the image (only for cameras) */
	m_video_properties.hue =			cvGetCaptureProperty(m_capture, CV_CAP_PROP_HUE );/*Hue of the image (only for cameras) */
	m_video_properties.gain =			cvGetCaptureProperty(m_capture, CV_CAP_PROP_GAIN );/*Gain of the image (only for cameras) */
	m_video_properties.exposure =		cvGetCaptureProperty(m_capture, CV_CAP_PROP_EXPOSURE );/*Exposure (only for cameras) */
	m_video_properties.convert_rgb =	cvGetCaptureProperty(m_capture, CV_CAP_PROP_CONVERT_RGB );/*Boolean flags indicating whether images should be converted to RGB */
	m_video_properties.white_balance = cvGetCaptureProperty(m_capture, CV_CAP_PROP_WHITE_BALANCE );/*Currently unsupported */
#endif
		/*! CV_CAP_PROP_RECTIFICATION TOWRITE (note: only supported by DC1394 v 2.x backend currently)
		– Property identifier. Can be one of the following:*/

	fprintf(stderr, "V4L2Device::%s:%d : props=\n", __func__, __LINE__);
	printVideoProperties(&m_video_properties);

	return m_video_properties;
}

/** @brief Get video properties (not updated) */
t_video_properties V4L2Device::getVideoProperties()
{
	return m_video_properties;
}
int V4L2Device::setVideoProperties(t_video_properties props)
{
	/// \bug FIXME : implement here
	if(m_video_properties.brightness != props.brightness
			|| m_video_properties.contrast != props.contrast
			|| m_video_properties.saturation != props.saturation
			|| m_video_properties.hue != props.hue
			|| m_video_properties.white_balance != props.white_balance
			)
	{
		fprintf(stderr, "[V4L2]::%s:%d : change br=%g/contrast=%g/saturation=%g/hue=%g/whiteness=%g...\n", __func__, __LINE__,
				props.brightness, props.contrast, props.saturation, props.hue, props.white_balance);
		video_picture pic;
		pic.brightness = (u16)(props.brightness*VIDEOPICTURE_SCALE);
		pic.contrast = (u16)(props.contrast*VIDEOPICTURE_SCALE);
		pic.colour = (u16)(props.saturation*VIDEOPICTURE_SCALE);
		pic.hue = (u16)(props.hue*VIDEOPICTURE_SCALE);
		pic.whiteness = (u16)(props.white_balance*VIDEOPICTURE_SCALE);
		setpicture(pic.brightness, pic.hue, pic.colour, pic.contrast, pic.whiteness);
	}

	fprintf(stderr, "V4L2Device::%s:%d : props=\n", __func__, __LINE__);
	m_video_properties = props;
	printVideoProperties(&m_video_properties);

	return 0;
}

IplImage * V4L2Device::readImageRGB32()
{
	if(!rawframebuffer) { return NULL; }
	if(!m_iplImageRGB32) {
		m_iplImageRGB32 = swCreateImage(m_imageSize, IPL_DEPTH_8U,
								   4);
	}

	// Convert into BGR32
	unsigned char * image = (unsigned char *)m_iplImageRGB32->imageData;

	// if needed, conversion to RGB32 coding
	if(m_capture_palette == VIDEO_PALETTE_RGB32) {
		memcpy(image,  rawframebuffer, m_imageSize.width*m_imageSize.height*4);

		return 0;
	}
//		fprintf(stderr, "%s:%d : convert2RGB32...\n", __func__, __LINE__);
	convert2RGB32(rawframebuffer, image);


	return m_iplImageRGB32;
}

IplImage * V4L2Device::readImageY()
{
	if(!rawframebuffer) { return NULL; }
	if(!m_iplImageY) {
		m_iplImageY = swCreateImage(m_imageSize, IPL_DEPTH_8U,
								   1);
	}
	// Convert into BGR32
	unsigned char * image = (unsigned char *)m_iplImageRGB32->imageData;

	// if needed, conversion to RGB32 coding
	if(m_capture_palette == VIDEO_PALETTE_GREY) {
		memcpy(image,  rawframebuffer, m_imageSize.width*m_imageSize.height);

		return 0;
	}
	fprintf(stderr, "%s:%d : convert2Y...\n", __func__, __LINE__);
	convert2Y(rawframebuffer, image);

	return m_iplImageY;
}


int V4L2Device::convert2RGB32(unsigned char * src, unsigned char * dest)
{
	unsigned char *dy, *du, *dv;
	int i;
	int n = m_imageSize.width * m_imageSize.height;
//	fprintf(stderr, "[V4L2]::%s:%d : convert from pal = %d\n", __func__, __LINE__,
//			m_capture_palette
//			);
	// Conversion RGB 32...certainement à compléter !
	switch (m_capture_palette)
	{
	case VIDEO_PALETTE_RGB32:
		/* cool... */
		memcpy(dest, src, n * 4);
		break;

	case VIDEO_PALETTE_RGB24:
		for (i = 0; i < n; i++)
		{
			dest[0] = src[0];
			dest[1] = src[1];
			dest[2] = src[2];
			src += 3;
			dest += 4;
		}
		break;

	case VIDEO_PALETTE_YUV420P:
		// Base format with Philips webcams (at least the PCVC 740K = ToUCam PRO)
		dy = src;
		du = src + n;
		dv = du + (n / 4);

		ccvt_420p_bgr32(m_imageSize.width, m_imageSize.height, dy, du, dv, dest);
		break;

	case VIDEO_PALETTE_YUV420:
		// Non planar format... Hmm...
		ccvt_420i_bgr32(m_imageSize.width, m_imageSize.height, src, dest);
		break;

	case VIDEO_PALETTE_YUYV:
		// Hmm. CPiA cam has this nativly. Anyway, it's just another format. The last one, as far as I'm concerned ;)
		/* we need to work this one out... */
		ccvt_yuyv_bgr32(m_imageSize.width, m_imageSize.height, src, dest);
		break;
	}
	return 0;
}


int V4L2Device::convert2Y(unsigned char * src, unsigned char * dest)
{
   //dst = RGB[CurBuffer].bits();
   //dy = Y[CurBuffer].bits();
   //du = U[CurBuffer].bits();
   //dv = V[CurBuffer].bits();
	unsigned char *dy, *du, *dv;
	int i;
	int n = m_imageSize.width*m_imageSize.height;

	// Conversion RGB 32...certainement à compléter !
	switch (m_capture_palette)
	{
	case VIDEO_PALETTE_YUV420:
		// Non planar format... Hmm...
	case VIDEO_PALETTE_RGB32:
		/* cool... */
		readImageRGB32(); // to convert to BGRA

		// then convert to Y
		cvCvtColor(m_iplImageRGB32, m_iplImageY, CV_BGRA2GRAY);
		break;

//		case VIDEO_PALETTE_RGB24:
//			for (i = 0; i < n; i++)
//			{
//				dest[0] = src[1]; /// \bug FIXME use only green
//				src += 3;
//				dest ++;
//			}
//			break;

	case VIDEO_PALETTE_YUV420P:
		// Base format with Philips webcams (at least the PCVC 740K = ToUCam PRO)
		memcpy(dest, src, m_imageSize.width * m_imageSize.height);
		break;
	}

	return 0;
}
/* Stop acquisition */
int V4L2Device::stopAcquisition()
{

	return VDclose();
}

/**************************************************************************
  V4L2 Specific section
  *************************************************************************/

int V4L2Device::init_mmap() {
	struct v4l2_requestbuffers req;
	
	CLEAR (req);

	req.count               = 4;
	req.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory              = V4L2_MEMORY_MMAP;

	if (-1 == xioctl(vd.fd, VIDIOC_REQBUFS, &req)) {
		if (EINVAL == errno) {
			fprintf (stderr, "%s does not support "
					 "memory mapping\n", videodevice);
			exit (EXIT_FAILURE);
		} else {
			errno_exit ("VIDIOC_REQBUFS");
		}
	}

	if (req.count < 2) {
		fprintf (stderr, "Insufficient buffer memory on %s\n",
				 videodevice);
		exit (EXIT_FAILURE);
	}

	buffers = (t_v4l2_buffer *)calloc(req.count, sizeof (*buffers));

	if (!buffers) {
		fprintf (stderr, "Out of memory\n");
		exit (EXIT_FAILURE);
	}

	for (n_buffers = 0; n_buffers < req.count; ++n_buffers) {
		struct v4l2_buffer buf;

		CLEAR (buf);

		buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory      = V4L2_MEMORY_MMAP;
		buf.index       = n_buffers;

		if (-1 == xioctl(vd.fd, VIDIOC_QUERYBUF, &buf))
				errno_exit ("VIDIOC_QUERYBUF");

		buffers[n_buffers].length = buf.length;
		buffers[n_buffers].start = 
				mmap (NULL /* start anywhere */,
					  buf.length,
					  PROT_READ | PROT_WRITE /* required */,
					  MAP_SHARED /* recommended */,
					  vd.fd, buf.m.offset);
		fprintf(stderr, "\tV4L2::%s:%d : buffers[n_buffers=%u].start = %p length=%d\n", 
					__func__, __LINE__, n_buffers, buffers[n_buffers].start, buffers[n_buffers].length); 
		if (MAP_FAILED == buffers[n_buffers].start) {
			errno_exit ("mmap");
		}
	}
	
	return 0;
}


int V4L2Device::start_capturing() {

	/* set framerate */
	struct v4l2_streamparm* setfps;  
	setfps=(struct v4l2_streamparm *) calloc(1, sizeof(struct v4l2_streamparm));
	
	memset(setfps, 0, sizeof(struct v4l2_streamparm));
	setfps->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	
	setfps->parm.capture.timeperframe.numerator = 1;
	setfps->parm.capture.timeperframe.denominator = vd.frame_rate;

	int ret = ioctl(vd.fd, VIDIOC_S_PARM, setfps); 
	if(ret == -1) {
		perror("Unable to set frame rate");
	}
	
	ret = ioctl(vd.fd, VIDIOC_G_PARM, setfps); 
	if(ret == 0) {
		if (setfps->parm.capture.timeperframe.numerator != 1 ||
			setfps->parm.capture.timeperframe.denominator != vd.frame_rate) {
			V4L2_printf("  Frame rate:   %u/%u fps (requested frame rate %u fps is "
				"not supported by device)\n",
				setfps->parm.capture.timeperframe.denominator,
				setfps->parm.capture.timeperframe.numerator,
				vd.frame_rate);
		}
		else {
			V4L2_printf("  Frame rate:   %d fps\n", vd.frame_rate);
		}
	}
	else {
		perror("Unable to read out current frame rate");
		
	}
	
	
	
	
	
	
	
	unsigned int i;
	enum v4l2_buf_type type;
	for (i = 0; i < n_buffers; ++i) {
		struct v4l2_buffer buf;
		
		CLEAR (buf);
		
		buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory      = V4L2_MEMORY_MMAP;
		buf.index       = i;
		
		
		if (-1 == xioctl(vd.fd, VIDIOC_QBUF, &buf)) {
			int errnum = errno;
			fprintf(stderr, "[V4L2]::%s:%d : error in xioctl(vd.fd, VIDIOC_QBUF, &buf) i=%d err=%d='%s'\n", 
						__func__, __LINE__, i, errnum, strerror(errnum));
		}
	}
	
	
	// START CAPTURE
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	
	if (-1 == xioctl(vd.fd, VIDIOC_STREAMON, &type)) {
		int errnum = errno;
		fprintf(stderr, "[V4L2]::%s:%d : error in xioctl(vd.fd, VIDIOC_STREAMON, &buf) err=%d=%s\n", 
						__func__, __LINE__, errnum, strerror(errnum));
	}
	return 0;
}

#define MAXWIDTH 2536
#define MAXHEIGHT 1485
#define MINWIDTH 160
#define MINHEIGHT 120

int V4L2Device::changeSize(tBoxSize * newSize) 
{
	if(!newSize) return -1;
	int w=0, h=0;
	
	if(newSize->width <= 0 || newSize->height <= 0) {
		w = DEFAULT_VIDEO_WIDTH;
		h = DEFAULT_VIDEO_HEIGHT;
	}
	else {
		w = newSize->width;
		h = newSize->height;
	}
	fprintf(stderr, "\t\t[V4L2]::%s:%d : capturing size is set to %dx%d.\n", __func__, __LINE__, w, h);
	
	if(w > MAXWIDTH || h > MAXHEIGHT) {
		w = MAXWIDTH;
		h = MAXHEIGHT;
		fprintf(stderr, "\t\t[V4L2]::%s:%d : capturing size is set to %dx%d.\n", __func__, __LINE__, w, h);
	} else if(w < MINWIDTH || h < MINHEIGHT) {
			w = MINWIDTH;
			h = MINHEIGHT;
			fprintf(stderr, "\t\t[V4L2]::%s:%d : capturing size is set to %dx%d.\n", __func__, __LINE__, w, h);
		}
	init_device(w,h);

	// Init memory map
    init_mmap();
	
	fprintf(stderr, "[V4L2]::%s:%d: size=%dx%d \n", __func__, __LINE__, w, h);
	
	video_width = w;
	video_height = h;
	video_area = video_width * video_height;

	m_imageSize = cvSize(w,h);

	// tell caller that the size changed
	newSize->width  = w;
	newSize->height = h;
	
	unsigned char first = 1;
	if(buffers) {
		first = 0;
		stop_capturing();
	}
	
	start_capturing();
	
	return 0;
}



int V4L2Device::init_device(int w, int h) {
	// From http://v4l2spec.bytesex.org/spec/a12020.htm
	struct v4l2_capability cap;
    struct v4l2_cropcap cropcap;
    struct v4l2_crop crop;
    struct v4l2_format fmt;
	
    if (-1 == xioctl(vd.fd, VIDIOC_QUERYCAP, &cap)) {
		if (EINVAL == errno) {
			fprintf (stderr, "V4L::%s:%d %s is no V4L2 device\n",__func__, __LINE__,
				videodevice);
			exit (EXIT_FAILURE);
		} else {
			fprintf (stderr, "V4L2::%s:%d : xioctl(vd.fd=%d, VIDIOC_QUERYCAP, &cap) FAILED\n",
				__func__, __LINE__, vd.fd);
			errno_exit ("VIDIOC_QUERYCAP");
		}
	}

	if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
		fprintf (stderr, "%s is no video capture device\n",
			videodevice);
		exit (EXIT_FAILURE);
	}

	if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
		fprintf (stderr, "%s does not support streaming i/o\n",
			videodevice);
		exit (EXIT_FAILURE);
	}

	/* Select video input, video standard and tune here. */
	CLEAR (cropcap);
	
	cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	
	if (0 == xioctl(vd.fd, VIDIOC_CROPCAP, &cropcap)) {
		crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        crop.c = cropcap.defrect; /* reset to default */
		
        if (-1 == xioctl(vd.fd, VIDIOC_S_CROP, &crop)) {
			int err_num = errno;
			switch (err_num) {
			case EINVAL:
				/* Cropping not supported. */
				fprintf(stderr, "V4L2::%s:%d : Cropping not supported.\n", __func__, __LINE__);
				break;
			default:
 				fprintf(stderr, "V4L2::%s:%d : Error err_num=%d ignored.\n", __func__, __LINE__, err_num);
			   /* Errors ignored. */
				break;
			}
		}
	} else {
		/* Errors ignored. */
		fprintf(stderr, "V4L2::%s:%d :xioctl failed.\n", __func__, __LINE__);
	}


	CLEAR (fmt);

	fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width       = w; 
	fmt.fmt.pix.height      = h;
	fmt.fmt.pix.field       = V4L2_FIELD_ANY;//V4L2_FIELD_INTERLACED;

	/*
	#define v4l2_fourcc(a,b,c,d)\
        (((__u32)(a)<<0)|((__u32)(b)<<8)|((__u32)(c)<<16)|((__u32)(d)<<24))
	*/
	
	int retry_format = 0;
#define MAX_PIXELFORMATS 6
	__u32 pixelformats[MAX_PIXELFORMATS] = {
			V4L2_PIX_FMT_YUYV,
			V4L2_PIX_FMT_GREY,
			V4L2_PIX_FMT_YVU420,
			V4L2_PIX_FMT_RGB24,
			V4L2_PIX_FMT_RGB32 ,
			V4L2_PIX_FMT_YUV420};
	
	
	bool format_ok = false;
	while(!format_ok && retry_format<MAX_PIXELFORMATS) {
		fmt.fmt.pix.pixelformat = pixelformats[retry_format];
		
		if (-1 == xioctl(vd.fd, VIDIOC_S_FMT, &fmt))
		{
			int errnum = errno;
			fprintf(stderr, "[V4L2]::%s:%d : xioctl(vd.fd, VIDIOC_S_FMT, &fmt) FAILED !! err=%d=%s\n", 
						__func__, __LINE__, errnum, strerror(errnum));
			retry_format++;
		} else {
			format_ok = true;
			
			char * FourCC = (char *)&fmt.fmt.pix.pixelformat;
			
			fprintf(stderr, "[V4L2]::%s:%d : xioctl(vd.fd, VIDIOC_S_FMT, &fmt) SUPPORTED %c%c%c%c with size : %d x %d !!!!\n", 
					__func__, __LINE__,
					FourCC[0], FourCC[1], FourCC[2], FourCC[3],
					w,h);
			m_nChannels = 4; // use 32bit output by default
			switch(fmt.fmt.pix.pixelformat) {
			default:
				m_capture_palette = -1;
				break;
			case V4L2_PIX_FMT_GREY:
				m_capture_palette = VIDEO_PALETTE_GREY;
				m_nChannels = 1;
				break;
			case V4L2_PIX_FMT_YUYV:
				m_capture_palette = VIDEO_PALETTE_YUYV;
				break;
			case V4L2_PIX_FMT_YVU420:
			case V4L2_PIX_FMT_YUV420:
				m_capture_palette = VIDEO_PALETTE_YUV420P;
				break;
			case V4L2_PIX_FMT_RGB24:
				m_capture_palette = VIDEO_PALETTE_RGB24;
				break;
			case V4L2_PIX_FMT_RGB32:
				m_capture_palette = VIDEO_PALETTE_RGB32;
				break;
			}

			fprintf(stderr, "[V4L2]::%s:%d :  %c%c%c%c => palette V4L=%d!! !!\n", __func__, __LINE__,
				FourCC[0], FourCC[1], FourCC[2], FourCC[3], m_capture_palette);
			
		}
	}
	/* Note VIDIOC_S_FMT may change width and height. */

	/* Buggy driver paranoia. */
	int l_min = fmt.fmt.pix.width * 2;
	if (fmt.fmt.pix.bytesperline < l_min)
		fmt.fmt.pix.bytesperline = l_min;
	l_min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
	if (fmt.fmt.pix.sizeimage < l_min)
		fmt.fmt.pix.sizeimage = l_min;

	vd.window.x = 0;
	vd.window.y = 0;
	vd.window.width = fmt.fmt.pix.width;
	vd.window.height = fmt.fmt.pix.height;
	
	
	return 0;
}

int V4L2Device::getCompressedBuffer(unsigned char ** rawbuffer, 
		unsigned char ** compbuffer, long * compsize) { 
	// No hardware compression 
	*compsize=0;
	*compbuffer = 0;
	
	unsigned char * l_rawbuffer = getFrameBuffer();
	if(!l_rawbuffer) 
	{
		#ifndef PRODUCTION
		fprintf(stderr, "[V4L2]::%s:%d : no acq !\n", __func__, __LINE__);
		#endif
		return -1;
	}
	
	*rawbuffer = l_rawbuffer;
	*compbuffer = 0;
	
	return 0;
}

int V4L2Device::setnorm(char *n) 
{
	for(int i=0; normlists[i].name[0] != '\0'; i++)
	{
		if(!strcmp(n, normlists[i].name)) {
			norm = normlists[i].type;
		    v4l_setdefaultnorm(norm); // moved after getcapabilities to get the channel number
			// then ioctl to set the channel
			setchannel(channel);
			
			return 1;
		}
	}
	return 0;
}


char * V4L2Device::getDeviceName() { 
	return videodevice; 
}


/** @brief Preferred image format */
typedef struct {
	enum v4l2_buf_type type;
	int width;
	int height;
	int pix_fmt;
} t_preferred_fmt;
const int nb_preferred_formats = 6;

#define CAMERA_WIDTH	800
#define CAMERA_HEIGHT	600


const t_preferred_fmt preferred_formats[] = {
	
	{ V4L2_BUF_TYPE_VIDEO_CAPTURE, CAMERA_WIDTH, CAMERA_HEIGHT, V4L2_PIX_FMT_RGB32},
	{ V4L2_BUF_TYPE_VIDEO_CAPTURE, CAMERA_WIDTH, CAMERA_HEIGHT, V4L2_PIX_FMT_BGR32},
	{ V4L2_BUF_TYPE_VIDEO_CAPTURE, CAMERA_WIDTH, CAMERA_HEIGHT, V4L2_PIX_FMT_YUYV},
	{ V4L2_BUF_TYPE_VIDEO_CAPTURE, CAMERA_WIDTH, CAMERA_HEIGHT, V4L2_PIX_FMT_YUV422P},
	{ V4L2_BUF_TYPE_VIDEO_CAPTURE, CAMERA_WIDTH, CAMERA_HEIGHT, V4L2_PIX_FMT_RGB24},

	// For PWC webcams
	{ V4L2_BUF_TYPE_VIDEO_CAPTURE, 640, 480, V4L2_PIX_FMT_YUV420}
};

int V4L2Device::VDopen(char * device, tBoxSize *newSize) 
{
	fprintf(stderr, "[V4L2]::%s:%d : open device '%s' with preferred size %dx%d\n",
			__func__, __LINE__, device, 
			!newSize?-1:(int)newSize->width, !newSize?-1:(int)newSize->height); fflush(stderr);

	int q = 85;
	int maxc;
	
	if(videodevice) 
		delete [] videodevice;
	videodevice = NULL;
	
	if(device == NULL){
    	videodevice = new char [ strlen(DEFAULT_VIDEO_DEVICE) +1];
		strcpy(videodevice, DEFAULT_VIDEO_DEVICE);
	}
	else {
		videodevice = new char [ strlen(device) +1];
		strcpy(videodevice, device);
	}

	// Open device (open_device())
	struct stat st; 
	
	if (-1 == stat (videodevice, &st)) {
		fprintf (stderr, "[V4L2]::%s:%d Cannot identify '%s': %d, %s\n", __func__, __LINE__, 
			 videodevice, errno, strerror (errno));
		exit (EXIT_FAILURE);
	}
	
	if (!S_ISCHR (st.st_mode)) {
		fprintf (stderr, "[V4L2]::%s:%d : ERROR %s is no device\n",  __func__, __LINE__, videodevice);
		exit (EXIT_FAILURE);
	}
	
	vd.fd = open(videodevice, O_RDWR /* required */ | O_NONBLOCK, 0);
	
	if (-1 == vd.fd) {
		fprintf (stderr, "[V4L2]::%s:%d Cannot open '%s': %d, %s\n", __func__, __LINE__, 
			 videodevice, errno, strerror (errno));
		exit (EXIT_FAILURE);
	}
		
	
	struct v4l2_input       v4l2Input ; 
		memset(&v4l2Input, 0, sizeof(struct v4l2_input));
	struct v4l2_fmtdesc     v4l2Fmtdesc ;      // image formats	
		memset(&v4l2Fmtdesc, 0, sizeof(struct v4l2_fmtdesc));
	struct v4l2_standard    v4l2Standard ;     // video standards
		memset(&v4l2Standard, 0, sizeof(struct v4l2_standard));
	struct v4l2_format      v4l2Format;        // format use to set the device
		memset(&v4l2Format, 0, sizeof(struct v4l2_format));
	char * device_node = videodevice;
	struct v4l2_frmsize_stepwise v4l2FrameSize;
		memset(&v4l2FrameSize, 0, sizeof(struct v4l2_frmsize_stepwise));
	
	int v4l2fd = vd.fd;
	struct v4l2_capability  v4l2Capabilities ; // Device capabilities
		memset(&v4l2Capabilities, 0, sizeof(struct v4l2_capability));
	// get capabilities
	if(-1 == ioctl(v4l2fd, VIDIOC_QUERYCAP, &v4l2Capabilities))
	{
		V4L2_printf("Cannot get capabilities for node '%s' => Maybe not V4L2 !\n", device_node) ;
		
		return -1;
		
	} else {
		V4L2_printf("Node '%s' => V4L2 device !\n", device_node) ;
	}
	
	
	// print capabilities	  
	V4L2_printf("Driver  : %s\n",  v4l2Capabilities.driver   );
	V4L2_printf("Card    : %s\n",  v4l2Capabilities.card     );
	V4L2_printf("Version : %u.%u.%u\n", (v4l2Capabilities.version >> 16) & 0xFF,
								   (v4l2Capabilities.version >> 8) & 0xFF,
								  (v4l2Capabilities.version & 0xFF));
	V4L2_printf("Bus     : %s\n",  v4l2Capabilities.bus_info );
	
	V4L2_printf("Capabilities :\n");
	if(v4l2Capabilities.capabilities & V4L2_CAP_VIDEO_CAPTURE)
	{ 
		V4L2_printf(" - V4L2_CAP_VIDEO_CAPTURE\n");        
	}
	if(v4l2Capabilities.capabilities & V4L2_CAP_VIDEO_OUTPUT)
	{ 
		V4L2_printf(" - V4L2_CAP_VIDEO_OUTPUT\n" );         
	}
	if(v4l2Capabilities.capabilities & V4L2_CAP_VIDEO_OVERLAY)
	{ 
		V4L2_printf(" - V4L2_CAP_VIDEO_OVERLAY\n" );        
	}
	if(v4l2Capabilities.capabilities & V4L2_CAP_VBI_CAPTURE)
	{ 
		V4L2_printf(" - V4L2_CAP_VBI_CAPTURE\n" );      
	}
	if(v4l2Capabilities.capabilities & V4L2_CAP_VBI_OUTPUT)
	{ 
		V4L2_printf(" - V4L2_CAP_VBI_OUTPUT\n" );       
	}
	if(v4l2Capabilities.capabilities & V4L2_CAP_SLICED_VBI_CAPTURE)
	{ 
		V4L2_printf(" - V4L2_CAP_SLICED_VBI_CAPTURE\n" );   
	}
	if(v4l2Capabilities.capabilities & V4L2_CAP_SLICED_VBI_OUTPUT)
	{ 
		V4L2_printf(" - V4L2_CAP_SLICED_VBI_OUTPUT\n" );    
	}
	if(v4l2Capabilities.capabilities & V4L2_CAP_RDS_CAPTURE)
	{ 
		V4L2_printf(" - V4L2_CAP_RDS_CAPTURE\n" );            
	}
	#ifdef V4L2_CAP_VIDEO_OUTPUT_OVERLAY
	if(v4l2Capabilities.capabilities & V4L2_CAP_VIDEO_OUTPUT_OVERLAY)
	{ 
		V4L2_printf(" - V4L2_CAP_VIDEO_OUTPUT_OVERLAY\n" ); 
	}
	#else // V4L2_CAP_VIDEO_OVERLAY
	if((v4l2Capabilities.capabilities & V4L2_CAP_VIDEO_OVERLAY))
	{
	        V4L2_printf(" - V4L2_CAP_VIDEO_OVERLAY\n" );
	}
	#endif
	if(v4l2Capabilities.capabilities & V4L2_CAP_TUNER)
	{ 
		V4L2_printf(" - V4L2_CAP_TUNER\n" );                 
	}
	if(v4l2Capabilities.capabilities & V4L2_CAP_AUDIO)
	{ 
		V4L2_printf(" - V4L2_CAP_AUDIO\n" );           
	}
	if(v4l2Capabilities.capabilities & V4L2_CAP_RADIO)
	{ 
		V4L2_printf(" - V4L2_CAP_RADIO\n" );           
	}
	if(v4l2Capabilities.capabilities & V4L2_CAP_READWRITE)
	{ 
		V4L2_printf(" - V4L2_CAP_READWRITE\n" );      
	}
	if(v4l2Capabilities.capabilities & V4L2_CAP_ASYNCIO)
	{ 
		V4L2_printf(" - V4L2_CAP_ASYNCIO\n" );                   
	}
	if(v4l2Capabilities.capabilities & V4L2_CAP_STREAMING)
	{ 
		V4L2_printf(" - V4L2_CAP_STREAMING\n" );        
		if(v4l2Capabilities.capabilities & V4L2_CAP_TIMEPERFRAME)
		{ 
			V4L2_printf(" - V4L2_CAP_TIMEPERFRAME\n" );        
		}
	}
	
	
	// 	  Enumerate video inputs
	V4L2_printf("Inputs :\n");
	v4l2Input.index = 0 ;
	while (-1 != ioctl(v4l2fd, VIDIOC_ENUMINPUT, &v4l2Input))
	{
		// 	  Print video inputs
		V4L2_printf(" - Input index    %d\n", v4l2Input.index) ;   // Identifies the input, set by the application.
		V4L2_printf(" - Input name     %s\n", v4l2Input.name) ;    // Name of the video input, a NUL-terminated ASCII string, for example: "Vin (Composite 2)". This information is intended for the user, preferably the connector label on the device itself.
		V4L2_printf(" - Input type     %d\n", v4l2Input.type) ;    // Type of the input, see Table 2.
		V4L2_printf(" - Input audioset %d\n", v4l2Input.audioset); // 		
		V4L2_printf(" - Input tuner    %d\n", v4l2Input.tuner) ;   // Capture devices can have zero or more tuners (RF demodulators). When the type is set to V4L2_INPUT_TYPE_TUNER this is an RF connector and this field identifies the tuner. It corresponds to struct v4l2_tuner field index. For details on tuners see Section 1.6.
		V4L2_printf(" - Input std      %d\n", (int)v4l2Input.std) ;     // Every video input supports one or more different video standards. This field is a set of all supported standards. For details on video standards and how to switch see Section 1.7.
		V4L2_printf(" - Input status   %d\n", v4l2Input.status) ;  // This field provides status information about the input. See Table 3 for flags. status is only valid when this is the current input.
		v4l2Input.index ++ ;
	}
	
	
	// select video input : S-Video = index 3
	int VideoInputIndex = 0 ;
	if(-1 == ioctl(v4l2fd, VIDIOC_S_INPUT, &VideoInputIndex))
	{
		V4L2_printf("Cannot set video input %d to '%s' : retry with 0\n",
			VideoInputIndex, device_node );
		VideoInputIndex = 0;
		if(-1 == ioctl(v4l2fd, VIDIOC_S_INPUT, &VideoInputIndex))
		{
			V4L2_printf("Cannot set video input %d to %s : retry FAILED. Bye\n", 
				VideoInputIndex, device_node) ;
		}
	}
	
	
	
	// 	  Enumerate image formats
	V4L2_printf("Image formats :\n");
	v4l2Fmtdesc.index = 0 ;
	v4l2Fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE ;
	while (-1 != ioctl(v4l2fd, VIDIOC_ENUM_FMT, &v4l2Fmtdesc))
	{
		// 	  Print image formats
		V4L2_printf(" - Input index       %d\n", v4l2Fmtdesc.index) ;   // Identifies the image format, set by the application.
		V4L2_printf(" - Input type        %d\n", v4l2Fmtdesc.type) ;    // Type of the data stream, set by the application.
		V4L2_printf(" - Input flags       %d (1 = compressed)\n", v4l2Fmtdesc.flags); // 		
		V4L2_printf(" - Input description %s\n", v4l2Fmtdesc.description) ;   // Capture devices can have zero or more tuners (RF demodulators). When the type is set to V4L2_INPUT_TYPE_TUNER this is an RF connector and this field identifies the tuner. It corresponds to struct v4l2_tuner field index. For details on tuners see Section 1.6.
		V4L2_printf(" - Input pixelformat %c%c%c%c\n\n", (v4l2Fmtdesc.pixelformat)&0xFF,  (v4l2Fmtdesc.pixelformat>>8)&0xFF,  (v4l2Fmtdesc.pixelformat>>16)&0xFF,  (v4l2Fmtdesc.pixelformat>>24)&0xFF) ;     // Every video input supports one or more different video standards. This field is a set of all supported standards. For details on video standards and how to switch see Section 1.7.
		v4l2Fmtdesc.index ++ ;
	}
	
	
	// 	  Enumerate video standards
	V4L2_printf("Video standards :\n");
	v4l2Standard.index = 0 ;
	while (-1 != ioctl(v4l2fd, VIDIOC_ENUMSTD, &v4l2Standard))
	{
		V4L2_printf(" - Input index           %d\n",  v4l2Standard.index) ;   // Identifies the image format, set by the application.
		V4L2_printf(" - Input id              %d\n",  (int)v4l2Standard.id) ;    // Type of the data stream, set by the application.
		V4L2_printf(" - Input name            %s\n",  v4l2Standard.name); // 		
		V4L2_printf(" - Input frameperiod num %d\n",  v4l2Standard.frameperiod.numerator) ;   // Capture devices can have zero or more tuners (RF demodulators). When the type is set to V4L2_INPUT_TYPE_TUNER this is an RF connector and this field identifies the tuner. It corresponds to struct v4l2_tuner field index. For details on tuners see Section 1.6.
		V4L2_printf(" - Input frameperiod den %d\n",  v4l2Standard.frameperiod.denominator) ;   // Capture devices can have zero or more tuners (RF demodulators). When the type is set to V4L2_INPUT_TYPE_TUNER this is an RF connector and this field identifies the tuner. It corresponds to struct v4l2_tuner field index. For details on tuners see Section 1.6.
		V4L2_printf(" - Input framelines      %d\n\n", v4l2Standard.framelines) ;     // Every video input supports one or more different video standards. This field is a set of all supported standards. For details on video standards and how to switch see Section 1.7.
		v4l2Standard.index ++ ;
	}
	
	vd.frame_rate = 0;
	vd.frame_period_ms = 0;
	
	max_width = max_height = min_width = min_height = 0;
	
	if(1) {
		
		int fmt, retry = 1;
		
		for(fmt = 0; fmt < nb_preferred_formats && retry ; fmt++) {
			// 	  tries video format
			V4L2_printf("Sets video format : preferred # %d\n", fmt);
			memset(&v4l2Format, 0, sizeof(v4l2Format));
			
			v4l2Format.type                =  preferred_formats[fmt].type;
			v4l2Format.fmt.pix.width       =  preferred_formats[fmt].width;
			v4l2Format.fmt.pix.height      =  preferred_formats[fmt].height;
			v4l2Format.fmt.pix.pixelformat =  preferred_formats[fmt].pix_fmt;
			
			struct v4l2_frmsizeenum fsize;
			
			memset(&fsize, 0, sizeof(fsize));
			fsize.index = 0;
			u32 pixfmt = fsize.pixel_format = preferred_formats[fmt].pix_fmt;
			int ret;
			
			while ((ret = xioctl(v4l2fd, VIDIOC_ENUM_FRAMESIZES, &fsize)) == 0) {
				if (fsize.type == V4L2_FRMSIZE_TYPE_DISCRETE) {
					V4L2_printf("{ discrete: width = %u, height = %u }\n",
							fsize.discrete.width, fsize.discrete.height);
					ret = enum_frame_intervals(v4l2fd, pixfmt,
							fsize.discrete.width, fsize.discrete.height);
					if (ret != 0)
						V4L2_printf("  Unable to enumerate frame sizes.\n");
					 
					v4l2Format.fmt.pix.width = fsize.discrete.width;
					v4l2Format.fmt.pix.height = fsize.discrete.height;

				} else if (fsize.type == V4L2_FRMSIZE_TYPE_CONTINUOUS) {
					
					V4L2_printf("{ continuous: min { width = %u, height = %u } .. "
							"max { width = %u, height = %u } }\n",
							fsize.stepwise.min_width, fsize.stepwise.min_height,
							fsize.stepwise.max_width, fsize.stepwise.max_height);
					v4l2Format.fmt.pix.width = fsize.stepwise.max_width;
					v4l2Format.fmt.pix.height = fsize.stepwise.max_height;
					
					printf("  Refusing to enumerate frame intervals.\n");
					break;
				} else if (fsize.type == V4L2_FRMSIZE_TYPE_STEPWISE) {
					V4L2_printf("{ stepwise: min { width = %u, height = %u } .. "
							"max { width = %u, height = %u } / "
							"stepsize { width = %u, height = %u } }\n",
							fsize.stepwise.min_width, fsize.stepwise.min_height,
							fsize.stepwise.max_width, fsize.stepwise.max_height,
							fsize.stepwise.step_width, fsize.stepwise.step_height);
					v4l2Format.fmt.pix.width = fsize.stepwise.max_width;
					v4l2Format.fmt.pix.height = fsize.stepwise.max_height;
					V4L2_printf("  Refusing to enumerate frame intervals.\n");
					break;
				}
				fsize.index++;
			}
			
			
			V4L2_printf("Trying to use %d x %d / FourCC=%c%c%c%c...\n",
						v4l2Format.fmt.pix.width , v4l2Format.fmt.pix.height, 
						(v4l2Format.fmt.pix.pixelformat)&0xFF,  
						(v4l2Format.fmt.pix.pixelformat>>8)&0xFF,  
						(v4l2Format.fmt.pix.pixelformat>>16)&0xFF,  
						(v4l2Format.fmt.pix.pixelformat>>24)&0xFF);
			
			
			if(-1 == xioctl(v4l2fd, VIDIOC_S_FMT, &v4l2Format))
			{
				int errnum = errno;
				
				V4L2_printf("Cannot set %s image format = "
					"{type=%d wxh=%dx%d FourCC=%c%c%c%c} error=%d='%s'\n", 
					device_node,
					v4l2Format.type,
					v4l2Format.fmt.pix.width, v4l2Format.fmt.pix.height,
					(v4l2Format.fmt.pix.pixelformat)&0xFF,  
						(v4l2Format.fmt.pix.pixelformat>>8)&0xFF,  
						(v4l2Format.fmt.pix.pixelformat>>16)&0xFF,  
						(v4l2Format.fmt.pix.pixelformat>>24)&0xFF,
					errnum, strerror(errnum)
					) ;
				
				if(errnum == 5) {
					// Wait 
					usleep(200000);
					retry++;
					if(retry < 5) {
					   fmt--;
					} else {
						retry = 0;
					}
				}
			}
			else
			{
				retry = 0;
				
				newSize->width =	video_width = vd.width = v4l2Format.fmt.pix.width;
				newSize->height = video_height = vd.height = v4l2Format.fmt.pix.height;
				vd.v4l2_pix_fmt = preferred_formats[fmt].pix_fmt;
				
				V4L2_printf(" VIDIOC_S_FMT success : \n");
				V4L2_printf(" - video format type   %d\n", v4l2Format.type) ;    // video format type
				V4L2_printf(" - video format width  %d\n", v4l2Format.fmt.pix.width) ;    // video format width
				V4L2_printf(" - video format height %d\n", v4l2Format.fmt.pix.height) ;    // video format height
				V4L2_printf(" - video pixel format  %c%c%c%c\n", (v4l2Format.fmt.pix.pixelformat)&0xFF,  
														 (v4l2Format.fmt.pix.pixelformat>>8)&0xFF,  
														 (v4l2Format.fmt.pix.pixelformat>>16)&0xFF,  
														 (v4l2Format.fmt.pix.pixelformat>>24)&0xFF) ;    // video format type
				V4L2_printf(" - video format field        %d (1 = progressive)\n", v4l2Format.fmt.pix.field) ;    // video format field
				V4L2_printf(" - video format bytesperline %d\n", v4l2Format.fmt.pix.bytesperline) ;    // video format bytesperline
				V4L2_printf(" - video format sizeimage    %d\n", v4l2Format.fmt.pix.sizeimage) ;    // video format sizeimage
				V4L2_printf(" - video format colorspace   %d\n", v4l2Format.fmt.pix.colorspace) ;    // video format 
				V4L2_printf(" - video format private flag %d\n", v4l2Format.fmt.pix.priv) ;    // video format 
			}
		}
	}
	
	
	
	
	// set acquisition size
	changeSize(newSize);

	initialised = true;
//    echo();

	return 0;
}

int V4L2Device::VDclose()
{
	if(!initialised) {
		fprintf(stderr, "\t\t[V4L2]::%s:%d : already closed.\n", __func__, __LINE__);
		return -1;
	}

	fprintf(stderr, "\t\t[V4L2] : Closing device...\n");
	initialised = false;
	
	// Stop_capturing
	stop_capturing();
	
	// uninit_device() and free mmap
	for(unsigned int i = 0; i < n_buffers; ++i)
		if (-1 == munmap (buffers[i].start, buffers[i].length))
			errno_exit ("munmap");
	
	free(buffers);
	
	return 1;
}

int V4L2Device::stop_capturing() {
	unsigned int i;
	enum v4l2_buf_type type;

	for (i = 0; i < n_buffers; ++i) 
	{
		struct v4l2_buffer buf;
		
		CLEAR (buf);
		
		buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory      = V4L2_MEMORY_MMAP;
		buf.index       = i;
		
		if (-1 == xioctl(vd.fd, VIDIOC_QBUF, &buf))
			errno_exit ("VIDIOC_QBUF");
	}
                
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if (-1 == xioctl(vd.fd, VIDIOC_STREAMON, &type)) {
		//errno_exit ("VIDIOC_STREAMON");
		fprintf(stderr, "V4L2Device::%s:%d : error in VIDIOC_STREAMON\n", __func__, __LINE__);
	}
	return 0;
}
// starts video acquisition (public function)
int V4L2Device::VDInitAll()
{
	
	return 0;
}

// display video parameters
void V4L2Device::echo()
{
	int i=0;

	if(vd.fd==-1)
		printf("\t\tNo video device opened !\n");
	else if(!initialised)
		printf("\t\tVideo device not initialised ! \n");
	else {
		printf("Video device name : %s\n", vd.capability.name);
		printf("Main capabilities : \n");
		printf("\tchannels: %d\n", vd.capability.channels);
		printf("\tmax size: %dx%d\n",vd.capability.maxwidth, vd.capability.maxheight);
		printf("\tmin size: %dx%d\n",vd.capability.minwidth, vd.capability.minheight);
		printf("\tDevice type : \n");
		if(vd.capability.type & VID_TYPE_CAPTURE) printf("\t\tVID_TYPE_CAPTURE,");
		if(vd.capability.type & VID_TYPE_OVERLAY) printf("\t\tVID_TYPE_OVERLAY,");
		if(vd.capability.type & VID_TYPE_CLIPPING) printf("\t\tVID_TYPE_CLIPPING,");
		if(vd.capability.type & VID_TYPE_FRAMERAM) printf("\t\tVID_TYPE_FRAMERAM,");
		if(vd.capability.type & VID_TYPE_SCALES) printf("\t\tVID_TYPE_SCALES,");
		if(vd.capability.type & VID_TYPE_MONOCHROME) printf("\t\tVID_TYPE_MONOCHROME,");
		if(vd.capability.type & VID_TYPE_SUBCAPTURE) printf("\t\tVID_TYPE_SUBCAPTURE,");
		printf("\nCurrent status;\n");
		printf("\tpicture.depth: %d\n",vd.picture.depth);
		printf("\tmbuf.size: %08x\n",vd.mbuf.size);
		printf("\tmbuf.frames: %d\n",vd.mbuf.frames);
		printf("\tmbuf.offsets[0]: %08x\n",vd.mbuf.offsets[0]);
		printf("\tmbuf.offsets[1]: %08x\n",vd.mbuf.offsets[1]);
		printf("\nVideo Window : \n");
		printf("\tx = %d y = %d \n", vd.window.x, vd.window.y);
		printf("\twidth = %d height = %d \n", vd.window.width, vd.window.height);
        printf("Misc : \n");
		printf("Palette : ");
		for(i=0;i<NB_PALETTES;i++)
		{
			if(palettes_defined[i].value==vd.picture.palette)
			{
				printf("%s\n", palettes_defined[i].designation);
				break;
			}
		}
		if(hasFramerate)
			printf("Video device HAS framerate\n");
		else
			printf("Video device DOES NOT support framerate\n");
		if(useSelectAvailable)
			printf("Select() AVAILABLE\n");
		else
			printf("Select() NOT AVAILABLE\n");
	}
}

// public function : returns already read capability
int V4L2Device::getcapability(video_capability * dest)
{
	if(!initialised)
		return -1;

	// Transfer from V4L2 to V4L capability (v4l2_capability => video_capability)
	/*
	struct v4l2_capability
	{
			__u8    driver[16];     // i.e. "bttv" 
			__u8    card[32];       // i.e. "Hauppauge WinTV"
			__u8    bus_info[32];   // "PCI:" + pci_name(pci_dev) 
			__u32   version;        // should use KERNEL_VERSION() 
			__u32   capabilities;   // Device capabilities 
			__u32   reserved[4];
	};

=>
	struct video_capability
	{
		char name[32];
		int type;
		int channels;	// * Num channels * /
		int audios;	// * Num audio devices * /
		int maxwidth;	// * Supported width * /
		int maxheight;	// * And height * /
		int minwidth;	// * Supported width * /
		int minheight;	// * And height * /
	};
	*/
	memcpy(dest, &(vd.capability), sizeof(video_capability));
	
	int v4l2fd = vd.fd;
	struct v4l2_capability  v4l2Capabilities ; // Device capabilities
		memset(&v4l2Capabilities, 0, sizeof(struct v4l2_capability));
	// get capabilities
	if(-1 == ioctl(v4l2fd, VIDIOC_QUERYCAP, &v4l2Capabilities))
	{
		V4L2_printf("Cannot get capabilities for node '%s' => Maybe not V4L2 !\n", getDeviceName ()) ;
		
		return -1;
	} 
	
	
	// print capabilities	  
	V4L2_printf("Driver  : %s\n",  v4l2Capabilities.driver   );
	
	V4L2_printf("Card    : %s\n",  v4l2Capabilities.card     );
	strcpy((char *)dest->name, (char *)v4l2Capabilities.card);
	V4L2_printf("Version : %u.%u.%u\n", (v4l2Capabilities.version >> 16) & 0xFF,
								   (v4l2Capabilities.version >> 8) & 0xFF,
								  (v4l2Capabilities.version & 0xFF));
	V4L2_printf("Bus     : %s\n",  v4l2Capabilities.bus_info );
	
	V4L2_printf("Capabilities :\n");
	if(v4l2Capabilities.capabilities & V4L2_CAP_VIDEO_CAPTURE)
	{ 
		V4L2_printf(" - V4L2_CAP_VIDEO_CAPTURE\n"); 
		dest->type |= VID_TYPE_CAPTURE;
	
	}
	if(v4l2Capabilities.capabilities & V4L2_CAP_VIDEO_OUTPUT)
	{ 
		V4L2_printf(" - V4L2_CAP_VIDEO_OUTPUT\n" );         
	}
	if(v4l2Capabilities.capabilities & V4L2_CAP_VIDEO_OVERLAY)
	{ 
		V4L2_printf(" - V4L2_CAP_VIDEO_OVERLAY\n" );        
		dest->type |= VID_TYPE_OVERLAY;
	}
	if(v4l2Capabilities.capabilities & V4L2_CAP_VBI_CAPTURE)
	{ 
		V4L2_printf(" - V4L2_CAP_VBI_CAPTURE\n" );      
	}
	if(v4l2Capabilities.capabilities & V4L2_CAP_VBI_OUTPUT)
	{ 
		V4L2_printf(" - V4L2_CAP_VBI_OUTPUT\n" );       
	}
	if(v4l2Capabilities.capabilities & V4L2_CAP_SLICED_VBI_CAPTURE)
	{ 
		V4L2_printf(" - V4L2_CAP_SLICED_VBI_CAPTURE\n" );   
	}
	if(v4l2Capabilities.capabilities & V4L2_CAP_SLICED_VBI_OUTPUT)
	{ 
		V4L2_printf(" - V4L2_CAP_SLICED_VBI_OUTPUT\n" );    
	}
	if(v4l2Capabilities.capabilities & V4L2_CAP_RDS_CAPTURE)
	{ 
		V4L2_printf(" - V4L2_CAP_RDS_CAPTURE\n" );            
	}
	if(v4l2Capabilities.capabilities & V4L2_CAP_TUNER)
	{ 
		V4L2_printf(" - V4L2_CAP_TUNER\n" );                 
	}
	if(v4l2Capabilities.capabilities & V4L2_CAP_AUDIO)
	{ 
		V4L2_printf(" - V4L2_CAP_AUDIO\n" );           
	}
	if(v4l2Capabilities.capabilities & V4L2_CAP_RADIO)
	{ 
		V4L2_printf(" - V4L2_CAP_RADIO\n" );           
	}
	if(v4l2Capabilities.capabilities & V4L2_CAP_READWRITE)
	{ 
		V4L2_printf(" - V4L2_CAP_READWRITE\n" );      
	}
	if(v4l2Capabilities.capabilities & V4L2_CAP_ASYNCIO)
	{ 
		V4L2_printf(" - V4L2_CAP_ASYNCIO\n" );                   
	}
	if(v4l2Capabilities.capabilities & V4L2_CAP_STREAMING)
	{ 
		V4L2_printf(" - V4L2_CAP_STREAMING\n" );        
		if(v4l2Capabilities.capabilities & V4L2_CAP_TIMEPERFRAME)
		{ 
			V4L2_printf(" - V4L2_CAP_TIMEPERFRAME\n" );        
		}
	}
	
	return 0;
}

int V4L2Device::getpicture(video_picture * dest)
{
	if(!initialised)
		return -1;

	memcpy(dest, &(vd.picture), sizeof(video_picture));
	return 0;
}

int V4L2Device::getmbuf(video_mbuf * dest)
{
	if(!initialised)
		return -1;

	memcpy(dest, &(vd.mbuf), sizeof(video_mbuf));
	return 0;
}

int V4L2Device::getFD()
{
	return vd.fd;
}

int V4L2Device::maxchannel()
{
	if(!initialised)
		return -1;

	return vd.capability.channels;
}

int V4L2Device::getchannel(video_channel * dest, int noChannel)
{
	if(!initialised)
		return -1;

	if(noChannel>(vd.capability.channels-1))
		return -1;

	memcpy(dest, &(vd.channel[noChannel]), sizeof(video_channel));
	return 0;
}

int V4L2Device::getpalette()
{
	if(!initialised)
		return -1;
	
	if(g_debug_V4L2Device) {	
		for(int i=0;i<NB_PALETTES;i++)
		{
			if(palettes_defined[i].value == m_capture_palette)
			{
				fprintf(stderr, "[V4L2]::getpalette : current palette : %d='%s'\n", 
					m_capture_palette, palettes_defined[i].designation);
				return m_capture_palette;
			}
		}

		fprintf(stderr, "[V4L2] : not defined for palette=%d\n", m_capture_palette);
	}
	return m_capture_palette;
}

int V4L2Device::getcapturewindow (video_window * dest)
{
	if(!initialised)
		return -1;

	memcpy(dest, &(vd.window), sizeof(video_window));
	return 0;
}

int V4L2Device::getcapturewindowsize (tBoxSize * dest)
{
	fprintf(stderr, "[V4L2]::%s:%d\n", __func__, __LINE__);

	if(!initialised)
		return -1;

	dest->x = vd.window.x;
	dest->y = vd.window.y;
	
	dest->width = vd.window.width;
	dest->height = vd.window.height;
	fprintf(stderr, "[V4L2]::%s:%d : crop : %lu,%lu+%lux%lu\n", __func__, __LINE__,
		dest->x,dest->y,dest->width,dest->height);
	
	return 0;
}

/* UNUSED XXX
// only after a SetSubCapture
// this value isn't read at init()
int V4L2Device::getsubcapture(video_capture * dest)
{
	if(vd.fd==-1)
		return -1;
	
	v4l_getsubcapture(dest);
	return 0;
}
*/

pthread_mutex_t * V4L2Device::getMutex()
{
	return &(vd.mutex);
}

bool V4L2Device::VDHasFramerate()
{
	return hasFramerate;
}
bool V4L2Device::VDSelectAvailable()
{
	return useSelectAvailable;
}

bool V4L2Device::VDIsInitialised()
{
	return initialised;
}

bool V4L2Device::VDHasMmap()
{
	fprintf(stderr, "[V4L2]::%s:%d \n", __func__, __LINE__);
	if(!buffers) return false;
	
	return (buffers[0].length>0);
}

int V4L2Device::setframebuffer(void *base, int width, int height, int depth, int bpl)
{
	if(vd.fd==-1)
		return -1;
	return v4l_setframebuffer(base, width, height, depth, bpl);
}

int V4L2Device::setpicture(int br, int hue, int col, int cont, int white)
{
	if(vd.fd ==-1)
		return -1;

	video_picture tmpPicture;
	if(br>=0)
		tmpPicture.brightness = br;
	if(hue>=0)
		tmpPicture.hue = hue;
	if(col>=0)
		tmpPicture.colour = col;
	if(cont>=0)
		tmpPicture.contrast = cont;
	if(white>=0)
		tmpPicture.whiteness = white;
	v4l_setpicture(br, hue, col, cont, white);

	return 0;
}



int V4L2Device::setsubcapture ( int x, int y, int width, int height, int decimation, int flags)
{
	if(vd.fd==-1)
		return -1;

	video_capture tmpCapture;
	tmpCapture.x = x;
	tmpCapture.y = y;
	tmpCapture.width = width;
	tmpCapture.height = height;
	tmpCapture.decimation = decimation;
	tmpCapture.flags = flags;

	if(ioctl(vd.fd, VIDIOCSCAPTURE, &tmpCapture) < 0)
		return -1;

	// MAJ de capture inutile : valeur relue à chaque appel de getsubcapture
	return 0;
}

/** unused
int V4L2Device::setdefaultnorm	(int defaultNorme)
{
	if(vd.fd==-1)
		return -1;

	int i;

	for(i=0;i<vd.capability.channels;i++) {
		vd.channel[i].norm = defaultNorme;
	}

	return 0;
}
*/
int V4L2Device::getcurrentchannel()
{
	if(vd.fd==-1) {
		perror("\t\t[V4L2]::getcurrentchannel: vd.fd==-1");
		return -1;
	}

	return channel;
}

char * V4L2Device::getnorm()
{
	if(ioctl(vd.fd, VIDIOCGCHAN, &(vd.channel[channel])) < 0)
	{
		fprintf(stderr, "!\t!\t[V4L2]::getnorm:VIDIOCGCHAN (channel = %d)", channel);
		return NULL;
	}
	for(int i=0; normlists[i].name[0] != '\0'; i++) {
		if(normlists[i].type == vd.channel[channel].norm) {
			return (char *)normlists[i].name;
		}
	}
	return NULL;
}

int V4L2Device::setchannel(int ch)
{
	if(vd.fd==-1)
		return -1;

	if(ioctl(vd.fd, VIDIOCSCHAN, &(vd.channel[ch])) < 0)
	{
		perror("!\t!\t[V4L2]::setchannel:VIDIOCSCHAN");
		return -1;
	}
	channel = ch;
	return 0;
}

int V4L2Device::setfrequence(int freq)
{
	if(vd.fd==-1)
		return -1;

	unsigned long longfreq=(freq*16)/1000;
	if(ioctl(vd.fd, VIDIOCSFREQ, &longfreq) < 0)
		return -1;

	return 0;
}

int V4L2Device::setpalette(int pal)
{
	if(vd.fd==-1)
		return -1;

	
	if(ioctl(vd.fd, VIDIOCSPICT, &(vd.picture)) < 0)
		return -1;

	m_capture_palette = pal;
	vd.picture.palette = pal;
	return 0;
}


unsigned char * V4L2Device::read_frame(void)
{
	struct v4l2_buffer buf;
	
	CLEAR (buf);

	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;
	
	if (-1 == xioctl(vd.fd, VIDIOC_DQBUF, &buf)) {
		switch (errno) {
		case EAGAIN:
			return NULL;
		
		case EIO:
			/* Could ignore EIO, see spec. */
		
			/* fall through */
		
		default:
			errno_exit ("VIDIOC_DQBUF");
		}
		
		return NULL;
	}
	
	assert(buf.index < n_buffers);
	
	//process_image (buffers[buf.index].start);
	unsigned char * retbuf = (unsigned char *)buffers[buf.index].start;

	if (-1 == xioctl(vd.fd, VIDIOC_QBUF, &buf)) {
		errno_exit ("VIDIOC_QBUF");
	}
	
	return retbuf;
}

unsigned char * V4L2Device::getFrameBuffer()
{
	//fprintf(stderr,"START --> V4L2Device::getFrameBuffer()!!\n");
	
	fd_set fds;
	struct timeval tv;
	int r;
	
	FD_ZERO (&fds);
	FD_SET (vd.fd, &fds);
	
	/* Timeout. */
	tv.tv_sec = 2;
	tv.tv_usec = 0;
	
	r = select(vd.fd + 1, &fds, NULL, NULL, &tv);
	
	if(-1 == r) {
		if (EINTR == errno)
			return NULL;
		
		errno_exit ("select");
	}
	
	if(0 == r) {
		fprintf (stderr, "select timeout\n");
		//exit (EXIT_FAILURE);
		return NULL;
	}
	
	return read_frame();
}

/** v4l_getcapability - get the capability into vd.capability
 */
int V4L2Device::v4l_getcapability()
{
	if(g_debug_V4L2Device) fprintf(stderr, "[V4L2]::v4l_getcapability:VIDIOCGCAP...\n");
	
	
	if(ioctl(vd.fd, VIDIOCGCAP, &(vd.capability)) < 0) {
		perror("!\t!\t[V4L2]::v4l_getcapability:VIDIOCGCAP");
		return -1;
	}
	

	if(g_debug_V4L2Device) 
		fprintf(stderr, "\t\t[V4L2]:v4l_getcapability : done\n");
	return 0;
}

/** v4l_setdefaultnorm - set default norm and reset parameters
 */
int V4L2Device::v4l_setdefaultnorm(int n)
{
	int i;

	for(i=0;i<vd.capability.channels;i++) {
		v4l_setchannelnorm(i, n);
	}
	if(v4l_getcapability())
		return -1;
	if(v4l_getpicture())
		return -1;
	norm = n;
	return 0;
}

/** v4l_setchannelnorm - set the norm of channel
 @param vd device handling struct
 @param channel channel number
 @param n input norm PAL/SECAM/NTSC/OTHER (source: videodev.h)
 */
int V4L2Device::v4l_setchannelnorm(int ch, int n)
{
	vd.channel[ch].norm = n;
	return 0;
}



/** v4l_getpicture - get current properties of the picture
 */
int V4L2Device::v4l_getpicture()
{
	if(ioctl(vd.fd, VIDIOCGPICT, &(vd.picture)) < 0) {
		perror("!\t!\t[V4L2]::v4l_getpicture:VIDIOCGPICT");
		return -1;
	}
	return 0;
}

/// returns video buffer address
unsigned char * V4L2Device::video_getaddress()
{
	return (vd.map + vd.mbuf.offsets[vd.frame]);
}

/// v4lgetmbuf - get the size of the buffer to mmap
int V4L2Device::v4l_getmbuf()
{
	if(ioctl(vd.fd, VIDIOCGMBUF, &(vd.mbuf))<0) {
		perror("!\t!\t[V4L2]::v4l_getmbuf:VIDIOCGMBUF");
		return -1;
	}
	return 0;
}


/** v4lmmap - initialize mmap interface
 */
int V4L2Device::v4l_mmap()
{
	if(v4l_getmbuf()<0)
		return -1;
	if((vd.map = (unsigned char *)mmap(0, vd.mbuf.size, PROT_READ|PROT_WRITE, MAP_SHARED, vd.fd, 0)) < 0) {
		perror("!\t!\t[V4L2]::v4l_mmap:mmap");
		return -1;
	}
	return 0;
}

/** v4l_munmap - free memory area for mmap interface
 */
int V4L2Device::v4l_munmap()
{
	if(munmap(vd.map, vd.mbuf.size) < 0) {
		perror("!\t!\t[V4L2]::v4l_munmap:munmap");
		return -1;
	}
	return 0;
}

/** v4l_grabinit - set parameters for mmap interface
 @param vd device =handling struct
 @param width width of the buffer
 @param height height of the buffer
 */
int V4L2Device::v4l_grabinit(int width, int height)
{
	vd.mmap.width = width;
	vd.mmap.height = height;
	vd.mmap.format = vd.picture.palette;
	vd.frame = 0;
	vd.framestat[0] = 0;
	vd.framestat[1] = 0;
	return 0;
}

/** v4l_grabstart - activate mmap capturing
 @param vd: v4l device object
 @param frame: frame number for storing captured image
 */
int V4L2Device::v4l_grabstart(int frame)
{
	if(vd.framestat[frame]) {
		fprintf(stderr, "[V4L2]::v4l_grabstart: frame %d is already used to grab.\n\t>", frame);
		
		frame = frame ^ 1; // XXX TEST : marche a priori (07/05/2003)
		//return -1;
	}
	vd.mmap.frame = frame;
	if(ioctl(vd.fd, VIDIOCMCAPTURE, &(vd.mmap)) < 0) {
		perror("!\t!\t[V4L2]::v4l_grabstart:VIDIOCMCAPTURE");
		return -1;
	}
	vd.framestat[frame] = 1;
	return 0;
}

/** v4l_sync - wait until mmap capturing of the frame is finished
 */
int V4L2Device::v4l_sync(int frame)
{
	if(g_debug_V4L2Device) 
		fprintf(stderr, "\t\t[V4L2]::v4l_sync: sync frame %d.\n",frame);
	if(vd.framestat[frame] == 0) {
		fprintf(stderr, "!\t!\t[V4L2]::v4l_sync: grabbing to frame %d is not started.\n", frame);
	}
	if(ioctl(vd.fd, VIDIOCSYNC, &frame) < 0) {
		perror("!\t!\t[V4L2]::v4l_sync:VIDIOCSYNC");
		return -1;
	}
	vd.framestat[frame] = 0;
	return 0;
}

/* check supported pixel format */
int V4L2Device::GrabCheck(int pal)
{
	int ret;

	fprintf(stderr, "\t\t[V4L2]::GrabCheck(%d) : v4l_setpalette(%d)\n", pal, pal);
 	// problem with Philips webcam when we suppress converter
	if(v4l_setpalette(pal)) {
		ret = -1;
		fprintf(stderr, "!\t!\t[V4L2]::GrabCheck : v4l_setpalette error\n");
		return ret;
	}
	
	fprintf(stderr, "\t\t[V4L2]::GrabCheck(%d) : v4l_grabstart(0)\n", pal);
	if(v4l_grabstart(0) <0) {
		ret = -1;
		return ret;
	}
	fprintf(stderr, "\t\t[V4L2]::GrabCheck(%d) : v4l_sync(0)\n", pal);
	ret = v4l_sync(0);
	
	fprintf(stderr, "\t\t[V4L2]::GrabCheck(%d) : ok with pal = %d\n", pal, pal);
	// read vd
	fprintf(stderr, "\t\t[V4L2]::v4l_getpicture ...\n");
	v4l_getpicture();
    fprintf(stderr, "\t\t[V4L2]::GrabCheck(%d) : v4l_getpicture returned pal = %d\n", 
		pal, vd.picture.palette);
	
	m_capture_palette = vd.picture.palette;
	return 0;
}

int V4L2Device::setGrabFormat(int pal)
{
	if(pal == 0) {
		if(GrabCheck(DEFAULT_PALETTE) == 0) {
			return 0;
		}
	} else {
	}

    fprintf(stderr, "\t\t[V4L2]::setGrabFormat : default palette (%d) not supported. "
		"Looking for supported palette...\n", 
		pal);
    
	v4l_getpicture();
	
	fprintf(stderr, "====>\t[V4L2] Get current palette : %d = ", 
		vd.picture.palette);
	int i;
	for(i=0; i<NB_PALETTES; i++) 
    	if(palettes_defined[i].value == vd.picture.palette) {
			fprintf(stderr, "'%s' SUPPORTED !!\n", palettes_defined[i].designation);
			m_capture_palette = palettes_defined[i].value;
			return 0;
		}
	
	for(i=0; i<NB_PALETTES; i++)
       	if(GrabCheck( palettes_defined[i].value ) == 0) {
        	fprintf(stderr, "\t\t[V4L2]::setGrabFormat : Palette %s is supported.\n",
         		palettes_defined[i].designation);
			
			m_capture_palette = palettes_defined[i].value;
			
			return 0;
		}

    fprintf(stderr, "!\t!\t[V4L2]::setGrabFormet : no supported palette found.\n");

	return -1;
}
/** v4l_getframebuffer - get frame buffer address
 */
int V4L2Device::v4l_getframebuffer()
{
	if(ioctl(vd.fd, VIDIOCGFBUF, &(vd.buffer)) < 0) {
		perror("!\t!\t[V4L2]::v4l_getframebuffer:VIDIOCGFBUF");
		return -1;
	}
	return 0;
}

/** v4l_setframebuffer - set parameters of frame buffer
 @param vd device handling struct
 @param base base PHYSICAL address of the frame buffer
 @param width width of the frame buffer
 @param height height of the frame buffer
 @param depth color depth of the frame buffer
 @param bpl number of bytes of memory between the start of two adjacent lines
 */
int V4L2Device::v4l_setframebuffer(void *base, 
	int width, int height, int depth, int bytesperline)
{
	vd.buffer.base = base;
	vd.buffer.width = width;
	vd.buffer.height = height;
	vd.buffer.depth = depth;
	vd.buffer.bytesperline = bytesperline;
	
	if(ioctl(vd.fd, VIDIOCSFBUF, &(vd.buffer)) < 0) {
		perror("!\t!\t[V4L2]::v4l_setframebuffer:VIDIOCSFBUF");
		return -1;
	}
	return 0;
}


/** v4l_setpalette - set the palette for the images
 */
int V4L2Device::v4l_setpalette(int pal)
{
	vd.picture.palette = pal;
	vd.mmap.format = pal;
	if(ioctl(vd.fd, VIDIOCSPICT, &(vd.picture)) < 0) {
		perror("!\t!\t[V4L2]::v4l_setpalette:VIDIOCSPICT");
		return -1;
	}
	m_capture_palette = pal;
	return 0;
}

/** v4l_setpicture - set the picture properties 
 @param vd device handling struct
 @param br picture brightness
 @param hue picture hue
 @param col picture color
 @param cont picture contrast
 @param white picture whiteness
 */
int V4L2Device::v4l_setpicture(int br, int hue, int col, int cont, int white)
{
	if(br>=0)
		vd.picture.brightness = br;
	if(hue>=0)
		vd.picture.hue = hue;
	if(col>=0)
		vd.picture.colour = col;
	if(cont>=0)
		vd.picture.contrast = cont;
	if(white>=0)
		vd.picture.whiteness = white;
	if(ioctl(vd.fd, VIDIOCSPICT, &(vd.picture)) < 0) {
		perror("!\t!\t[V4L2]::v4l_setpicture:VIDIOCSPICT");
		return -1;
	}
	return 0;
}

/** v4lgrabf - flip-flop grabbing
 */
int V4L2Device::v4l_grabf()
{
	int f;

	f = vd.frame;
	vd.frame = vd.frame ^ 1;
	return v4l_grabstart(f);
}

/** v4l_getaddress - returns a offset addres of buffer for mmap capturing
 */
unsigned char * V4L2Device::v4l_getaddress()
{
	return (vd.map + vd.mbuf.offsets[vd.frame]);
}
