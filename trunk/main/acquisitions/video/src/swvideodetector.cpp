/***************************************************************************
	swvideodetector.cpp
		  generic functions for Sisell Workshop video detectors (motion, presence...)
							 -------------------
	begin                : Tue Mar 12 2002
	copyright            : (C) 2002 by Olivier Vin & Christophe Seyve
	email                : olivier.vine@sisell.com  & cseyve@free.fr
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
#include <stdlib.h>

#include "swvideodetector.h"

void SavePPMFile(char *filename, bool colored, CvSize size, unsigned char *buffer)
{
    FILE * f = fopen(filename, "wb");
    if(!f) {
		fprintf(stderr, "SavePPMFile : cannot open file '%s'\n", filename);
        return;
    }

    // Write header
    fprintf(f, "P%c\n# generated by SWvideodetector.cpp (SISELL)\n%d %d\n255\n",
            (colored?'6':'5'),
            (int)size.width, (int)size.height);
    fwrite(buffer, size.width*size.height*(colored?1:3), sizeof(unsigned char), f);
    fclose(f);
}


CvSize LoadPPMHeader(char *name)
{
	FILE * f;
	unsigned long size = 0;
	CvSize rSize;

	if(!(f = fopen(name, "rb"))) {
			fprintf(stderr, "Cannot open file '%s'\n", name);
			rSize.width = rSize.height = 0;
			return rSize;
	}

	// Read the first line, normaly p6 for color picture, or p5 for greyscale
	char line[512];
	do {
			fgets(line, 512, f);
	} while( line[0] == '#'); // comment line
	if(strncmp(line, "P6", 2) ==0) {
			size = 3;
			printf("\tColor image\n");
	}
	if(strncmp(line, "P5", 2) ==0) {
			size = 1;
			printf("\tGreyscaled image\n");
	}
	// read size : width height
	do {
			fgets(line, 512, f);
	} while( line[0] == '#'); // comment line

	int width, height=0;

	if(sscanf(line, "%d%d", &width, &height) == 2) {
		rSize.width = width;
		rSize.height = height;
	}

	return rSize;
}


int LoadPPMFile(char *name, unsigned char * Image, int maxsize)
{
	FILE * f;
	long size = 0;
	int width = 0, height=0;
	char line[512];

	if(!(f = fopen(name, "rb"))) {
			fprintf(stderr, "Cannot open file '%s'\n", name);
			return 0;
	}

	// Read the first line, normaly p6 for color picture, or p5 for greyscale
	do {
			fgets(line, 512, f);
	} while( line[0] == '#'); // comment line
	if(strncmp(line, "P6", 2) ==0) {
			size = 3;
			printf("\tColor image\n");
	}
	if(strncmp(line, "P5", 2) ==0) {
			size = 1;
			printf("\tGreyscaled image\n");
	}

	// read size : width height
	do {
			fgets(line, 512, f);
	} while( line[0] == '#'); // comment line

	sscanf(line, "%d%d", &width, &height);

	size *= width * height;
	printf("\tSize : %d x %d = %lu bytes\n", width, height, size);
	if(size > maxsize) {
		fclose(f);
		return 0;
	}

	// Reading the image data
	// next line always equal 255
	fgets(line, 512, f);

	// We assume that pixmap is already allocated
	fread(Image, sizeof(unsigned char), size, f);

	fclose(f);
	return 1;
}







int swByteDepth(IplImage * iplImage) {
	int byte_depth = iplImage->nChannels;

	/* IplImage:
	int  depth; // pixel depth in bits: IPL_DEPTH_8U, IPL_DEPTH_8S, IPL_DEPTH_16S,
			IPL_DEPTH_32S, IPL_DEPTH_32F and IPL_DEPTH_64F are supported
	*/

	switch(iplImage->depth) {
	case IPL_DEPTH_8U:
		byte_depth *= sizeof(unsigned char);
		break;
	case IPL_DEPTH_8S:
		byte_depth *= sizeof(char);
		break;
	case IPL_DEPTH_16U:
		byte_depth *= sizeof(u16);
		break;
	case IPL_DEPTH_16S:
		byte_depth *= sizeof(i16);
		break;
	case IPL_DEPTH_32S:
		byte_depth *= sizeof(i32);
		break;
	case IPL_DEPTH_32F:
		byte_depth *= sizeof(float);
		break;
	case IPL_DEPTH_64F:
		byte_depth *= sizeof(double);
		break;
	default:
		break;
	}

	return byte_depth;
}
/* Print image properties */
void swPrintProperties(IplImage * img) {
	/*
		 *

	  IplImage
		|-- int  nChannels;     // Number of color channels (1,2,3,4)
		|-- int  depth;         // Pixel depth in bits:
		|                       //   IPL_DEPTH_8U, IPL_DEPTH_8S,
		|                       //   IPL_DEPTH_16U,IPL_DEPTH_16S,
		|                       //   IPL_DEPTH_32S,IPL_DEPTH_32F,
		|                       //   IPL_DEPTH_64F
		|-- int  width;         // image width in pixels
		|-- int  height;        // image height in pixels
		|-- char* imageData;    // pointer to aligned image data
		|                       // Note that color images are stored in BGR order
		|-- int  dataOrder;     // 0 - interleaved color channels,
		|                       // 1 - separate color channels
		|                       // cvCreateImage can only create interleaved images
		|-- int  origin;        // 0 - top-left origin,
		|                       // 1 - bottom-left origin (Windows biswaps style)
		|-- int  widthStep;     // size of aligned image row in bytes
		|-- int  imageSize;     // image data size in bytes = height*widthStep
		|-- struct _IplROI *roi;// image ROI. when not NULL specifies image
		|                       // region  to be processed.
		|-- char *imageDataOrigin; // pointer to the unaligned origin of image data
		|                          // (needed for correct image deallocation)
		|
		|-- int  align;         // Alignment of image rows: 4 or 8 byte alignment
		|                       // OpenCV ignores this and uses widthStep instead
		|-- char colorModel[4]; // Color model - ignored by OpenCV

	 */
	fprintf(stderr, "[imgutils] %s:%d : IMAGE PROPERTIES : img=%p\n"
			"\t nChannels = %d\n"
			"\t depth = %d => %d bytes per pixel\n"
			"\t width = %d\n"
			"\t height = %d\n"
			"\t imageData = %p\n"
			"\t dataOrder = %d - %s\n"
			"\t origin = %d - %s\n"
			"\t widthStep = %d\n"
			"\t imageSize = %d\n"
			"\t align = %d (OpenCV ignores this and uses widthStep instead)\n"
			"\t colorModel = %c%c%c%c\n"
			"==============================\n\n"

			, __func__, __LINE__, img,
			img->nChannels,
			img->depth, swByteDepth(img),
			img->width,
			img->height,
			img->imageData,
			img->dataOrder, (img->dataOrder==0?"INTERLEAVED":"SEPARATE"),
			img->origin,(img->origin==0?"TOP-LEFT":"BOTTOM-LEFT (Windows BMP)"),
			img->widthStep,
			img->imageSize,
			img->align,
			img->colorModel[0],img->colorModel[1],img->colorModel[2],img->colorModel[3]

			);

}

