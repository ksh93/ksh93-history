/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1985-2004 AT&T Corp.                  *
*                      and is licensed under the                       *
*          Common Public License, Version 1.0 (the "License")          *
*                        by AT&T Corp. ("AT&T")                        *
*      Any use, downloading, reproduction or distribution of this      *
*      software constitutes acceptance of the License.  A copy of      *
*                     the License is available at                      *
*                                                                      *
*         http://www.research.att.com/sw/license/cpl-1.0.html          *
*         (with md5 checksum 8a5e0081c856944e76c69a1cf29c2e8b)         *
*                                                                      *
*              Information and Software Systems Research               *
*                            AT&T Research                             *
*                           Florham Park NJ                            *
*                                                                      *
*                 Glenn Fowler <gsf@research.att.com>                  *
*                  David Korn <dgk@research.att.com>                   *
*                   Phong Vo <kpv@research.att.com>                    *
*                                                                      *
***********************************************************************/
#pragma prototyped

/*
 * Glenn Fowler
 * AT&T Research
 *
 * character code map interface
 *
 * NOTE: used for mapping between 8-bit character encodings
 *	 ISO character sets are handled by sfio
 */

#ifndef _CHARCODE_H
#define _CHARCODE_H	1

#include <ast.h>
#include <ast_ccode.h>

/* _cc_map[] for backwards compatibility -- drop 20050101 */

#if _BLD_ast && defined(__EXPORT__)
#define extern		extern __EXPORT__
#endif
#if !_BLD_ast && defined(__IMPORT__)
#define extern		extern __IMPORT__
#endif

extern const unsigned char*	_cc_map[];

#undef	extern

#if _BLD_ast && defined(__EXPORT__)
#define extern		__EXPORT__
#endif

extern unsigned char*	_ccmap(int, int);
extern void*		_ccmapcpy(unsigned char*, void*, const void*, size_t);
extern void*		_ccmapstr(unsigned char*, void*, size_t);

extern int		ccmapid(const char*);
extern char*		ccmapname(int);
extern void*		ccnative(void*, const void*, size_t);

#undef	extern

#define CCOP(i,o)		((i)==(o)?0:(((o)<<8)|(i)))
#define CCIN(x)			((x)&0xFF)
#define CCOUT(x)		(((x)>>8)&0xFF)
#define CCCONVERT(x)		((x)&0xFF00)

#define CCCVT(x)		CCMAP(x,0)
#define CCMAP(i,o)		((i)==(o)?(unsigned char*)0:_ccmap(i,o))
#define CCMAPCHR(m,c)		((m)?m[c]:(c))
#define CCMAPCPY(m,t,f,n)	((m)?_ccmapcpy(m,t,f,n):memcpy(t,f,n))
#define CCMAPSTR(m,s,n)		((m)?_ccmapstr(m,s,n):(void*)(s))

#define ccmap(i,o)		CCMAP(i,o)
#define ccmapchr(m,c)		CCMAPCHR(m,c)
#define ccmapcpy(m,t,f,n)	CCMAPCPY(m,t,f,n)
#define ccmapstr(m,s,n)		CCMAPSTR(m,s,n)

#define CCMAPC(c,i,o)		((i)==(o)?(c):CCMAP(i,o)[c])
#define CCMAPM(t,f,n,i,o)	((i)==(o)?memcpy(t,f,n):_ccmapcpy(CCMAP(i,o),t,f,n))
#define CCMAPS(s,n,i,o)		((i)==(o)?(void*)(s):_ccmapstr(CCMAP(i,o),s,n))

#define ccmapc(c,i,o)		CCMAPC(c,i,o)
#define ccmapm(t,f,n,i,o)	CCMAPM(t,f,n,i,o)
#define ccmaps(s,n,i,o)		CCMAPS(s,n,i,o)

#endif
