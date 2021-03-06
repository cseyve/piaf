/***************************************************************************
	 OpenCVVideoAcquisition.h  - OpenCV Capture acquisition class for video capture
							 -------------------
	begin                : Thu Dec 2 2010
	copyright            : (C) 2010 by Christophe Seyve (CSE)
	email                : cseyve@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <stdio.h>

#include "opencvvideoacquisition.h"
#include "swvideodetector.h"
#include "piaf-common.h"

#define OpenCVAcq_printf(...) fprintf( stderr,"[OpenCV %d]::%s:%d: ", m_idx_device, __func__, __LINE__); fprintf( stderr, __VA_ARGS__);

double swGetCaptureProperty(CvCapture * capture, int property);
void swSetCaptureProperty(CvCapture * capture, int property, double value);

OpenCVVideoAcquisition::OpenCVVideoAcquisition(int idx_device)
{
	clearImageInfoStruct(&mImageInfo);

	m_capture = NULL;
	m_iplImage = m_grayImage = NULL;
	m_idx_device = idx_device;

	memset(&m_video_properties, 0, sizeof(t_video_properties));
	m_imageSize = cvSize(0,0);

	OpenCVAcq_printf( "OpenCVAcq::%s:%d : create device # %d\n",
			__func__, __LINE__, idx_device);

	// open any type of device and index of desired device
#ifdef OPENCV_22
	try {
		m_capture = cvCreateCameraCapture(CV_CAP_ANY + m_idx_device );
	} catch (cv::Exception e) {
		OpenCVAcq_printf( "[VidAcq] : VDopen ERROR : CAUGHT EXCEPTION WHILE OPENING DEVICE  [%d]\n",
				idx_device
				); fflush(stderr);

		return;
	}
#else
	m_capture = cvCreateCameraCapture(CV_CAP_ANY + m_idx_device );
#endif
	OpenCVAcq_printf( "OpenCVAcq::%s:%d : create device # %d =W capture=%p\n",
			__func__, __LINE__,
			idx_device, m_capture);
	if(m_capture) {
		// try to use a big image size
		const int acq_widths[] = { 1920, 1600, 1280, 1024, 800, 720, 640, 360, 320, 160, 0 };
		const int acq_heights[]= { 1080, 1200, 720, 768, 600, 576, 480, 288, 240, 120, 0 };
		try {

			for(int acq_idx = 0; acq_widths[acq_idx]>0; acq_idx++ )
			{
				int retwidth = cvSetCaptureProperty(m_capture,CV_CAP_PROP_FRAME_WIDTH ,acq_widths[acq_idx]);
				int retheight = cvSetCaptureProperty(m_capture,CV_CAP_PROP_FRAME_HEIGHT ,acq_heights[acq_idx]);

				// check if size is ok
				double cur_width = swGetCaptureProperty(m_capture, CV_CAP_PROP_FRAME_WIDTH);
				double cur_height = swGetCaptureProperty(m_capture, CV_CAP_PROP_FRAME_HEIGHT);

				//
				OpenCVAcq_printf( "OpenCVCap::%s:%d : setprop (%dx%d) returned %d,%d => cur size=%gx%g\n",
						__func__, __LINE__,
						acq_widths[acq_idx], acq_heights[acq_idx],
						retwidth, retheight,
						cur_width, cur_height
						);

				if((int)round(cur_width) == acq_widths[acq_idx]
					&& (int)round(cur_height) == acq_heights[acq_idx]
					&& m_video_properties.max_width<1)
				{
					m_video_properties.max_width = acq_widths[acq_idx];
					m_video_properties.max_height = acq_heights[acq_idx];
				}
			}


			// finally use max width/height
			cvSetCaptureProperty(m_capture,CV_CAP_PROP_FRAME_WIDTH ,m_video_properties.max_width);
			cvSetCaptureProperty(m_capture,CV_CAP_PROP_FRAME_HEIGHT ,m_video_properties.max_height);
		}
		catch (cv::Exception e)
		{
			PIAF_MSG(SWLOG_WARNING, "Caught OpenCV exception");
		}
	}

}

OpenCVVideoAcquisition::~OpenCVVideoAcquisition()
{
	stopAcquisition();

}

/** \brief Use sequential mode
	If true, grabbing is done when a new image is requested.
	Else a thread is started to grab */