/* Create an IplImage width OpenCV's cvCreateImage and clear buffer */
IplImage * swCreateImage(CvSize size, int depth, int channels) {

	/*
	fprintf(stderr, "[utils] %s:%d : creating IplImage : %dx%d x depth=%d x channels=%d\n",
			__func__, __LINE__,
			size.width, size.height, depth, channels);
	*/
	IplImage * img = cvCreateImage(size, depth, channels);
	if(img) {
		if(!(img->imageData)) {
			fprintf(stderr, "[utils] %s:%d : ERROR : img->imageData=NULL while "
				"creating IplImage => %dx%d x depth=%d x channels=%d\n",
				__func__, __LINE__,
				img->width, img->height, img->depth, img->nChannels);
		}
		memset(img->imageData, 0, sizeof(char) * img->widthStep * img->height);
		return img;
	} else {
		fprintf(stderr, "[utils] %s:%d : ERROR : creating IplImage => %dx%d x depth=%d x channels=%d\n",
			__func__, __LINE__,
			img->width, img->height, img->depth, img->nChannels);
	}

	if(img->width==0 || img->height==0 || img->imageData==0) {
		fprintf(stderr, "[utils] %s:%d : ERROR : creating IplImage => %dx%d x depth=%d x channels=%d\n",
			__func__, __LINE__,
			img->width, img->height, img->depth, img->nChannels);
	}

	return img;
}

