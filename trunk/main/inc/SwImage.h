/***************************************************************************
                SWimage.h  -  Sisell Workshop measure class
                             -------------------
    begin                : Sun May 5 2002
    copyright            : (C) 2002 by Olivier Vin (OLV) 
                                     & Christophe Seyve (CSE)
    email                : olivier.vine@sisell.com 
                           & cseyve@free.fr 
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _SW_IMAGE_
#define _SW_IMAGE_

#include "sw_types.h"

/// Image struct for binary exchanges
typedef struct _swImageStruct {
	i32 width;	/*! Image width = nb columns = pitch */
	i32 height;	/*! Image height = nb rows */


	// Date of image ===========================================================
	u32 Date;	/*! Date in second from Epoch */
	u32 TickCount;	/*! Tick count for accurate dating */

 	// Measure parameters ======================================================
	float FrameRate;	/*! Input movie/device framerate in frame per sec */

	// Image data description ==================================================
  	swType pixelType;	/*! Type of data for one pixel */

  	u8 depth;	/*! Depth of image, e.g. nb of channels (3 for RGB24, ...) */

	u32 buffer_size;	/*! Size of image buffer */
  	i32 deltaTus;		/*! */

	void * buffer;	/*! Raw image buffer */


	u32	metadata_size;
	unsigned char * metadata;

} swImageStruct;

/// Measure struct for binary exchanges
typedef struct _swMeasure1DStruct {

  // Date of measure
  unsigned long Date;
  unsigned long TickCount;

  // Measure parameters
  float FrameRate;

  swType pixelType;

  int 	CAN_size;
  int 	frequency;

  unsigned char depth;
  unsigned long buffer_size;
  void * buffer;
  
} swMeasure1DStruct;


/// Obsolete image class. Do not use.
class SwImage
{
 public:
  SwImage();

  ~SwImage();


 private:
	 swImageStruct image;
	 // Measure Buffer
	 
};





#endif