void OpenCVVideoAcquisition::setSequentialMode(bool on)
{

}

/** \brief Start acquisition */
int OpenCVVideoAcquisition::startAcquisition()
{
	m_imageSize.width = m_imageSize.height = 0;
	if(!m_capture)
	{
		// open any type of device and index of desired device
#ifdef OPENCV_22
		try {
			m_capture = cvCreateCameraCapture(CV_CAP_ANY + m_idx_device );
		} catch (cv::Exception e) {
			OpenCVAcq_printf( "[VidAcq] : VDopen ERROR : CAUGHT EXCEPTION WHILE OPENING DEVICE  [%d]\n",
							  m_idx_device
							  ); fflush(stderr);

			return -1;
		}
#else
		m_capture = cvCreateCameraCapture(CV_CAP_ANY + m_idx_device );
#endif
	}

	OpenCVAcq_printf( "[VAcq]::%s:%d : m_capture=%p\n", __func__, __LINE__,
			m_capture);
	if(m_capture) {
		OpenCVAcq_printf( "[VidAcq] : VDopen SUCCESS : OPEN DEVICE  [%d]\n",
				m_idx_device
				); fflush(stderr);
	} else {
		OpenCVAcq_printf( "[VidAcq] : VDopen ERROR : CANNOT OPEN DEVICE  [%d]\n",
				m_idx_device
				); fflush(stderr);
		return -1;
	}


	OpenCVAcq_printf( "[VidAcq] : VDopen : capture first frame to see if it's possible\n"); fflush(stderr);
	int ret = grab();
	if(ret >= 0) {
		OpenCVAcq_printf( "[VidAcq] : grab success =>  VDIsInitialised...\n"); fflush(stderr);
		m_captureIsInitialised = 1;
	} else {
		OpenCVAcq_printf( "[VidAcq] : grab failed => not initialized\n"); fflush(stderr);
		m_captureIsInitialised = 0;
	}

	// update video properties
	updateVideoProperties();

	return 0;
}

int OpenCVVideoAcquisition::grab()
{
	if(!m_capture)
	{
		OpenCVAcq_printf( "[VA ] %s:%d : capture=%p FAILED\n", __func__, __LINE__,
				m_capture);
		startAcquisition();
		return -1;
	}

	// Lock grab mutex
	mGrabMutex.lock();

	// Grab image
	if( !cvGrabFrame( m_capture ) ) {
		OpenCVAcq_printf( "[VA ] %s:%d : capture=%p FAILED\n", __func__, __LINE__,
				m_capture);

		mGrabMutex.unlock();
		return -1;

	} else {
		IplImage * frame = cvRetrieveFrame( m_capture );
		if(frame) {
			if(m_imageSize.width != frame->width
			   || m_imageSize.height != frame->height)
			{
				m_imageSize.width = frame->width;
				m_imageSize.height = frame->height;

				OpenCVAcq_printf( "[OpenCVVA]::%s:%d : capture=%p size changed => %dx%d\n",
						__func__, __LINE__,
						m_capture,
						m_imageSize.width, m_imageSize.height);
			}
		} else {
			OpenCVAcq_printf( "[OpenCVVA]::%s:%d : capture=%p cvRetrieveFrame FAILED\n",
					__func__, __LINE__,
					m_capture);
		}
	}

	// unock grab mutex
	mGrabMutex.unlock();

	return 0;
}

/* Return image size */
CvSize OpenCVVideoAcquisition::getImageSize()
{
	return m_imageSize;
}

