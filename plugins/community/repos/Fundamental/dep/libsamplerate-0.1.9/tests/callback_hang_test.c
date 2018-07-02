/*
** Copyright (c) 2002-2016, Erik de Castro Lopo <erikd@mega-nerd.com>
** All rights reserved.
**
** This code is released under 2-clause BSD license. Please see the
** file at : https://github.com/erikd/libsamplerate/blob/master/COPYING
*/

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#if HAVE_ALARM && HAVE_SIGNAL && HAVE_SIGALRM

#include <signal.h>

#include <samplerate.h>

#include "util.h"

#define	SHORT_BUFFER_LEN	512
#define	LONG_BUFFER_LEN		(1 << 14)

typedef struct
{	double ratio ;
	int count ;
} SRC_PAIR ;

static void callback_hang_test (int converter) ;

static void alarm_handler (int number) ;
static long input_callback (void *cb_data, float **data) ;


int
main (void)
{
	/* Set up SIGALRM handler. */
	signal (SIGALRM, alarm_handler) ;

	puts ("") ;
	callback_hang_test (SRC_ZERO_ORDER_HOLD) ;
	callback_hang_test (SRC_LINEAR) ;
	callback_hang_test (SRC_SINC_FASTEST) ;
	puts ("") ;

	return 0 ;
} /* main */


static void
callback_hang_test (int converter)
{	static float output [LONG_BUFFER_LEN] ;
	static SRC_PAIR pairs [] =
	{
		{ 1.2, 5 }, { 1.1, 1 }, { 1.0, 1 }, { 3.0, 1 }, { 2.0, 1 }, { 0.3, 1 },
		{ 1.2, 0 }, { 1.1, 10 }, { 1.0, 1 }
		} ;


	SRC_STATE	*src_state ;

	double src_ratio = 1.0 ;
	int k, error ;

	printf ("\tcallback_hang_test  (%-28s) ....... ", src_get_name (converter)) ;
	fflush (stdout) ;

	/* Perform sample rate conversion. */
	src_state = src_callback_new (input_callback, converter, 1, &error, NULL) ;
	if (src_state == NULL)
	{	printf ("\n\nLine %d : src_callback_new () failed : %s\n\n", __LINE__, src_strerror (error)) ;
		exit (1) ;
		} ;

	for (k = 0 ; k < ARRAY_LEN (pairs) ; k++)
	{	alarm (1) ;
		src_ratio = pairs [k].ratio ;
		src_callback_read (src_state, src_ratio, pairs [k].count, output) ;
		} ;

	src_state = src_delete (src_state) ;

	alarm (0) ;
	puts ("ok") ;

	return ;
} /* callback_hang_test */

static void
alarm_handler (int number)
{
	(void) number ;
	printf ("\n\n    Error : Hang inside src_callback_read() detected. Exiting!\n\n") ;
	exit (1) ;
} /* alarm_handler */

static long
input_callback (void *cb_data, float **data)
{
	static float buffer [20] ;

	(void) cb_data ;
	*data = buffer ;

	return ARRAY_LEN (buffer) ;
} /* input_callback */

#else

int
main (void)
{
	puts ("\tCan't run this test on this platform.") ;
	return 0 ;
} /* main */

#endif
