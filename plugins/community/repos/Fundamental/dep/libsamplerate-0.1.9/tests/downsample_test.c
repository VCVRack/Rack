/*
** Copyright (c) 2008-2016, Erik de Castro Lopo <erikd@mega-nerd.com>
** All rights reserved.
**
** This code is released under 2-clause BSD license. Please see the
** file at : https://github.com/erikd/libsamplerate/blob/master/COPYING
*/

#include <stdio.h>
#include <stdlib.h>
#include <samplerate.h>

#include "util.h"

static void
downsample_test (int converter)
{	static float in [1000], out [10] ;
	SRC_DATA data ;

    printf ("        downsample_test     (%-28s) ....... ", src_get_name (converter)) ;
	fflush (stdout) ;

	data.src_ratio = 1.0 / 255.0 ;
	data.input_frames = ARRAY_LEN (in) ;
	data.output_frames = ARRAY_LEN (out) ;
	data.data_in = in ;
	data.data_out = out ;

	if (src_simple (&data, converter, 1))
	{	puts ("src_simple failed.") ;
		exit (1) ;
		} ;

	puts ("ok") ;
} /* downsample_test */

int
main (void)
{
	puts ("") ;

	downsample_test (SRC_ZERO_ORDER_HOLD) ;
	downsample_test (SRC_LINEAR) ;
	downsample_test (SRC_SINC_FASTEST) ;
	downsample_test (SRC_SINC_MEDIUM_QUALITY) ;
	downsample_test (SRC_SINC_BEST_QUALITY) ;

	puts ("") ;

	return 0 ;
} /* main */
