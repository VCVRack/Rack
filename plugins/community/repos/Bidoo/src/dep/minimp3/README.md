minimp3
==========

[![Build Status](https://travis-ci.org/lieff/minimp3.svg)](https://travis-ci.org/lieff/minimp3)
<a href="https://scan.coverity.com/projects/lieff-minimp3">
  <img alt="Coverity Scan Build Status"
       src="https://scan.coverity.com/projects/14844/badge.svg"/>
</a>
[![codecov](https://codecov.io/gh/lieff/minimp3/branch/master/graph/badge.svg)](https://codecov.io/gh/lieff/minimp3)

Minimalistic, single-header library for decoding MP3. minimp3 is designed to be
small, fast (with SSE and NEON support), and accurate (ISO conformant). You can
find a rough benchmark below, measured using ``perf`` on an i7-6700K, IO
included, no CPU heat to address speedstep:

| Vector      | Hz    | Samples| Sec    | Clockticks | Clockticks per second | PSNR | Max diff |
| ----------- | ----- | ------ | ------ | --------- | ------ | ------ | - |
|compl.bit    | 48000 | 248832 | 5.184  | 14306684  | 2.759M | 124.22 | 1 |
|he_32khz.bit | 32000 | 172800 | 5.4    | 8426158   | 1.560M | 139.67 | 1 |
|he_44khz.bit | 44100 | 472320 | 10.710 | 21296300  | 1.988M | 144.04 | 1 |
|he_48khz.bit | 48000 | 172800 | 3.6    | 8453846   | 2.348M | 139.67 | 1 |
|hecommon.bit | 44100 | 69120  | 1.567  | 3169715   | 2.022M | 133.93 | 1 |
|he_free.bit  | 44100 | 156672 | 3.552  | 5798418   | 1.632M | 137.48 | 1 |
|he_mode.bit  | 44100 | 262656 | 5.955  | 9882314   | 1.659M | 118.00 | 1 |
|si.bit       | 44100 | 135936 | 3.082  | 7170520   | 2.326M | 120.30 | 1 |
|si_block.bit | 44100 | 73728  | 1.671  | 4233136   | 2.533M | 125.18 | 1 |
|si_huff.bit  | 44100 | 86400  | 1.959  | 4785322   | 2.442M | 107.98 | 1 |
|sin1k0db.bit | 44100 | 725760 | 16.457 | 24842977  | 1.509M | 111.03 | 1 |

Conformance test passed on all vectors (PSNR > 96db).

## Comparison with keyj's [minimp3](http://keyj.emphy.de/minimp3/)

Comparison by features:

| Keyj minimp3 | Current |
| ------------ | ------- |
| Fixed point  | Floating point |
| source: 84kb | 70kb |
| binary: 34kb (20kb compressed) | 30kb (20kb) |
| no vector opts | SSE/NEON intrinsics |
| no free format | free format support |

Below, you can find the benchmark and conformance test for keyj's minimp3: 


| Vector      | Hz    | Samples| Sec    | Clockticks | Clockticks per second | PSNR | Max diff |
| ----------- | ----- | ------ | ------ | --------- | ------  | ----- | - |
|compl.bit    | 48000 | 248832 | 5.184  | 31849373  | 6.143M  | 71.50 | 41 |
|he_32khz.bit | 32000 | 172800 | 5.4    | 26302319  | 4.870M  | 71.63 | 24 |
|he_44khz.bit | 44100 | 472320 | 10.710 | 41628861  | 3.886M  | 71.63 | 24 |
|he_48khz.bit | 48000 | 172800 | 3.6    | 25899527  | 7.194M  | 71.63 | 24 |
|hecommon.bit | 44100 | 69120  | 1.567  | 20437779  | 13.039M | 71.58 | 25 |
|he_free.bit  | 44100 | 0 | 0  | -  | - | -  | - |
|he_mode.bit  | 44100 | 262656 | 5.955  | 30988984  | 5.203M  | 71.78 | 27 |
|si.bit       | 44100 | 135936 | 3.082  | 24096223  | 7.817M  | 72.35 | 36 |
|si_block.bit | 44100 | 73728  | 1.671  | 20722017  | 12.394M | 71.84 | 26 |
|si_huff.bit  | 44100 | 86400  | 1.959  | 21121376  | 10.780M | 27.80 | 65535 |
|sin1k0db.bit | 44100 | 730368 | 16.561 | 55569636  | 3.355M  | 0.15  | 58814 |

Keyj minimp3 conformance test fails on all vectors (PSNR < 96db), and free 
format is unsupported. This caused some problems when it was used 
[here](https://github.com/lieff/lvg), and was the main motivation for this work.

## Usage

First, we need to initialize the decoder structure:

```
//#define MINIMP3_ONLY_MP3
//#define MINIMP3_ONLY_SIMD
//#define MINIMP3_NO_SIMD
//#define MINIMP3_NONSTANDARD_BUT_LOGICAL
#define MINIMP3_IMPLEMENTATION
#include "minimp3.h"
...
    static mp3dec_t mp3d;
    mp3dec_init(&mp3d);
```

Note that you must define ``MINIMP3_IMPLEMENTATION`` in exactly one source file.
You can ``#include`` ``minimp3.h`` in as many files as you like.
Also you can use ``MINIMP3_ONLY_MP3`` define to strip MP1/MP2 decoding code.
MINIMP3_ONLY_SIMD define controls generic (non SSE/NEON) code generation (always enabled on x64/arm64 targets).
In case you do not want any platform-specific SIMD optimizations, you can define ``MINIMP3_NO_SIMD``.
MINIMP3_NONSTANDARD_BUT_LOGICAL define saves some code bytes, and enforces non-stadnard but logical behaviour of mono-stereo transition (rare case).

Then. we decode the input stream frame-by-frame:

```
    /*typedef struct
    {
        int frame_bytes;
        int channels;
        int hz;
        int layer;
        int bitrate_kbps;
    } mp3dec_frame_info_t;*/
    mp3dec_frame_info_t info;
    short pcm[MINIMP3_MAX_SAMPLES_PER_FRAME];
    /*unsigned char *input_buf; - input byte stream*/
    samples = mp3dec_decode_frame(&mp3d, input_buf, buf_size, pcm, &info);
```

The ``mp3dec_decode_frame()`` function decodes one full MP3 frame from the 
input buffer, which must be large enough to hold one full frame. 

The decoder will analyze the input buffer to properly sync with the MP3 stream,
and will skip ID3 data, as well as any data which is not valid. Short buffers
may cause false sync and can produce 'squealing' artefacts. The bigger the size
of the input buffer, the more reliable the sync procedure. We recommend having
as many as 10 consecutive MP3 frames (~16KB) in the input buffer at a time.

The size of the consumed MP3 data is returned in the ``mp3dec_frame_info_t`` 
field of the ``frame_bytes`` struct; you must remove the data corresponding to 
the ``frame_bytes`` field  from the input buffer before the next decoder 
invocation.  

The decoding function returns the number of decoded samples. The following cases
are possible:

- **0:** No MP3 data was found in the input buffer  
- **384:**  Layer 1  
- **576:**  MPEG 2 Layer 3  
- **1152:** Otherwise  

The following is a description of the possible combinations of the number of
samples and ``frame_bytes`` field values:

- More than 0 samples and ``frame_bytes > 0``:  Succesful decode  
- 0 samples and ``frame_bytes >  0``: The decoder skipped ID3 or invalid data  
- 0 samples and ``frame_bytes == 0``: Insufficient data

If ``frame_bytes == 0``, the other fields may be uninitialized or unchanged; if
``frame_bytes != 0``, the other fields are available. The application may call 
``mp3dec_init()`` when changing decode position, but this is not necessary.  

As a special case, the decoder supports already split MP3 streams (for example,
after doing an MP4 demux). In this case, the input buffer must contain _exactly
one_ non-free-format frame.

## Seeking

You can seek to any byte in the stream and call ``mp3dec_decode_frame``; this
will work in almost all cases, but is not completely guaranteed. If granule data
is accidentally detected as a valid MP3 header, short audio artefacting is
possible. If the file is known to be cbr, then all frames have equal size and
lack ID3 tags, which allows us to decode the first frame and calculate all frame
positions as ``frame_bytes * N``. However, because of padding, frames can differ
in size even in this case.

## Bindings

 * https://github.com/tosone/minimp3 - go bindings
 * https://github.com/germangb/minimp3-rs - rust bindings

## Interesting links

 * http://keyj.emphy.de/minimp3/
 * https://github.com/technosaurus/PDMP3
 * https://github.com/technosaurus/PDMP2
 * https://github.com/packjpg/packMP3
 * https://sites.google.com/a/kmlager.com/www/projects
 * https://sourceforge.net/projects/mp3dec/
 * http://blog.bjrn.se/2008/10/lets-build-mp3-decoder.html
 * http://www.mp3-converter.com/mp3codec/
 * http://www.multiweb.cz/twoinches/mp3inside.htm
 * https://www.mp3-tech.org/
 * http://id3.org/mp3Frame
 * https://www.datavoyage.com/mpgscript/mpeghdr.htm