/* Stop acquisition */
int OpenCVVideoAcquisition::stopAcquisition()
{
	if(m_capture) {
		OpenCVAcq_printf( "[VidAcq]::%s:%d : stopping capture !\n",
				__func__, __LINE__);
		cvReleaseCapture( &m_capture );
		m_capture = NULL;
	}

	swReleaseImage(&m_iplImage );
	swReleaseImage(&m_grayImage);

	m_iplImage = m_grayImage = NULL;

	m_captureIsInitialised = 0;

	OpenCVAcq_printf( "[VidAcq]::%s:%d : acquisition stopped !\n",
			__func__, __LINE__);

	return 0;
}
/** \brief Get the list of output format */
QList<t_video_output_format> OpenCVVideoAcquisition::getOutputFormats()
{
	t_video_output_format RGB32format;
	RGB32format.id = 0;
	strcpy(RGB32format.description, "BGR32");
	RGB32format.ipl_depth = IPL_DEPTH_8U;
	RGB32format.ipl_nchannels = 4;
	QList<t_video_output_format> out;
	out.append(RGB32format);
	DEBUG_MSG("NOT IMPLEMENTED => only RGB32");

	return out;
}

/** \brief Set the output format */
int OpenCVVideoAcquisition::setOutputFormat(int id)
{
	DEBUG_MSG("NOT IMPLEMENTED => id=%d", id);

	return id;
}

/** @brief Read image as data of selected format */
IplImage * OpenCVVideoAcquisition::readImage()
{
	DEBUG_MSG("NOT IMPLEMENTED => return readimageRGB32()");
	return readImageRGB32();
}

IplImage * OpenCVVideoAcquisition::readImageRaw()
{
	fprintf(stderr, "%s %s:%d : NOT IMPLEMENTED => return BGR32\n", __FILE__, __func__, __LINE__);
	if(m_iplImage)
	{
		OpenCVAcq_printf("iplImage = %dx%dx%dbx%d",
					 m_iplImage->width, m_iplImage->height, m_iplImage->depth, m_iplImage->nChannels
					 );
	}
	return readImageRGB32();
}

/** \brief Grabs one image and convert to RGB32 coding format
	if sequential mode, return last acquired image, else read and return image

	\return NULL if error
	*/
IplImage * OpenCVVideoAcquisition::readImageRGB32()
{
	if(!m_captureIsInitialised) return NULL;

	int ret = cvGrabFrame( m_capture );
	if(ret == 0) {
		OpenCVAcq_printf( "[VidAcq]::%s:%d : ERROR : cvGrabFrame returned %d !\n",
				__func__, __LINE__, ret);
		return NULL;
	}
	IplImage * frame = cvRetrieveFrame( m_capture );
	if(!m_iplImage && frame) {
		OpenCVAcq_printf( "[VidAcq]::%s:%d : created m_iplImage %d x %d x %d !\n",
				__func__, __LINE__,
				frame->width, frame->height, frame->nChannels);

		m_iplImage = swCreateImage(cvGetSize(frame), IPL_DEPTH_8U,
								   frame->nChannels );//== 1 ? 1 : 4);

		OpenCVAcq_printf( "[VidAcq]::%s:%d : created m_iplImage %d x %d x %d !\n",
				__func__, __LINE__,
				m_iplImage->width, m_iplImage->height, m_iplImage->nChannels);
// FIXME : fourcc
		if(m_iplImage->nChannels == 4) {
			m_video_properties.fourcc_dble = CV_FOURCC('B', 'G', 'R', 'A'); //VIDEO_PALETTE_RGB32;
		} else if(m_iplImage->nChannels == 3) {
			m_video_properties.fourcc_dble = CV_FOURCC('R', 'G', 'B', '\0'); //VIDEO_PALETTE_RGB24;
		} else {
			m_video_properties.fourcc_dble = CV_FOURCC('G', 'R', 'A', 'Y');//VIDEO_PALETTE_GREY;
		}
	}

	if(frame)
	{
		if(frame->width != m_iplImage->width
		   || frame->height != m_iplImage->height)
		{
			swReleaseImage(&m_iplImage);
			m_iplImage = swCreateImage(cvGetSize(frame), IPL_DEPTH_8U,
									  frame->nChannels == 1 ? 1 : 4 );
		}

		swConvert(frame, m_iplImage);
	}

	return m_iplImage;
}

/** \brief Grabs one image and convert to grayscale coding format
	if sequential mode, return last acquired image, else read and return image

	\return NULL if error
	*/
