/* Copyright (C) 2007 Hong Zhiqian */
/**
   @file fixed_tm.h
   @author Hong Zhiqian
   @brief Various compatibility routines for Speex (TriMedia version)
*/
/*
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:
   
   - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
   
   - Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
   
   - Neither the name of the Xiph.org Foundation nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.
   
   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef FIXED_TM_H
#define FIXED_TM_H

#include <ops/custom_defs.h>


#undef	SATURATE
#undef	SATURATE16
#undef	SATURATE32
#define SATURATE(x,a)			iclipi(x,a)
#define SATURATE16(x,a) 		iclipi(x,a)
#define SATURATE32(x,a) 		iclipi(x,a)

#undef	EXTEND32
#define	EXTEND32(x)				sex16(x)

#undef	NEG16
#undef	NEG32
#define NEG16(x)				ineg((int)(x))
#define NEG32(x)				ineg(x)

#undef	ABS
#undef	ABS16
#undef	ABS32
#define	ABS(x)					iabs(x)
#define	ABS32(x)				iabs(x)
#define	ABS16(x)				iabs((int)(x))

#undef	MIN16
#undef	MIN32
#define	MIN16(a,b)				imin((int)(a),(int)(b))
#define	MIN32(a,b)				imin(a,b)

#undef	MAX16
#undef	MAX32
#define	MAX16(a,b)				imax((int)(a),(int)(b))
#define	MAX32(a,b)				imax(a,b)

#undef	ADD16
#undef	SUB16
#undef	ADD32
#undef	SUB32
#undef	MULT16_16
#undef	MULT16_16_16		

#define ADD16(a,b)				((int)(a) + (int)(b))
#define SUB16(a,b)				((int)(a) - (int)(b))
#define ADD32(a,b)				((int)(a) + (int)(b))
#define SUB32(a,b)				((int)(a) - (int)(b))
#define MULT16_16_16(a,b)		((int)(a) * (int)(b))
#define MULT16_16(a,b)			((int)(a) * (int)(b))

#if TM_DEBUGMEM_ALIGNNMENT
#include <stdio.h>
#define TMDEBUG_ALIGNMEM(x)																	\
		{	if( ((int)(x) & (int)(0x00000003)) != 0 )										\
			{	printf("memory not align. file: %s, line: %d\n",  __FILE__, __LINE__);		\
			}																				\
		}

#else
#define TMDEBUG_ALIGNMEM(x)
#endif

#endif

