/***************************************************************************
	 virtualdeviceacquisition.cpp  - Virtual acquisition class for video capture
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

#include "virtualdeviceacquisition.h"
#include "piaf-common.h"

void printVideoProperties(t_video_properties * props)
{
	fprintf(stderr, "Props = { ");
	fprintf(stderr, "pos_msec=%g, ", props->pos_msec);
	fprintf(stderr, "pos_frames=%g, ", props->pos_frames);
	fprintf(stderr, "pos_avi_ratio=%g, ", props->pos_avi_ratio);
	fprintf(stderr, "frame_width=%d x ", props->frame_width);
	fprintf(stderr, "frame_height=%d, ", props->frame_height);
	fprintf(stderr, "fps=%g, ", props->fps);
	fprintf(stderr, "fourcc_dble=%g, ", props->fourcc_dble);
	fprintf(stderr, "fourcc='%s', ", props->fourcc);
	fprintf(stderr, "norm='%s', ", props->norm);
	fprintf(stderr, "frame_count=%g, ", props->frame_count);
	fprintf(stderr, "format=%g, ", props->format);
	fprintf(stderr, "mode=%g, ", props->mode);
	fprintf(stderr, "brightness=%d, ", props->brightness);
	fprintf(stderr, "contrast=%d, ", props->contrast);
	fprintf(stderr, "saturation=%d, ", props->saturation);
	fprintf(stderr, "hue=%d, ", props->hue);
	fprintf(stderr, "gain=%d, ", props->gain);
	fprintf(stderr, "exposure=%g, ", props->exposure);
	fprintf(stderr, "convert_rgb=%g, ", props->convert_rgb);
	fprintf(stderr, "white_balance=%d, ", props->white_balance);

	fprintf(stderr, " }\n");
}







