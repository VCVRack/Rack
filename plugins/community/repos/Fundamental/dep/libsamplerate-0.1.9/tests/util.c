/*
** Copyright (c) 2002-2016, Erik de Castro Lopo <erikd@mega-nerd.com>
** All rights reserved.
**
** This code is released under 2-clause BSD license. Please see the
** file at : https://github.com/erikd/libsamplerate/blob/master/COPYING
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include	"util.h"

#ifndef	M_PI
#define	M_PI			3.14159265358979323846264338
#endif

void
gen_windowed_sines (int freq_count, const double *freqs, double max, float *output, int output_len)
{	int 	k, freq ;
	double	amplitude, phase ;

	amplitude = max / freq_count ;

	for (k = 0 ; k < output_len ; k++)
		output [k] = 0.0 ;

	for (freq = 0 ; freq < freq_count ; freq++)
	{	phase = 0.9 * M_PI / freq_count ;

		if (freqs [freq] <= 0.0 || freqs [freq] >= 0.5)
		{	printf ("\n%s : Error : freq [%d] == %g is out of range. Should be < 0.5.\n", __FILE__, freq, freqs [freq]) ;
			exit (1) ;
			} ;

		for (k = 0 ; k < output_len ; k++)
			output [k] += amplitude * sin (freqs [freq] * (2 * k) * M_PI + phase) ;
		} ;

	/* Apply Hanning Window. */
	for (k = 0 ; k < output_len ; k++)
		output [k] *= 0.5 - 0.5 * cos ((2 * k) * M_PI / (output_len - 1)) ;

	/*	data [k] *= 0.3635819 - 0.4891775 * cos ((2 * k) * M_PI / (output_len - 1))
					+ 0.1365995 * cos ((4 * k) * M_PI / (output_len - 1))
					- 0.0106411 * cos ((6 * k) * M_PI / (output_len - 1)) ;
		*/

	return ;
} /* gen_windowed_sines */

void
save_oct_float (char *filename, float *input, int in_len, float *output, int out_len)
{	FILE 	*file ;
	int		k ;

	printf ("Dumping input and output data to file : %s.\n\n", filename) ;

	if (! (file = fopen (filename, "w")))
		return ;

	fprintf (file, "# Not created by Octave\n") ;

	fprintf (file, "# name: input\n") ;
	fprintf (file, "# type: matrix\n") ;
	fprintf (file, "# rows: %d\n", in_len) ;
	fprintf (file, "# columns: 1\n") ;

	for (k = 0 ; k < in_len ; k++)
		fprintf (file, "% g\n", input [k]) ;

	fprintf (file, "# name: output\n") ;
	fprintf (file, "# type: matrix\n") ;
	fprintf (file, "# rows: %d\n", out_len) ;
	fprintf (file, "# columns: 1\n") ;

	for (k = 0 ; k < out_len ; k++)
		fprintf (file, "% g\n", output [k]) ;

	fclose (file) ;
	return ;
} /* save_oct_float */

void
save_oct_double (char *filename, double *input, int in_len, double *output, int out_len)
{	FILE 	*file ;
	int		k ;

	printf ("Dumping input and output data to file : %s.\n\n", filename) ;

	if (! (file = fopen (filename, "w")))
		return ;

	fprintf (file, "# Not created by Octave\n") ;

	fprintf (file, "# name: input\n") ;
	fprintf (file, "# type: matrix\n") ;
	fprintf (file, "# rows: %d\n", in_len) ;
	fprintf (file, "# columns: 1\n") ;

	for (k = 0 ; k < in_len ; k++)
		fprintf (file, "% g\n", input [k]) ;

	fprintf (file, "# name: output\n") ;
	fprintf (file, "# type: matrix\n") ;
	fprintf (file, "# rows: %d\n", out_len) ;
	fprintf (file, "# columns: 1\n") ;

	for (k = 0 ; k < out_len ; k++)
		fprintf (file, "% g\n", output [k]) ;

	fclose (file) ;
	return ;
} /* save_oct_double */

void
interleave_data (const float *in, float *out, int frames, int channels)
{	int fr, ch ;

	for (fr = 0 ; fr < frames ; fr++)
		for (ch = 0 ; ch < channels ; ch++)
			out [ch + channels * fr] = in [fr + frames * ch] ;

	return ;
} /* interleave_data */

void
deinterleave_data (const float *in, float *out, int frames, int channels)
{	int fr, ch ;

	for (ch = 0 ; ch < channels ; ch++)
		for (fr = 0 ; fr < frames ; fr++)
			out [fr + frames * ch] = in [ch + channels * fr] ;

	return ;
} /* deinterleave_data */

void
reverse_data (float *data, int datalen)
{	int left, right ;
	float temp ;

	left = 0 ;
	right = datalen - 1 ;

	while (left < right)
	{	temp = data [left] ;
		data [left] = data [right] ;
		data [right] = temp ;
		left ++ ;
		right -- ;
		} ;

} /* reverse_data */

const char *
get_cpu_name (void)
{
	const char *name = "Unknown", *search = NULL ;
	static char buffer [512] ;
	FILE * file = NULL ;
	int is_pipe = 0 ;

#if defined (__linux__)
	file = fopen ("/proc/cpuinfo", "r") ;
	search = "model name" ;
#elif defined (__APPLE__)
	file = popen ("/usr/sbin/system_profiler -detailLevel full SPHardwareDataType", "r") ;
	search = "Processor Name" ;
	is_pipe = 1 ;
#elif defined (__FreeBSD__)
	file = popen ("sysctl -a", "r") ;
	search = "hw.model" ;
	is_pipe = 1 ;
#else
	file = NULL ;
#endif

	if (file == NULL)
		return name ;

	if (search == NULL)
	{	printf ("Error : search is NULL in function %s.\n", __func__) ;
		return name ;
		} ;

	while (fgets (buffer, sizeof (buffer), file) != NULL)
		if (strstr (buffer, search))
		{	char *src, *dest ;

			if ((src = strchr (buffer, ':')) != NULL)
			{	src ++ ;
				while (isspace (src [0]))
					src ++ ;
				name = src ;

				/* Remove consecutive spaces. */
				src ++ ;
				for (dest = src ; src [0] ; src ++)
				{	if (isspace (src [0]) && isspace (dest [-1]))
						continue ;
					dest [0] = src [0] ;
					dest ++ ;
					} ;
				dest [0] = 0 ;
				break ;
				} ;
			} ;

	if (is_pipe)
		pclose (file) ;
	else
		fclose (file) ;

	return name ;
} /* get_cpu_name */

