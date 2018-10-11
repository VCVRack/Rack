#ifndef __SPEEX_TYPES_H__
#define __SPEEX_TYPES_H__

#ifdef __TCS__

#include <tmNxTypes.h>



typedef Int16			spx_int16_t;
typedef UInt16			spx_uint16_t;
typedef Int32			spx_int32_t;
typedef UInt32			spx_uint32_t;

#ifdef FIXED_POINT
#define VMUX(a,b,c)		mux((a),(b),(c))
#define VABS(a)			iabs((a))
#define VMAX(a,b)		imax((a),(b))
#define VMIN(a,b)		imin((a),(b))
#else
#define VMUX(a,b,c)		fmux((a),(b),(c))
#define VABS(a)			fabs((a))
#define VMAX(a,b)		fmax((a),(b))
#define VMIN(a,b)		fmin((a),(b))
#endif

#endif


#endif