/* Create an IplImage header width OpenCV's cvCreateImage and clear buffer */
IplImage * swCreateImageHeader(CvSize size, int depth, int channels) {

	/*
	fprintf(stderr, "[utils] %s:%d : creating IplImage : %dx%d x depth=%d x channels=%d\n",
			__func__, __LINE__,
			size.width, size.height, depth, channels);
	*/
	IplImage * img = cvCreateImageHeader(size, depth, channels);
	if(img) {
		if(!(img->imageData)) {
			fprintf(stderr, "[utils] %s:%d : ERROR : img->imageData=NULL while "
				"creating IplImage header => %dx%d x depth=%d x channels=%d\n",
				__func__, __LINE__,
				img->width, img->height, img->depth, img->nChannels);
		}
		return img;
	} else {
		fprintf(stderr, "[utils] %s:%d : ERROR : creating IplImage => %dx%d x depth=%d x channels=%d\n",
			__func__, __LINE__,
			img->width, img->height, img->depth, img->nChannels);
	}

	if(img->width==0 || img->height==0 || img->imageData==0) {
		fprintf(stderr, "[utils] %s:%d : ERROR : creating IplImage => %dx%d x depth=%d x channels=%d\n",
			__func__, __LINE__,
			img->width, img->height, img->depth, img->nChannels);
	}

	return img;
}

/* Release an image and clear pointer */
void swReleaseImage(IplImage ** img) {
	if(!img) return;
	if(!(*img) ) return;

#if defined(OPENCV2)
	try {
		cvReleaseImage(img);
	} catch(cv::Exception e) {
		fprintf(stderr, "[swvideodetector] %s:%d : error in openCV cvReleaseImage ", __func__, __LINE__);
	}
#else
	cvReleaseImage(img);
#endif
	*img = NULL;
}

/* Release an image header and clear pointer */
void swReleaseImageHeader(IplImage ** img) {
	if(!img) return;
	if(!(*img) ) return;
#ifdef OPENCV2
	try {
		swReleaseImageHeader(img);
	} catch(cv::Exception e) {
		fprintf(stderr, "[swvideodetector] %s:%d : error in openCV cvReleaseImage ", __func__, __LINE__);
	}
#else
	cvReleaseImageHeader(img);
#endif

	*img = NULL;
}

// If image has a odd size, many functions will fail on OpenCV, so add a pixel
IplImage * swAddBorder4x(IplImage * originalImage) {
	if(   (originalImage->width % 4) > 0
	   || (originalImage->height % 4) > 0
	   ) {

		int new_width = originalImage->width;
		while( (new_width % 4)) new_width++;
		int new_height = originalImage->height;
		while( (new_height % 4)) new_height++;

		IplImage * copyImage = swCreateImage(
				cvSize( new_width, new_height),
				originalImage->depth,
				originalImage->nChannels);

		for(int r = 0; r < originalImage->height; r++) {
			memcpy( copyImage->imageData + r * copyImage->widthStep,
				originalImage->imageData + r * originalImage->widthStep, originalImage->widthStep);
		}


		return copyImage;
	}
	else {
		IplImage * copyImage = swCreateImage(cvSize(originalImage->width, originalImage->height), IPL_DEPTH_8U, originalImage->nChannels);
		cvCopy(originalImage, copyImage);
		return copyImage;
	}
	return originalImage;
}


/* Convert an image into another */
int swConvert(IplImage *imageIn, IplImage * imageRGBA)
{


	// convert
	switch(imageIn->nChannels)
	{
	case 1:
		switch(imageRGBA->nChannels)
		{
		case 1:
			cvCopy(imageIn, imageRGBA);
			break;
		case 4:
			cvCvtColor(imageIn, imageRGBA, CV_GRAY2BGRA);
			break;
		}

		break;
	case 3:
		switch(imageRGBA->nChannels)
		{
		case 1:
			cvCvtColor(imageIn, imageRGBA, CV_RGB2GRAY);
			break;
		case 3:
			cvCopy(imageIn, imageRGBA);
			break;
		case 4:
			cvCvtColor(imageIn, imageRGBA, CV_RGB2BGRA);
			break;
		}

		break;
	case 4:
		switch(imageRGBA->nChannels)
		{
		case 1:
			cvCvtColor(imageIn, imageRGBA, CV_RGBA2GRAY);
			break;
		case 3:
			cvCvtColor(imageIn, imageRGBA, CV_RGBA2RGB);
			break;
		case 4:
			cvCopy(imageIn, imageRGBA);
			break;
		}

		break;
	}

	return 0;
}
