/*
** Copyright (c) 2002-2016, Erik de Castro Lopo <erikd@mega-nerd.com>
** All rights reserved.
**
** This code is released under 2-clause BSD license. Please see the
** file at : https://github.com/erikd/libsamplerate/blob/master/COPYING
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "config.h"

#include <float_cast.h>

#if (HAVE_SNDFILE)

#include <samplerate.h>
#include <sndfile.h>

#include "audio_out.h"

#define ARRAY_LEN(x)	((int) (sizeof (x) / sizeof ((x) [0])))

#define	BUFFER_LEN			4096
#define	VARISPEED_BLOCK_LEN	64

#define	MIN(a,b)		((a) < (b) ? (a) : (b))

#define	SRC_MAGIC		((int) ('S' << 16) + ('R' << 8) + ('C'))
#define	SNDFILE_MAGIC	((int) ('s' << 24) + ('n' << 20) + ('d' << 16) + ('f' << 12) + ('i' << 8) + ('l' << 4) + 'e')

#ifndef	M_PI
#define	M_PI			3.14159265358979323846264338
#endif


typedef struct
{	int			magic ;
	SNDFILE 	*sndfile ;
	SF_INFO 	sfinfo ;

	float		buffer	[BUFFER_LEN] ;
} SNDFILE_CB_DATA ;

typedef struct
{	int			magic ;

	SNDFILE_CB_DATA	sf ;

	int			freq_point ;

	SRC_STATE	*src_state ;

} SRC_CB_DATA ;

static int varispeed_get_data (SRC_CB_DATA *data, float *samples, int frames) ;
static void varispeed_play (const char *filename, int converter) ;

static long src_input_callback (void *cb_data, float **data) ;

int
main (int argc, char *argv [])
{	const char	*cptr, *progname, *filename ;
	int			k, converter ;

	converter = SRC_SINC_FASTEST ;

	progname = argv [0] ;

	if ((cptr = strrchr (progname, '/')) != NULL)
		progname = cptr + 1 ;

	if ((cptr = strrchr (progname, '\\')) != NULL)
		progname = cptr + 1 ;

	printf ("\n"
		"  %s\n"
		"\n"
		"  This is a demo program which plays the given file at a slowly \n"
		"  varying speed. Lots of fun with drum loops and full mixes.\n"
		"\n"
		"  It uses Secret Rabbit Code (aka libsamplerate) to perform the \n"
		"  vari-speeding and libsndfile for file I/O.\n"
		"\n", progname) ;

	if (argc == 2)
		filename = argv [1] ;
	else if (argc == 4 && strcmp (argv [1], "-c") == 0)
	{	filename = argv [3] ;
		converter = atoi (argv [2]) ;
		}
	else
	{	printf ("  Usage :\n\n       %s [-c <number>] <input file>\n\n", progname) ;
		puts (
			"  The optional -c argument allows the converter type to be chosen from\n"
			"  the following list :"
			"\n"
			) ;

		for (k = 0 ; (cptr = src_get_name (k)) != NULL ; k++)
			printf ("       %d : %s\n", k, cptr) ;

		puts ("") ;
		exit (1) ;
		} ;

	varispeed_play (filename, converter) ;

	return 0 ;
} /* main */

/*==============================================================================
*/

static void
varispeed_play (const char *filename, int converter)
{	SRC_CB_DATA		data ;
	AUDIO_OUT		*audio_out ;
	int				error ;

	memset (&data, 0, sizeof (data)) ;

	data.magic = SRC_MAGIC ;
	data.sf.magic = SNDFILE_MAGIC ;

	if ((data.sf.sndfile = sf_open (filename, SFM_READ, &data.sf.sfinfo)) == NULL)
	{	puts (sf_strerror (NULL)) ;
		exit (1) ;
		} ;

	/* Initialize the sample rate converter. */
	if ((data.src_state = src_callback_new (src_input_callback, converter, data.sf.sfinfo.channels, &error, &data.sf)) == NULL)
	{	printf ("\n\nError : src_new() failed : %s.\n\n", src_strerror (error)) ;
		exit (1) ;
		} ;

	printf (

		"  Playing   : %s\n"
		"  Converter : %s\n"
		"\n"
		"  Press <control-c> to exit.\n"
		"\n",
		filename, src_get_name (converter)) ;

	if ((audio_out = audio_open (data.sf.sfinfo.channels, data.sf.sfinfo.samplerate)) == NULL)
	{	printf ("\n\nError : audio_open () failed.\n") ;
		exit (1) ;
		} ;

	/* Pass the data and the callbacl function to audio_play */
	audio_play ((get_audio_callback_t) varispeed_get_data, audio_out, &data) ;

	/* Cleanup */
	audio_close (audio_out) ;
	sf_close (data.sf.sndfile) ;
	src_delete (data.src_state) ;

} /* varispeed_play */

static long
src_input_callback (void *cb_data, float **audio)
{	SNDFILE_CB_DATA * data = (SNDFILE_CB_DATA *) cb_data ;
	const int input_frames = ARRAY_LEN (data->buffer) / data->sfinfo.channels ;
	int		read_frames ;

	if (data->magic != SNDFILE_MAGIC)
	{	printf ("\n\n%s:%d Eeeek, something really bad happened!\n", __FILE__, __LINE__) ;
		exit (1) ;
		} ;

	for (read_frames = 0 ; read_frames < input_frames ; )
	{	sf_count_t position ;

		read_frames += sf_readf_float (data->sndfile, data->buffer + read_frames * data->sfinfo.channels, input_frames - read_frames) ;

		position = sf_seek (data->sndfile, 0, SEEK_CUR) ;

		if (position < 0 || position == data->sfinfo.frames)
			sf_seek (data->sndfile, 0, SEEK_SET) ;
		} ;

	*audio = & (data->buffer [0]) ;

	return input_frames ;
} /* src_input_callback */


/*==============================================================================
*/

static int
varispeed_get_data (SRC_CB_DATA *data, float *samples, int out_frames)
{	float	*output ;
	int		rc, out_frame_count ;

	if (data->magic != SRC_MAGIC)
	{	printf ("\n\n%s:%d Eeeek, something really bad happened!\n", __FILE__, __LINE__) ;
		exit (1) ;
		} ;

	for (out_frame_count = 0 ; out_frame_count < out_frames ; out_frame_count += VARISPEED_BLOCK_LEN)
	{	double	src_ratio = 1.0 - 0.5 * sin (data->freq_point * 2 * M_PI / 20000) ;

		data->freq_point ++ ;

		output = samples + out_frame_count * data->sf.sfinfo.channels ;

		if ((rc = src_callback_read (data->src_state, src_ratio, VARISPEED_BLOCK_LEN, output)) < VARISPEED_BLOCK_LEN)
		{	printf ("\nError : src_callback_read short output (%d instead of %d)\n\n", rc, VARISPEED_BLOCK_LEN) ;
			exit (1) ;
			} ;
		} ;

	return out_frames ;
} /* varispeed_get_data */

/*==============================================================================
*/

#else /* (HAVE_SNFILE == 0) */

/* Alternative main function when libsndfile is not available. */

int
main (void)
{	puts (
		"\n"
		"****************************************************************\n"
		" This example program was compiled without libsndfile \n"
		" (http://www.zip.com.au/~erikd/libsndfile/).\n"
		" It is therefore completely broken and non-functional.\n"
		"****************************************************************\n"
		"\n"
		) ;

	return 0 ;
} /* main */

#endif

