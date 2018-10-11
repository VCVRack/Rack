#ifndef __SPEEX_TYPES_H__
#define __SPEEX_TYPES_H__

#if defined HAVE_STDINT_H
#  include <stdint.h>
#elif defined HAVE_INTTYPES_H
#  include <inttypes.h>
#elif defined HAVE_SYS_TYPES_H
#  include <sys/types.h>
#endif

typedef int16_t spx_int16_t;
typedef uint16_t spx_uint16_t;
typedef int32_t spx_int32_t;
typedef uint32_t spx_uint32_t;

#endif

