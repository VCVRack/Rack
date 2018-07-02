/*
** Copyright (c) 1999-2016, Erik de Castro Lopo <erikd@mega-nerd.com>
** All rights reserved.
**
** This code is released under 2-clause BSD license. Please see the
** file at : https://github.com/erikd/libsamplerate/blob/master/COPYING
*/

typedef	struct AUDIO_OUT_s AUDIO_OUT ;

typedef int (*get_audio_callback_t) (void *callback_data, float *samples, int frames) ;

/* A general audio output function (Linux/ALSA, Linux/OSS, Win32, MacOSX,
** Solaris) which retrieves data using the callback function in the above
** struct.
**
** audio_open - opens the device and returns an anonymous pointer to its
**              own private data.
*/

AUDIO_OUT *audio_open (int channels, int samplerate) ;

void audio_play (get_audio_callback_t callback, AUDIO_OUT *audio_out, void *callback_data) ;

void audio_close (AUDIO_OUT *audio_data) ;
