/*
** Copyright (c) 1999-2016, Erik de Castro Lopo <erikd@mega-nerd.com>
** All rights reserved.
**
** This code is released under 2-clause BSD license. Please see the
** file at : https://github.com/erikd/libsamplerate/blob/master/COPYING
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <config.h>

#include "audio_out.h"

#if HAVE_ALSA_ASOUNDLIB_H
	#define ALSA_PCM_NEW_HW_PARAMS_API
	#define ALSA_PCM_NEW_SW_PARAMS_API
	#include <alsa/asoundlib.h>
	#include <sys/time.h>
#endif

#if (HAVE_SNDFILE)

#include <float_cast.h>

#include <sndfile.h>

#define	BUFFER_LEN		(2048)

#define MAKE_MAGIC(a,b,c,d,e,f,g,h)		\
			((a) + ((b) << 1) + ((c) << 2) + ((d) << 3) + ((e) << 4) + ((f) << 5) + ((g) << 6) + ((h) << 7))

typedef	struct AUDIO_OUT_s
{	int magic ;
} AUDIO_OUT ;


/*------------------------------------------------------------------------------
**	Linux (ALSA and OSS) functions for playing a sound.
*/

#if defined (__linux__)

#if HAVE_ALSA_ASOUNDLIB_H

#define	ALSA_MAGIC		MAKE_MAGIC ('L', 'n', 'x', '-', 'A', 'L', 'S', 'A')

typedef struct
{	int magic ;
	snd_pcm_t * dev ;
	int channels ;
} ALSA_AUDIO_OUT ;

static int alsa_write_float (snd_pcm_t *alsa_dev, float *data, int frames, int channels) ;

