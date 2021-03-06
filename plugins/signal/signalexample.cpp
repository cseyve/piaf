/***************************************************************************
    main.cpp  -  description
                             -------------------
    begin                : Tue Jul  2 15:52:21 CEST 2002
    copyright            : (C) 2002 by Christophe Seyve
    email                : christophe.seyve@sisell.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>


// include componenet header
#include "SwPluginCore.h"

/* compile with :
gcc -Wall -I/usr/local/sisell/include SwPluginCore.cpp -c
gcc -Wall -I/usr/local/sisell/include main.cpp -c
gcc main.o SwPluginCore.o -o swcorecomponent 
# needs SwPluginCore.o
*/


/********************** GLOBAL SECTION ************************
DO NOT MODIFY THIS SECTION
plugin.data_in  : input data. You should cast this pointer to read the
			data with the correct type
plugin.data_out : output data. You should cast this pointer to write the
			data with the correct type
***************************************************************/
SwPluginCore plugin;

/******************* END OF GLOBAL SECTION ********************/

/********************** USER SECTION ************************
YOU SHOULD MODIFY THIS SECTION
Declare your variables, function parameters and function list
	

***************************************************************/
#define CATEGORY	"Signal"
#define SUBCATEGORY	"Mean"


// function parameters
unsigned char threshold_min = 127;
unsigned char threshold_max = 255;

swFuncParams threshold_params[] = {
	{"min", swU8, (void *)&threshold_min},
	{"max", swU8, (void *)&threshold_max},
};

swFuncParams invert_params[] = {
};


float erode_radius = 3.1415927;
short erode_iterations = 1;

swFuncParams erode_params[] = {
	{"radius", swFloat, (void *)&erode_radius},
	{"iteration", swS16, (void *)&erode_iterations}
};


void mean();

/* swFunctionDescriptor : 
	char * : function name
	int : number of parameters
	swFuncParams : function parameters
	swType : input type
	swType : output type
	void * : procedure
	*/
swFunctionDescriptor functions[] = {
	{"Mean", 		0, 	NULL, 				swMeasure1D, swMeasure1D, &mean, NULL},
};
int nb_functions = 1;

/******************* END OF USER SECTION ********************/



// function mean
void mean()
{
	swMeasure1DStruct * mIn = ((swMeasure1DStruct *)plugin.data_in);
	swMeasure1DStruct * mOut = ((swMeasure1DStruct *)plugin.data_out);
	double * dataIn  = (double *)mIn->buffer;
	double * dataOut = (double *)mOut->buffer;
	
	dataOut[0] = dataIn[0];
	
	for(unsigned long r = 1; r < mIn->buffer_size / sizeof(double); r++)
	{
		dataOut[r] = 255 - imageIn[r];
	}
}

// function threshold
void threshold()
{
	swImageStruct * imIn = ((swImageStruct *)plugin.data_in);
	swImageStruct * imOut = ((swImageStruct *)plugin.data_out);
	unsigned char * imageIn  = (unsigned char *)imIn->buffer;
	unsigned char * imageOut = (unsigned char *)imOut->buffer;
	
	
	for(unsigned long r = 0; r < imIn->buffer_size; r++)
	{
		if(imageIn[r] >= threshold_min && imageIn[r] <= threshold_max)
			imageOut[r] = 255;
		else
			imageOut[r] = 0;
	}
}

// function erode
void erode()
{
	swImageStruct * imIn = ((swImageStruct *)plugin.data_in);
	swImageStruct * imOut = ((swImageStruct *)plugin.data_out);
	unsigned char * imageIn  = (unsigned char *)imIn->buffer;
	unsigned char * imageOut = (unsigned char *)imOut->buffer;

	memcpy(imageOut, imageIn, imIn->buffer_size);
		
	for(int i=0; i<erode_iterations;i++)
	for(unsigned long r = 0; r < imIn->buffer_size; r++)
	{
		imageOut[r] = ((imageOut[r] & 0xFE) >> 1) | 0x80;
	}
}


// DO NOT MODIFY MAIN PROC
void signalhandler(int sig)
{
	fprintf(stderr, "================== RECEIVED SIGNAL %d = '%s' From process %d ==============\n", sig, sys_siglist[sig], getpid());
	signal(sig, signalhandler);
	
	exit(0);
}
int main(int argc, char *argv[])
{
	// SwPluginCore load
	for(int i=0; i<NSIG; i++)
		signal(i, signalhandler);
		
	fprintf(stderr, "registerCategory...\n");
	plugin.registerCategory(CATEGORY, SUBCATEGORY);
	
	// register functions 
	fprintf(stderr, "registerFunctions...\n");
	plugin.registerFunctions(functions, nb_functions );

	// process loop
	fprintf(stderr, "loop...\n");
	plugin.loop();

	fprintf(stderr, "exit(EXIT_SUCCESS). Bye.\n");
  	return EXIT_SUCCESS;
}
