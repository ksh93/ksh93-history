/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1985-2002 AT&T Corp.                *
*        and it may only be used by you under license from         *
*                       AT&T Corp. ("AT&T")                        *
*         A copy of the Source Code Agreement is available         *
*                at the AT&T Internet web site URL                 *
*                                                                  *
*       http://www.research.att.com/sw/license/ast-open.html       *
*                                                                  *
*    If you have copied or used this software without agreeing     *
*        to the terms of the license you are infringing on         *
*           the license and copyright and are violating            *
*               AT&T's intellectual property rights.               *
*                                                                  *
*            Information and Software Systems Research             *
*                        AT&T Labs Research                        *
*                         Florham Park NJ                          *
*                                                                  *
*               Glenn Fowler <gsf@research.att.com>                *
*                David Korn <dgk@research.att.com>                 *
*                 Phong Vo <kpv@research.att.com>                  *
*                                                                  *
*******************************************************************/
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

typedef struct
{
	const unsigned char*	map;
#ifdef _CC_PRIVATE_
	_CC_PRIVATE_
#endif
} Cc_t;

typedef struct
{
	const char*		name;
	const char*		description;
} Ccset_t;

#if _BLD_ast && defined(__EXPORT__)
#define __PUBLIC_DATA__		__EXPORT__
#else
#if !_BLD_ast && defined(__IMPORT__)
#define __PUBLIC_DATA__		__IMPORT__
#else
#define __PUBLIC_DATA__
#endif
#endif

extern __PUBLIC_DATA__ const unsigned char*	_cc_map[];

#undef	__PUBLIC_DATA__

#if _BLD_ast && defined(__EXPORT__)
#define extern		__EXPORT__
#endif

extern Cc_t*	ccopen(const char*, const char*);
extern size_t	cccopy(Cc_t*, char**, size_t*, char**, size_t*);
extern int	ccclose(Cc_t*);
extern Ccset_t*	ccsets(void);

extern int	ccmapc(int, int, int);
extern void*	ccmapcpy(void*, const void*, size_t, int, int);
extern int	ccmapid(const char*);
extern char*	ccmapname(int);
extern void*	ccmaps(void*, size_t, int, int);
extern void*	_ccmaps(void*, size_t, int, int);
extern void*	ccnative(void*, const void*, size_t);

#undef	extern

#define CCMAP(i,o)	_cc_map[(i)*CC_MAPS+(o)]

#define ccmapc(c,i,o)	CCMAPC(c,i,o)
#define CCMAPC(c,i,o)	((i)==(o)?(c):CCMAP(i,o)[c])

#define ccmaps(s,n,i,o)	CCMAPS(s,n,i,o)
#define CCMAPS(s,n,i,o)	((i)==(o)?(void*)(s):_ccmaps(s,n,i,o))

#endif