static AUDIO_OUT *
alsa_open (int channels, unsigned samplerate)
{	ALSA_AUDIO_OUT *alsa_out ;
	const char * device = "default" ;
	snd_pcm_hw_params_t *hw_params ;
	snd_pcm_uframes_t buffer_size ;
	snd_pcm_uframes_t alsa_period_size, alsa_buffer_frames ;
	snd_pcm_sw_params_t *sw_params ;

	int err ;

	alsa_period_size = 1024 ;
	alsa_buffer_frames = 4 * alsa_period_size ;

	if ((alsa_out = calloc (1, sizeof (ALSA_AUDIO_OUT))) == NULL)
	{	perror ("alsa_open : malloc ") ;
		exit (1) ;
		} ;

	alsa_out->magic	= ALSA_MAGIC ;
	alsa_out->channels = channels ;

	if ((err = snd_pcm_open (&alsa_out->dev, device, SND_PCM_STREAM_PLAYBACK, 0)) < 0)
	{	fprintf (stderr, "cannot open audio device \"%s\" (%s)\n", device, snd_strerror (err)) ;
		goto catch_error ;
		} ;

	snd_pcm_nonblock (alsa_out->dev, 0) ;

	if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0)
	{	fprintf (stderr, "cannot allocate hardware parameter structure (%s)\n", snd_strerror (err)) ;
		goto catch_error ;
		} ;

	if ((err = snd_pcm_hw_params_any (alsa_out->dev, hw_params)) < 0)
	{	fprintf (stderr, "cannot initialize hardware parameter structure (%s)\n", snd_strerror (err)) ;
		goto catch_error ;
		} ;

	if ((err = snd_pcm_hw_params_set_access (alsa_out->dev, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0)
	{	fprintf (stderr, "cannot set access type (%s)\n", snd_strerror (err)) ;
		goto catch_error ;
		} ;

	if ((err = snd_pcm_hw_params_set_format (alsa_out->dev, hw_params, SND_PCM_FORMAT_FLOAT)) < 0)
	{	fprintf (stderr, "cannot set sample format (%s)\n", snd_strerror (err)) ;
		goto catch_error ;
		} ;

	if ((err = snd_pcm_hw_params_set_rate_near (alsa_out->dev, hw_params, &samplerate, 0)) < 0)
	{	fprintf (stderr, "cannot set sample rate (%s)\n", snd_strerror (err)) ;
		goto catch_error ;
		} ;

	if ((err = snd_pcm_hw_params_set_channels (alsa_out->dev, hw_params, channels)) < 0)
	{	fprintf (stderr, "cannot set channel count (%s)\n", snd_strerror (err)) ;
		goto catch_error ;
		} ;

	if ((err = snd_pcm_hw_params_set_buffer_size_near (alsa_out->dev, hw_params, &alsa_buffer_frames)) < 0)
	{	fprintf (stderr, "cannot set buffer size (%s)\n", snd_strerror (err)) ;
		goto catch_error ;
		} ;

	if ((err = snd_pcm_hw_params_set_period_size_near (alsa_out->dev, hw_params, &alsa_period_size, 0)) < 0)
	{	fprintf (stderr, "cannot set period size (%s)\n", snd_strerror (err)) ;
		goto catch_error ;
		} ;

	if ((err = snd_pcm_hw_params (alsa_out->dev, hw_params)) < 0)
	{	fprintf (stderr, "cannot set parameters (%s)\n", snd_strerror (err)) ;
		goto catch_error ;
		} ;

	/* extra check: if we have only one period, this code won't work */
	snd_pcm_hw_params_get_period_size (hw_params, &alsa_period_size, 0) ;
	snd_pcm_hw_params_get_buffer_size (hw_params, &buffer_size) ;
	if (alsa_period_size == buffer_size)
	{	fprintf (stderr, "Can't use period equal to buffer size (%lu == %lu)", alsa_period_size, buffer_size) ;
		goto catch_error ;
		} ;

	snd_pcm_hw_params_free (hw_params) ;

	if ((err = snd_pcm_sw_params_malloc (&sw_params)) != 0)
	{	fprintf (stderr, "%s: snd_pcm_sw_params_malloc: %s", __func__, snd_strerror (err)) ;
		goto catch_error ;
		} ;

	if ((err = snd_pcm_sw_params_current (alsa_out->dev, sw_params)) != 0)
	{	fprintf (stderr, "%s: snd_pcm_sw_params_current: %s", __func__, snd_strerror (err)) ;
		goto catch_error ;
		} ;

	/* note: set start threshold to delay start until the ring buffer is full */
	snd_pcm_sw_params_current (alsa_out->dev, sw_params) ;

	if ((err = snd_pcm_sw_params_set_start_threshold (alsa_out->dev, sw_params, buffer_size)) < 0)
	{	fprintf (stderr, "cannot set start threshold (%s)\n", snd_strerror (err)) ;
		goto catch_error ;
		} ;

	if ((err = snd_pcm_sw_params (alsa_out->dev, sw_params)) != 0)
	{	fprintf (stderr, "%s: snd_pcm_sw_params: %s", __func__, snd_strerror (err)) ;
		goto catch_error ;
		} ;

	snd_pcm_sw_params_free (sw_params) ;

	snd_pcm_reset (alsa_out->dev) ;

catch_error :

	if (err < 0 && alsa_out->dev != NULL)
	{	snd_pcm_close (alsa_out->dev) ;
		return NULL ;
		} ;

	return (AUDIO_OUT *) alsa_out ;
} /* alsa_open */

static void
alsa_play (get_audio_callback_t callback, AUDIO_OUT *audio_out, void *callback_data)
{	static float buffer [BUFFER_LEN] ;
	ALSA_AUDIO_OUT *alsa_out ;
	int	read_frames ;

	if ((alsa_out = (ALSA_AUDIO_OUT*) audio_out) == NULL)
	{	printf ("alsa_close : AUDIO_OUT is NULL.\n") ;
		return ;
		} ;

	if (alsa_out->magic != ALSA_MAGIC)
	{	printf ("alsa_close : Bad magic number.\n") ;
		return ;
		} ;

	while ((read_frames = callback (callback_data, buffer, BUFFER_LEN / alsa_out->channels)))
		alsa_write_float (alsa_out->dev, buffer, read_frames, alsa_out->channels) ;

	return ;
} /* alsa_play */

static int
alsa_write_float (snd_pcm_t *alsa_dev, float *data, int frames, int channels)
{	static	int epipe_count = 0 ;

	int total = 0 ;
	int retval ;

	if (epipe_count > 0)
		epipe_count -- ;

	while (total < frames)
	{	retval = snd_pcm_writei (alsa_dev, data + total * channels, frames - total) ;

		if (retval >= 0)
		{	total += retval ;
			if (total == frames)
				return total ;

			continue ;
			} ;

		switch (retval)
		{	case -EAGAIN :
					puts ("alsa_write_float: EAGAIN") ;
					continue ;
					break ;

			case -EPIPE :
					if (epipe_count > 0)
					{	printf ("alsa_write_float: EPIPE %d\n", epipe_count) ;
						if (epipe_count > 140)
							return retval ;
						} ;
					epipe_count += 100 ;

#if 0
					if (0)
					{	snd_pcm_status_t *status ;

						snd_pcm_status_alloca (&status) ;
						if ((retval = snd_pcm_status (alsa_dev, status)) < 0)
							fprintf (stderr, "alsa_out: xrun. can't determine length\n") ;
						else if (snd_pcm_status_get_state (status) == SND_PCM_STATE_XRUN)
						{	struct timeval now, diff, tstamp ;

							gettimeofday (&now, 0) ;
							snd_pcm_status_get_trigger_tstamp (status, &tstamp) ;
							timersub (&now, &tstamp, &diff) ;

							fprintf (stderr, "alsa_write_float xrun: of at least %.3f msecs. resetting stream\n",
									diff.tv_sec * 1000 + diff.tv_usec / 1000.0) ;
							}
						else
							fprintf (stderr, "alsa_write_float: xrun. can't determine length\n") ;
						} ;
#endif

					snd_pcm_prepare (alsa_dev) ;
					break ;

			case -EBADFD :
					fprintf (stderr, "alsa_write_float: Bad PCM state.n") ;
					return 0 ;
					break ;

			case -ESTRPIPE :
					fprintf (stderr, "alsa_write_float: Suspend event.n") ;
					return 0 ;
					break ;

			case -EIO :
					puts ("alsa_write_float: EIO") ;
					return 0 ;

			default :
					fprintf (stderr, "alsa_write_float: retval = %d\n", retval) ;
					return 0 ;
					break ;
			} ; /* switch */
		} ; /* while */

	return total ;
} /* alsa_write_float */

static void
alsa_close (AUDIO_OUT *audio_out)
{	ALSA_AUDIO_OUT *alsa_out ;

	if ((alsa_out = (ALSA_AUDIO_OUT*) audio_out) == NULL)
	{	printf ("alsa_close : AUDIO_OUT is NULL.\n") ;
		return ;
		} ;

	if (alsa_out->magic != ALSA_MAGIC)
	{	printf ("alsa_close : Bad magic number.\n") ;
		return ;
		} ;

	memset (alsa_out, 0, sizeof (ALSA_AUDIO_OUT)) ;

	free (alsa_out) ;

	return ;
} /* alsa_close */

#endif /* HAVE_ALSA_ASOUNDLIB_H */

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>

#define	OSS_MAGIC		MAKE_MAGIC ('L', 'i', 'n', 'u', 'x', 'O', 'S', 'S')

typedef struct
{	int magic ;
	int fd ;
	int channels ;
} OSS_AUDIO_OUT ;

static AUDIO_OUT *opensoundsys_open (int channels, int samplerate) ;
static void opensoundsys_play (get_audio_callback_t callback, AUDIO_OUT *audio_out, void *callback_data) ;
static void opensoundsys_close (AUDIO_OUT *audio_out) ;


static AUDIO_OUT *
opensoundsys_open (int channels, int samplerate)
{	OSS_AUDIO_OUT	*opensoundsys_out ;
	int stereo, fmt, error ;

	if ((opensoundsys_out = calloc (1, sizeof (OSS_AUDIO_OUT))) == NULL)
	{	perror ("opensoundsys_open : malloc ") ;
		exit (1) ;
		} ;

	opensoundsys_out->magic	= OSS_MAGIC ;
	opensoundsys_out->channels = channels ;

	if ((opensoundsys_out->fd = open ("/dev/dsp", O_WRONLY, 0)) == -1)
	{	perror ("opensoundsys_open : open ") ;
		exit (1) ;
		} ;

	stereo = 0 ;
	if (ioctl (opensoundsys_out->fd, SNDCTL_DSP_STEREO, &stereo) == -1)
	{ 	/* Fatal error */
		perror ("opensoundsys_open : stereo ") ;
		exit (1) ;
		} ;

	if (ioctl (opensoundsys_out->fd, SNDCTL_DSP_RESET, 0))
	{	perror ("opensoundsys_open : reset ") ;
		exit (1) ;
		} ;

	fmt = CPU_IS_BIG_ENDIAN ? AFMT_S16_BE : AFMT_S16_LE ;
	if (ioctl (opensoundsys_out->fd, SNDCTL_DSP_SETFMT, &fmt) != 0)
	{	perror ("opensoundsys_open_dsp_device : set format ") ;
	    exit (1) ;
  		} ;

	if ((error = ioctl (opensoundsys_out->fd, SNDCTL_DSP_CHANNELS, &channels)) != 0)
	{	perror ("opensoundsys_open : channels ") ;
		exit (1) ;
		} ;

	if ((error = ioctl (opensoundsys_out->fd, SNDCTL_DSP_SPEED, &samplerate)) != 0)
	{	perror ("opensoundsys_open : sample rate ") ;
		exit (1) ;
		} ;

	if ((error = ioctl (opensoundsys_out->fd, SNDCTL_DSP_SYNC, 0)) != 0)
	{	perror ("opensoundsys_open : sync ") ;
		exit (1) ;
		} ;

	return 	(AUDIO_OUT*) opensoundsys_out ;
} /* opensoundsys_open */

static void
opensoundsys_play (get_audio_callback_t callback, AUDIO_OUT *audio_out, void *callback_data)
{	OSS_AUDIO_OUT *opensoundsys_out ;
	static float float_buffer [BUFFER_LEN] ;
	static short buffer [BUFFER_LEN] ;
	int		k, read_frames ;

	if ((opensoundsys_out = (OSS_AUDIO_OUT*) audio_out) == NULL)
	{	printf ("opensoundsys_play : AUDIO_OUT is NULL.\n") ;
		return ;
		} ;

	if (opensoundsys_out->magic != OSS_MAGIC)
	{	printf ("opensoundsys_play : Bad magic number.\n") ;
		return ;
		} ;

	while ((read_frames = callback (callback_data, float_buffer, BUFFER_LEN / opensoundsys_out->channels)))
	{	for (k = 0 ; k < read_frames * opensoundsys_out->channels ; k++)
			buffer [k] = lrint (32767.0 * float_buffer [k]) ;
		(void) write (opensoundsys_out->fd, buffer, read_frames * opensoundsys_out->channels * sizeof (short)) ;
		} ;

	return ;
} /* opensoundsys_play */

static void
opensoundsys_close (AUDIO_OUT *audio_out)
{	OSS_AUDIO_OUT *opensoundsys_out ;

	if ((opensoundsys_out = (OSS_AUDIO_OUT*) audio_out) == NULL)
	{	printf ("opensoundsys_close : AUDIO_OUT is NULL.\n") ;
		return ;
		} ;

	if (opensoundsys_out->magic != OSS_MAGIC)
	{	printf ("opensoundsys_close : Bad magic number.\n") ;
		return ;
		} ;

	memset (opensoundsys_out, 0, sizeof (OSS_AUDIO_OUT)) ;

	free (opensoundsys_out) ;

	return ;
} /* opensoundsys_close */

#endif /* __linux__ */

/*------------------------------------------------------------------------------
**	Mac OS X functions for playing a sound.
*/

#if (defined (__MACH__) && defined (__APPLE__)) /* MacOSX */

#include <Carbon.h>
#include <CoreAudio/AudioHardware.h>

#define	MACOSX_MAGIC	MAKE_MAGIC ('M', 'a', 'c', ' ', 'O', 'S', ' ', 'X')

typedef struct
{	int magic ;
	AudioStreamBasicDescription	format ;

	UInt32 			buf_size ;
	AudioDeviceID 	device ;

	int		channels ;
	int 	samplerate ;
	int		buffer_size ;
	int		done_playing ;

	get_audio_callback_t	callback ;

	void 	*callback_data ;
} MACOSX_AUDIO_OUT ;

static AUDIO_OUT *macosx_open (int channels, int samplerate) ;
static void macosx_play (get_audio_callback_t callback, AUDIO_OUT *audio_out, void *callback_data) ;
static void macosx_close (AUDIO_OUT *audio_out) ;

static OSStatus
macosx_audio_out_callback (AudioDeviceID device, const AudioTimeStamp* current_time,
	const AudioBufferList* data_in, const AudioTimeStamp* time_in,
	AudioBufferList* data_out, const AudioTimeStamp* time_out, void* client_data) ;


static AUDIO_OUT *
macosx_open (int channels, int samplerate)
{	MACOSX_AUDIO_OUT *macosx_out ;
	OSStatus	err ;
	size_t 		count ;

	if ((macosx_out = calloc (1, sizeof (MACOSX_AUDIO_OUT))) == NULL)
	{	perror ("macosx_open : malloc ") ;
		exit (1) ;
		} ;

	macosx_out->magic = MACOSX_MAGIC ;
	macosx_out->channels = channels ;
	macosx_out->samplerate = samplerate ;

	macosx_out->device = kAudioDeviceUnknown ;

	/*  get the default output device for the HAL */
	count = sizeof (AudioDeviceID) ;
	if ((err = AudioHardwareGetProperty (kAudioHardwarePropertyDefaultOutputDevice,
				&count, (void *) &(macosx_out->device))) != noErr)
	{	printf ("AudioHardwareGetProperty failed.\n") ;
		free (macosx_out) ;
		return NULL ;
		} ;

	/*  get the buffersize that the default device uses for IO */
	count = sizeof (UInt32) ;
	if ((err = AudioDeviceGetProperty (macosx_out->device, 0, false, kAudioDevicePropertyBufferSize,
				&count, &(macosx_out->buffer_size))) != noErr)
	{	printf ("AudioDeviceGetProperty (AudioDeviceGetProperty) failed.\n") ;
		free (macosx_out) ;
		return NULL ;
		} ;

	/*  get a description of the data format used by the default device */
	count = sizeof (AudioStreamBasicDescription) ;
	if ((err = AudioDeviceGetProperty (macosx_out->device, 0, false, kAudioDevicePropertyStreamFormat,
				&count, &(macosx_out->format))) != noErr)
	{	printf ("AudioDeviceGetProperty (kAudioDevicePropertyStreamFormat) failed.\n") ;
		free (macosx_out) ;
		return NULL ;
		} ;

	macosx_out->format.mSampleRate = samplerate ;
	macosx_out->format.mChannelsPerFrame = channels ;

	if ((err = AudioDeviceSetProperty (macosx_out->device, NULL, 0, false, kAudioDevicePropertyStreamFormat,
				sizeof (AudioStreamBasicDescription), &(macosx_out->format))) != noErr)
	{	printf ("AudioDeviceSetProperty (kAudioDevicePropertyStreamFormat) failed.\n") ;
		free (macosx_out) ;
		return NULL ;
		} ;

	/*  we want linear pcm */
	if (macosx_out->format.mFormatID != kAudioFormatLinearPCM)
	{	free (macosx_out) ;
		return NULL ;
		} ;

	macosx_out->done_playing = 0 ;

	/* Fire off the device. */
	if ((err = AudioDeviceAddIOProc (macosx_out->device, macosx_audio_out_callback,
			(void *) macosx_out)) != noErr)
	{	printf ("AudioDeviceAddIOProc failed.\n") ;
		free (macosx_out) ;
		return NULL ;
		} ;

	return (MACOSX_AUDIO_OUT *) macosx_out ;
} /* macosx_open */

static void
macosx_play (get_audio_callback_t callback, AUDIO_OUT *audio_out, void *callback_data)
{	MACOSX_AUDIO_OUT	*macosx_out ;
	OSStatus	err ;

	if ((macosx_out = (MACOSX_AUDIO_OUT*) audio_out) == NULL)
	{	printf ("macosx_play : AUDIO_OUT is NULL.\n") ;
		return ;
		} ;

	if (macosx_out->magic != MACOSX_MAGIC)
	{	printf ("macosx_play : Bad magic number.\n") ;
		return ;
		} ;

	/* Set the callback function and callback data. */
	macosx_out->callback = callback ;
	macosx_out->callback_data = callback_data ;

	err = AudioDeviceStart (macosx_out->device, macosx_audio_out_callback) ;
	if (err != noErr)
		printf ("AudioDeviceStart failed.\n") ;

	while (macosx_out->done_playing == SF_FALSE)
		usleep (10 * 1000) ; /* 10 000 milliseconds. */

	return ;
} /* macosx_play */

static void
macosx_close (AUDIO_OUT *audio_out)
{	MACOSX_AUDIO_OUT	*macosx_out ;
	OSStatus	err ;

	if ((macosx_out = (MACOSX_AUDIO_OUT*) audio_out) == NULL)
	{	printf ("macosx_close : AUDIO_OUT is NULL.\n") ;
		return ;
		} ;

	if (macosx_out->magic != MACOSX_MAGIC)
	{	printf ("macosx_close : Bad magic number.\n") ;
		return ;
		} ;


	if ((err = AudioDeviceStop (macosx_out->device, macosx_audio_out_callback)) != noErr)
	{	printf ("AudioDeviceStop failed.\n") ;
		return ;
		} ;

	err = AudioDeviceRemoveIOProc (macosx_out->device, macosx_audio_out_callback) ;
	if (err != noErr)
	{	printf ("AudioDeviceRemoveIOProc failed.\n") ;
		return ;
		} ;

} /* macosx_close */

static OSStatus
macosx_audio_out_callback (AudioDeviceID device, const AudioTimeStamp* current_time,
	const AudioBufferList* data_in, const AudioTimeStamp* time_in,
	AudioBufferList* data_out, const AudioTimeStamp* time_out, void* client_data)
{	MACOSX_AUDIO_OUT	*macosx_out ;
	int		k, size, frame_count, read_count ;
	float	*buffer ;

	if ((macosx_out = (MACOSX_AUDIO_OUT*) client_data) == NULL)
	{	printf ("macosx_play : AUDIO_OUT is NULL.\n") ;
		return 42 ;
		} ;

	if (macosx_out->magic != MACOSX_MAGIC)
	{	printf ("macosx_play : Bad magic number.\n") ;
		return 42 ;
		} ;

	size = data_out->mBuffers [0].mDataByteSize ;
	frame_count = size / sizeof (float) / macosx_out->channels ;

	buffer = (float*) data_out->mBuffers [0].mData ;

	read_count = macosx_out->callback (macosx_out->callback_data, buffer, frame_count) ;

	if (read_count < frame_count)
	{	memset (&(buffer [read_count]), 0, (frame_count - read_count) * sizeof (float)) ;
		macosx_out->done_playing = 1 ;
		} ;

	return noErr ;
} /* macosx_audio_out_callback */

#endif /* MacOSX */


/*------------------------------------------------------------------------------
**	Win32 functions for playing a sound.
**
**	This API sucks. Its needlessly complicated and is *WAY* too loose with
**	passing pointers arounf in integers and and using char* pointers to
**  point to data instead of short*. It plain sucks!
*/

#if (defined (_WIN32) || defined (WIN32))

#include <windows.h>
#include <mmsystem.h>

#define	WIN32_BUFFER_LEN	(1<<15)
#define	WIN32_MAGIC			MAKE_MAGIC ('W', 'i', 'n', '3', '2', 's', 'u', 'x')

typedef struct
{	int 			magic ;

	HWAVEOUT		hwave ;
	WAVEHDR			whdr [2] ;

	HANDLE			Event ;

	short			short_buffer [WIN32_BUFFER_LEN / sizeof (short)] ;
	float			float_buffer [WIN32_BUFFER_LEN / sizeof (short) / 2] ;

	int				bufferlen, current ;

	int				channels ;

	get_audio_callback_t	callback ;

	void 			*callback_data ;
} WIN32_AUDIO_OUT ;

static AUDIO_OUT *win32_open (int channels, int samplerate) ;
static void win32_play (get_audio_callback_t callback, AUDIO_OUT *audio_out, void *callback_data) ;
static void win32_close (AUDIO_OUT *audio_out) ;

static DWORD CALLBACK
	win32_audio_out_callback (HWAVEOUT hwave, UINT msg, DWORD data, DWORD param1, DWORD param2) ;

static AUDIO_OUT*
win32_open (int channels, int samplerate)
{	WIN32_AUDIO_OUT *win32_out ;

	WAVEFORMATEX wf ;
	int error ;

	if ((win32_out = calloc (1, sizeof (WIN32_AUDIO_OUT))) == NULL)
	{	perror ("win32_open : malloc ") ;
		exit (1) ;
		} ;

	win32_out->magic	= WIN32_MAGIC ;
	win32_out->channels = channels ;

	win32_out->current = 0 ;

	win32_out->Event = CreateEvent (0, FALSE, FALSE, 0) ;

	wf.nChannels = channels ;
	wf.nSamplesPerSec = samplerate ;
	wf.nBlockAlign = channels * sizeof (short) ;

	wf.wFormatTag = WAVE_FORMAT_PCM ;
	wf.cbSize = 0 ;
	wf.wBitsPerSample = 16 ;
	wf.nAvgBytesPerSec = wf.nBlockAlign * wf.nSamplesPerSec ;

	error = waveOutOpen (&(win32_out->hwave), WAVE_MAPPER, &wf, (DWORD) win32_audio_out_callback,
							(DWORD) win32_out, CALLBACK_FUNCTION) ;
	if (error)
	{	puts ("waveOutOpen failed.") ;
		free (win32_out) ;
		return NULL ;
		} ;

	waveOutPause (win32_out->hwave) ;

	return (WIN32_AUDIO_OUT *) win32_out ;
} /* win32_open */

static void
win32_play (get_audio_callback_t callback, AUDIO_OUT *audio_out, void *callback_data)
{	WIN32_AUDIO_OUT	*win32_out ;
	int		error ;

	if ((win32_out = (WIN32_AUDIO_OUT*) audio_out) == NULL)
	{	printf ("win32_play : AUDIO_OUT is NULL.\n") ;
		return ;
		} ;

	if (win32_out->magic != WIN32_MAGIC)
	{	printf ("win32_play : Bad magic number (%d %d).\n", win32_out->magic, WIN32_MAGIC) ;
		return ;
		} ;

	/* Set the callback function and callback data. */
	win32_out->callback = callback ;
	win32_out->callback_data = callback_data ;

	win32_out->whdr [0].lpData = (char*) win32_out->short_buffer ;
	win32_out->whdr [1].lpData = ((char*) win32_out->short_buffer) + sizeof (win32_out->short_buffer) / 2 ;

	win32_out->whdr [0].dwBufferLength = sizeof (win32_out->short_buffer) / 2 ;
	win32_out->whdr [1].dwBufferLength = sizeof (win32_out->short_buffer) / 2 ;

	win32_out->bufferlen = sizeof (win32_out->short_buffer) / 2 / sizeof (short) ;

	/* Prepare the WAVEHDRs */
	if ((error = waveOutPrepareHeader (win32_out->hwave, &(win32_out->whdr [0]), sizeof (WAVEHDR))))
	{	printf ("waveOutPrepareHeader [0] failed : %08X\n", error) ;
		waveOutClose (win32_out->hwave) ;
		return ;
		} ;

	if ((error = waveOutPrepareHeader (win32_out->hwave, &(win32_out->whdr [1]), sizeof (WAVEHDR))))
	{	printf ("waveOutPrepareHeader [1] failed : %08X\n", error) ;
		waveOutUnprepareHeader (win32_out->hwave, &(win32_out->whdr [0]), sizeof (WAVEHDR)) ;
		waveOutClose (win32_out->hwave) ;
		return ;
		} ;

	waveOutRestart (win32_out->hwave) ;

	/* Fake 2 calls to the callback function to queue up enough audio. */
	win32_audio_out_callback (0, MM_WOM_DONE, (DWORD) win32_out, 0, 0) ;
	win32_audio_out_callback (0, MM_WOM_DONE, (DWORD) win32_out, 0, 0) ;

	/* Wait for playback to finish. The callback notifies us when all
	** wave data has been played.
	*/
	WaitForSingleObject (win32_out->Event, INFINITE) ;

	waveOutPause (win32_out->hwave) ;
	waveOutReset (win32_out->hwave) ;

	waveOutUnprepareHeader (win32_out->hwave, &(win32_out->whdr [0]), sizeof (WAVEHDR)) ;
	waveOutUnprepareHeader (win32_out->hwave, &(win32_out->whdr [1]), sizeof (WAVEHDR)) ;

	waveOutClose (win32_out->hwave) ;
	win32_out->hwave = 0 ;

	return ;
} /* win32_play */

static void
win32_close (AUDIO_OUT *audio_out)
{	WIN32_AUDIO_OUT *win32_out ;

	if ((win32_out = (WIN32_AUDIO_OUT*) audio_out) == NULL)
	{	printf ("win32_close : AUDIO_OUT is NULL.\n") ;
		return ;
		} ;

	if (win32_out->magic != WIN32_MAGIC)
	{	printf ("win32_close : Bad magic number.\n") ;
		return ;
		} ;

	memset (win32_out, 0, sizeof (WIN32_AUDIO_OUT)) ;

	free (win32_out) ;
} /* win32_close */

static DWORD CALLBACK
win32_audio_out_callback (HWAVEOUT hwave, UINT msg, DWORD data, DWORD param1, DWORD param2)
{	WIN32_AUDIO_OUT	*win32_out ;
	int		read_count, frame_count, k ;
	short	*sptr ;

	/*
	** I consider this technique of passing a pointer via an integer as
	** fundamentally broken but thats the way microsoft has defined the
	** interface.
	*/
	if ((win32_out = (WIN32_AUDIO_OUT*) data) == NULL)
	{	printf ("win32_audio_out_callback : AUDIO_OUT is NULL.\n") ;
		return 1 ;
		} ;

	if (win32_out->magic != WIN32_MAGIC)
	{	printf ("win32_audio_out_callback : Bad magic number (%d %d).\n", win32_out->magic, WIN32_MAGIC) ;
		return 1 ;
		} ;

	if (msg != MM_WOM_DONE)
		return 0 ;

	/* Do the actual audio. */
	sample_count = win32_out->bufferlen ;
	frame_count = sample_count / win32_out->channels ;

	read_count = win32_out->callback (win32_out->callback_data, win32_out->float_buffer, frame_count) ;

	sptr = (short*) win32_out->whdr [win32_out->current].lpData ;

	for (k = 0 ; k < read_count ; k++)
		sptr [k] = lrint (32767.0 * win32_out->float_buffer [k]) ;

	if (read_count > 0)
	{	/* Fix buffer length is only a partial block. */
		if (read_count * sizeof (short) < win32_out->bufferlen)
			win32_out->whdr [win32_out->current].dwBufferLength = read_count * sizeof (short) ;

		/* Queue the WAVEHDR */
		waveOutWrite (win32_out->hwave, (LPWAVEHDR) &(win32_out->whdr [win32_out->current]), sizeof (WAVEHDR)) ;
		}
	else
	{	/* Stop playback */
		waveOutPause (win32_out->hwave) ;

		SetEvent (win32_out->Event) ;
		} ;

	win32_out->current = (win32_out->current + 1) % 2 ;

	return 0 ;
} /* win32_audio_out_callback */

#endif /* Win32 */

/*------------------------------------------------------------------------------
**	Solaris.
*/

#if (defined (sun) && defined (unix)) /* ie Solaris */

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/audioio.h>

#define	SOLARIS_MAGIC	MAKE_MAGIC ('S', 'o', 'l', 'a', 'r', 'i', 's', ' ')

typedef struct
{	int magic ;
	int fd ;
	int channels ;
	int samplerate ;
} SOLARIS_AUDIO_OUT ;

static AUDIO_OUT *solaris_open (int channels, int samplerate) ;
static void solaris_play (get_audio_callback_t callback, AUDIO_OUT *audio_out, void *callback_data) ;
static void solaris_close (AUDIO_OUT *audio_out) ;

static AUDIO_OUT *
solaris_open (int channels, int samplerate)
{	SOLARIS_AUDIO_OUT	*solaris_out ;
	audio_info_t		audio_info ;
	int					error ;

	if ((solaris_out = calloc (1, sizeof (SOLARIS_AUDIO_OUT))) == NULL)
	{	perror ("solaris_open : malloc ") ;
		exit (1) ;
		} ;

	solaris_out->magic		= SOLARIS_MAGIC ;
	solaris_out->channels	= channels ;
	solaris_out->samplerate	= channels ;

	/* open the audio device - write only, non-blocking */
	if ((solaris_out->fd = open ("/dev/audio", O_WRONLY | O_NONBLOCK)) < 0)
	{	perror ("open (/dev/audio) failed") ;
		exit (1) ;
		} ;

	/*	Retrive standard values. */
	AUDIO_INITINFO (&audio_info) ;

	audio_info.play.sample_rate = samplerate ;
	audio_info.play.channels = channels ;
	audio_info.play.precision = 16 ;
	audio_info.play.encoding = AUDIO_ENCODING_LINEAR ;
	audio_info.play.gain = AUDIO_MAX_GAIN ;
	audio_info.play.balance = AUDIO_MID_BALANCE ;

	if ((error = ioctl (solaris_out->fd, AUDIO_SETINFO, &audio_info)))
	{	perror ("ioctl (AUDIO_SETINFO) failed") ;
		exit (1) ;
		} ;

	return 	(AUDIO_OUT*) solaris_out ;
} /* solaris_open */

static void
solaris_play (get_audio_callback_t callback, AUDIO_OUT *audio_out, void *callback_data)
{	SOLARIS_AUDIO_OUT *solaris_out ;
	static float float_buffer [BUFFER_LEN] ;
	static short buffer [BUFFER_LEN] ;
	int		k, read_frames ;

	if ((solaris_out = (SOLARIS_AUDIO_OUT*) audio_out) == NULL)
	{	printf ("solaris_play : AUDIO_OUT is NULL.\n") ;
		return ;
		} ;

	if (solaris_out->magic != SOLARIS_MAGIC)
	{	printf ("solaris_play : Bad magic number.\n") ;
		return ;
		} ;

	while ((read_frames = callback (callback_data, float_buffer, BUFFER_LEN / solaris_out->channels)))
	{	for (k = 0 ; k < read_frames * solaris_out->channels ; k++)
			buffer [k] = lrint (32767.0 * float_buffer [k]) ;
		write (solaris_out->fd, buffer, read_frames * solaris_out->channels * sizeof (short)) ;
		} ;

	return ;
} /* solaris_play */

static void
solaris_close (AUDIO_OUT *audio_out)
{	SOLARIS_AUDIO_OUT *solaris_out ;

	if ((solaris_out = (SOLARIS_AUDIO_OUT*) audio_out) == NULL)
	{	printf ("solaris_close : AUDIO_OUT is NULL.\n") ;
		return ;
		} ;

	if (solaris_out->magic != SOLARIS_MAGIC)
	{	printf ("solaris_close : Bad magic number.\n") ;
		return ;
		} ;

	memset (solaris_out, 0, sizeof (SOLARIS_AUDIO_OUT)) ;

	free (solaris_out) ;

	return ;
} /* solaris_close */

#endif /* Solaris */

/*==============================================================================
**	Main function.
*/

AUDIO_OUT *
audio_open (int channels, int samplerate)
{
#if defined (__linux__)
	#if HAVE_ALSA_ASOUNDLIB_H
		if (access ("/proc/asound/cards", R_OK) == 0)
			return alsa_open (channels, samplerate) ;
	#endif
		return opensoundsys_open (channels, samplerate) ;
#elif (defined (__MACH__) && defined (__APPLE__))
	return macosx_open (channels, samplerate) ;
#elif (defined (sun) && defined (unix))
	return solaris_open (channels, samplerate) ;
#elif (defined (_WIN32) || defined (WIN32))
	return win32_open (channels, samplerate) ;
#else
	#warning "*** Playing sound not yet supported on this platform."
	#warning "*** Please feel free to submit a patch."
	printf ("Error : Playing sound not yet supported on this platform.\n") ;
	return NULL ;
#endif


	return NULL ;
} /* audio_open */

void
audio_play (get_audio_callback_t callback, AUDIO_OUT *audio_out, void *callback_data)
{

	if (callback == NULL)
	{	printf ("Error : bad callback pointer.\n") ;
		return ;
		} ;

	if (audio_out == NULL)
	{	printf ("Error : bad audio_out pointer.\n") ;
		return ;
		} ;

	if (callback_data == NULL)
	{	printf ("Error : bad callback_data pointer.\n") ;
		return ;
		} ;

#if defined (__linux__)
	#if HAVE_ALSA_ASOUNDLIB_H
		if (audio_out->magic == ALSA_MAGIC)
			alsa_play (callback, audio_out, callback_data) ;
	#endif
		opensoundsys_play (callback, audio_out, callback_data) ;
#elif (defined (__MACH__) && defined (__APPLE__))
	macosx_play (callback, audio_out, callback_data) ;
#elif (defined (sun) && defined (unix))
	solaris_play (callback, audio_out, callback_data) ;
#elif (defined (_WIN32) || defined (WIN32))
	win32_play (callback, audio_out, callback_data) ;
#else
	#warning "*** Playing sound not yet supported on this platform."
	#warning "*** Please feel free to submit a patch."
	printf ("Error : Playing sound not yet supported on this platform.\n") ;
	return ;
#endif

	return ;
} /* audio_play */

void
audio_close (AUDIO_OUT *audio_out)
{
#if defined (__linux__)
	#if HAVE_ALSA_ASOUNDLIB_H
		if (audio_out->magic == ALSA_MAGIC)
			alsa_close (audio_out) ;
	#endif
	opensoundsys_close (audio_out) ;
#elif (defined (__MACH__) && defined (__APPLE__))
	macosx_close (audio_out) ;
#elif (defined (sun) && defined (unix))
	solaris_close (audio_out) ;
#elif (defined (_WIN32) || defined (WIN32))
	win32_close (audio_out) ;
#else
	#warning "*** Playing sound not yet supported on this platform."
	#warning "*** Please feel free to submit a patch."
	printf ("Error : Playing sound not yet supported on this platform.\n") ;
	return ;
#endif

	return ;
} /* audio_close */

#else /* (HAVE_SNDFILE == 0) */

/* Do not have libsndfile installed so just return. */

AUDIO_OUT *
audio_open (int channels, int samplerate)
{
	(void) channels ;
	(void) samplerate ;

	return NULL ;
} /* audio_open */

void
audio_play (get_audio_callback_t callback, AUDIO_OUT *audio_out, void *callback_data)
{
	(void) callback ;
	(void) audio_out ;
	(void) callback_data ;

	return ;
} /* audio_play */

void
audio_close (AUDIO_OUT *audio_out)
{
	audio_out = audio_out ;

	return ;
} /* audio_close */

#endif /* HAVE_SNDFILE */