IplImage * OpenCVVideoAcquisition::readImageY()
{
	if(!m_captureIsInitialised) return NULL;
	if(!m_grayImage)
	{
		m_grayImage = cvCreateImage(cvGetSize(m_iplImage), IPL_DEPTH_8U, 1);
	}
	cvConvert(m_iplImage, m_grayImage);

	return NULL;
}

/** \brief Grabs one image of depth buffer
	if sequential mode, return last acquired image, else read and return image

	\return NULL if error
	*/
IplImage * OpenCVVideoAcquisition::readImageDepth()
{
	return NULL;
}


/** @brief Get video properties (not updated) */
t_video_properties OpenCVVideoAcquisition::getVideoProperties()
{
	return m_video_properties;
}
#define IMGSETTING_TO_CV	32768.

double swGetCaptureProperty(CvCapture * capture, int property)
{
	double ret = -1.;
	try {
		ret = cvGetCaptureProperty(capture, property);
	}
	catch(cv::Exception e)
	{
		PIAF_MSG(SWLOG_ERROR, "Caught exception for capture=%p prop=%d",
				 capture, property);
	}
	return ret;
}
void swSetCaptureProperty(CvCapture * capture, int property, double value)
{
	try {
		cvSetCaptureProperty(capture, property, value);
	}
	catch(cv::Exception e)
	{
		PIAF_MSG(SWLOG_ERROR, "Caught exception for capture=%p prop=%d value=%g",
				 capture, property, value);
	}
}

/** @brief Update and return video properties */
t_video_properties OpenCVVideoAcquisition::updateVideoProperties()
{
	if(!m_capture) {
		memset(&m_video_properties, 0, sizeof(t_video_properties));
		return m_video_properties;
	}

	//	m_video_properties. = swGetCaptureProperty(m_capture, );
	m_video_properties.pos_msec =		swGetCaptureProperty(m_capture, CV_CAP_PROP_POS_MSEC );/*Film current position in milliseconds or video capture timestamp */
	m_video_properties.pos_frames =		swGetCaptureProperty(m_capture, CV_CAP_PROP_POS_FRAMES );/*0-based index of the frame to be decoded/captured next */
	m_video_properties.pos_avi_ratio =	swGetCaptureProperty(m_capture, CV_CAP_PROP_POS_AVI_RATIO );/*Relative position of the video file (0 - start of the film, 1 - end of the film) */
	m_video_properties.frame_width =	swGetCaptureProperty(m_capture, CV_CAP_PROP_FRAME_WIDTH );/*Width of the frames in the video stream */
	m_video_properties.frame_height =	swGetCaptureProperty(m_capture, CV_CAP_PROP_FRAME_HEIGHT );/*Height of the frames in the video stream */
	m_video_properties.fps =			swGetCaptureProperty(m_capture, CV_CAP_PROP_FPS );/*Frame rate */
	m_video_properties.fourcc_dble =	swGetCaptureProperty(m_capture, CV_CAP_PROP_FOURCC );/*4-character code of codec */
//		char   fourcc[5] =	swGetCaptureProperty(m_capture, FOURCC coding CV_CAP_PROP_FOURCC 4-character code of codec */
//		char   norm[8] =		swGetCaptureProperty(m_capture, Norm: pal, ntsc, secam */
	m_video_properties.frame_count =	swGetCaptureProperty(m_capture, CV_CAP_PROP_FRAME_COUNT );/*Number of frames in the video file */
	m_video_properties.format =			swGetCaptureProperty(m_capture, CV_CAP_PROP_FORMAT );/*The format of the Mat objects returned by retrieve() */
	m_video_properties.mode =			swGetCaptureProperty(m_capture, CV_CAP_PROP_MODE );/*A backend-specific value indicating the current capture mode */

	m_video_properties.brightness =		swGetCaptureProperty(m_capture, CV_CAP_PROP_BRIGHTNESS ) *IMGSETTING_TO_CV;/*Brightness of the image (only for cameras) */
	m_video_properties.contrast =		swGetCaptureProperty(m_capture, CV_CAP_PROP_CONTRAST ) *IMGSETTING_TO_CV;/*Contrast of the image (only for cameras) */
	m_video_properties.saturation =		swGetCaptureProperty(m_capture, CV_CAP_PROP_SATURATION ) *IMGSETTING_TO_CV;/*Saturation of the image (only for cameras) */
	m_video_properties.hue =			swGetCaptureProperty(m_capture, CV_CAP_PROP_HUE ) *IMGSETTING_TO_CV;/*Hue of the image (only for cameras) */
#ifdef CV_CAP_PROP_WHITE_BALANCE
	m_video_properties.white_balance =	swGetCaptureProperty(m_capture, CV_CAP_PROP_WHITE_BALANCE ) *IMGSETTING_TO_CV;/*Currently unsupported */
#endif
	m_video_properties.gain =			swGetCaptureProperty(m_capture, CV_CAP_PROP_GAIN );/*Gain of the image (only for cameras) */
#ifdef OPENCV_22	
	m_video_properties.exposure =		swGetCaptureProperty(m_capture, CV_CAP_PROP_EXPOSURE );/*Exposure (only for cameras) */
#endif
	m_video_properties.convert_rgb =	swGetCaptureProperty(m_capture, CV_CAP_PROP_CONVERT_RGB );/*Boolean flags indicating whether images should be converted to RGB */

		/*! CV_CAP_PROP_RECTIFICATION TOWRITE (note: only supported by DC1394 v 2.x backend currently)
		– Property identifier. Can be one of the following:*/

	OpenCVAcq_printf( "OpenCVVideoAcquisition::%s:%d : props=\n", __func__, __LINE__);
	printVideoProperties(&m_video_properties);

	return m_video_properties;
}

