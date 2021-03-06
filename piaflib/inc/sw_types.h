/***************************************************************************
           sw_types.h  -  Piaf defined types
                             -------------------
    begin                : Wed Nov 13 10:07:22 CET 2002
    copyright            : (C) 2002 by Christophe Seyve
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

#ifndef SW_TYPES_H
#define SW_TYPES_H


#include <stdlib.h>
#include <stdint.h>

#ifndef FALSE
#define FALSE 0
#define TRUE  1
#endif

/*
 * redefinition of standard types
 */
#ifndef i8
typedef uint8_t		 	u8;
typedef int8_t 			i8;
typedef uint16_t 		u16;
typedef int16_t			i16;
typedef uint32_t		u32;
typedef int32_t 		i32;
typedef float			d32;
typedef double			d64;
typedef long double 		d80;
#endif

#ifndef ulong
typedef unsigned long ulong ;
#endif

/** @brief Region of interest / rectangle position and size */
typedef struct _tBoxSize {
	unsigned long x;
	unsigned long y;
	unsigned long width;
	unsigned long height;
} tBoxSize;


#endif