#define SETCAPTUREPROP(_prop,_prop_id)		if(props._prop !=m_video_properties._prop) \
		{ \
			swSetCaptureProperty(m_capture,(_prop_id),props._prop); \
		}
#define SETCAPTUREPROPSCALED(_prop,_prop_id)		if(props._prop !=m_video_properties._prop) \
		{ \
			swSetCaptureProperty(m_capture,(_prop_id),props._prop/IMGSETTING_TO_CV); \
		}
/* Set video properties (not updated) */
int OpenCVVideoAcquisition::setVideoProperties(t_video_properties props)
{
	if(!m_capture) {
		OpenCVAcq_printf( "OpenCVVidAcq::%s:%d : device not opened "
				"=> cannot change video properties:", __func__, __LINE__);

		return -1;
	}

	OpenCVAcq_printf( "OpenCVVidAcq::%s:%d : change video properties:", __func__, __LINE__);
	printVideoProperties(&props);


	// lock grab mutex
	mGrabMutex.lock();

	//	m_video_properties. = swGetCaptureProperty(m_capture, );
	SETCAPTUREPROP( pos_msec , CV_CAP_PROP_POS_MSEC );/*Film current position in milliseconds or video capture timestamp */
	SETCAPTUREPROP(	pos_frames , CV_CAP_PROP_POS_FRAMES );/*0-based index of the frame to be decoded/captured next */
	SETCAPTUREPROP(	pos_avi_ratio , CV_CAP_PROP_POS_AVI_RATIO );/*Relative position of the video file (0 - start of the film, 1 - end of the film) */

	if(props.frame_width != m_video_properties.frame_width
	   || props.frame_height != m_video_properties.frame_height) {
		OpenCVAcq_printf( "[OpenCVVidAcq]::%s:%d : SIZE CHANGED FOR DEVICE  [%d] => Stop acquisition...\n",
				__func__, __LINE__, m_idx_device
				); fflush(stderr);


		//stopAcquisition();
		cvReleaseCapture(&m_capture);

		OpenCVAcq_printf( "[OpenCVVidAcq]::%s:%d : SIZE CHANGED FOR DEVICE [%d] => recreate capture...\n",
				__func__, __LINE__, m_idx_device
				); fflush(stderr);
#ifdef OPENCV_22
		try {
			m_capture = cvCreateCameraCapture(CV_CAP_ANY + m_idx_device );
		} catch (cv::Exception e) {
#else
		m_capture = cvCreateCameraCapture(CV_CAP_ANY + m_idx_device );
		if(!m_capture) {
#endif
			OpenCVAcq_printf( "[VidAcq] : VDopen ERROR : CAUGHT EXCEPTION WHILE OPENING DEVICE  [%d]\n",
					m_idx_device
					); fflush(stderr);

			return -1;
		}

		OpenCVAcq_printf( "[OpenCVVidAcq]::%s:%d : SIZE CHANGED FOR DEVICE  [%d] : "
				"=> capture=%p / change properties to %dx%d\n",
				__func__, __LINE__, m_idx_device,
				m_capture,
				props.frame_width, props.frame_height
				); fflush(stderr);
		SETCAPTUREPROP(	frame_width , CV_CAP_PROP_FRAME_WIDTH );/*Width of the frames in the video stream */
		SETCAPTUREPROP(	frame_height , CV_CAP_PROP_FRAME_HEIGHT );/*Height of the frames in the video stream */

		// unlock grab mutex
//		mGrabMutex.unlock();

//		OpenCVAcq_printf( "[OpenCVVidAcq]::%s:%d : SIZE CHANGED FOR DEVICE [%d] => restart acquisition\n",
//				__func__, __LINE__, m_idx_device
//				); fflush(stderr);
//		startAcquisition();
	}

	SETCAPTUREPROP(	fps , CV_CAP_PROP_FPS );/*Frame rate */
	SETCAPTUREPROP(	fourcc_dble , CV_CAP_PROP_FOURCC );/*4-character code of codec */
	//		char   fourcc[5] =	swGetCaptureProperty(m_capture, FOURCC coding CV_CAP_PROP_FOURCC 4-character code of codec */
	//		char   norm[8] =		swGetCaptureProperty(m_capture, Norm: pal, ntsc, secam */
	SETCAPTUREPROP(	frame_count , CV_CAP_PROP_FRAME_COUNT );/*Number of frames in the video file */
	SETCAPTUREPROP(	format , CV_CAP_PROP_FORMAT );/*The format of the Mat objects returned by retrieve() */
	SETCAPTUREPROP(	mode , CV_CAP_PROP_MODE );/*A backend-specific value indicating the current capture mode */
	/*
	  Optimal parameters for Microsoft Lifecam

	Props = { pos_msec=-1, pos_frames=-1, pos_avi_ratio=-1,
	frame_width=1280, frame_height=720,
	fps=-1, fourcc_dble=-1, fourcc='', norm='', frame_count=-1, format=-1, mode=-1,

	brightness=17913.2, contrast=29491.2, saturation=19660.8, hue=0,

	gain=-1, exposure=-1, convert_rgb=-1, white_balance=-32768,  }
	*/
	SETCAPTUREPROPSCALED(	brightness , CV_CAP_PROP_BRIGHTNESS );/*Brightness of the image (only for cameras) */
	SETCAPTUREPROPSCALED(	contrast , CV_CAP_PROP_CONTRAST );/*Contrast of the image (only for cameras) */
	SETCAPTUREPROPSCALED(	saturation , CV_CAP_PROP_SATURATION );/*Saturation of the image (only for cameras) */
	SETCAPTUREPROPSCALED(	hue , CV_CAP_PROP_HUE );/*Hue of the image (only for cameras) */
#ifdef CV_CAP_PROP_WHITE_BALANCE
	SETCAPTUREPROPSCALED(	white_balance , CV_CAP_PROP_WHITE_BALANCE );/*Currently unsupported */
#endif
	SETCAPTUREPROP(	gain , CV_CAP_PROP_GAIN );/*Gain of the image (only for cameras) */
#ifdef CV_CAP_PROP_EXPOSURE
	SETCAPTUREPROP(	exposure , CV_CAP_PROP_EXPOSURE );/*Exposure (only for cameras) */
#endif
	SETCAPTUREPROP(	convert_rgb , CV_CAP_PROP_CONVERT_RGB );/*Boolean flags indicating whether images should be converted to RGB */

	// unlock grab mutex
	mGrabMutex.unlock();

	updateVideoProperties();
	return 0;
}



