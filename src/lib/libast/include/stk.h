/***************************************************************
*                                                              *
*           This software is part of the ast package           *
*              Copyright (c) 1985-2000 AT&T Corp.              *
*      and it may only be used by you under license from       *
*                     AT&T Corp. ("AT&T")                      *
*       A copy of the Source Code Agreement is available       *
*              at the AT&T Internet web site URL               *
*                                                              *
*     http://www.research.att.com/sw/license/ast-open.html     *
*                                                              *
*     If you received this software without first entering     *
*       into a license with AT&T, you have an infringing       *
*           copy and cannot use it without violating           *
*             AT&T's intellectual property rights.             *
*                                                              *
*               This software was created by the               *
*               Network Services Research Center               *
*                      AT&T Labs Research                      *
*                       Florham Park NJ                        *
*                                                              *
*             Glenn Fowler <gsf@research.att.com>              *
*              David Korn <dgk@research.att.com>               *
*               Phong Vo <kpv@research.att.com>                *
*                                                              *
***************************************************************/
#pragma prototyped
/*
 * David Korn
 * AT&T Research
 *
 * Interface definitions for a stack-like storage library
 *
 */

#ifndef _STK_H
#define _STK_H

#include <sfio.h>

#define _Stk_data	_Stak_data

#define stkstd		(&_Stk_data)

#define	Stk_t		Sfio_t

#define STK_SMALL	1		/* small stkopen stack		*/
#define STK_NULL	2		/* return NULL on overflow	*/

#define	stkptr(sp,n)	((char*)((sp)->data)+(n))
#define	stktell(sp)	((sp)->next-(sp)->data)
#define stkseek(sp,n)	((n)==0?(char*)((sp)->next=(sp)->data):_stkseek(sp,n))

#if _BLD_ast && defined(__EXPORT__)
#define __PUBLIC_DATA__		__EXPORT__
#else
#if !_BLD_ast && defined(__IMPORT__)
#define __PUBLIC_DATA__		__IMPORT__
#else
#define __PUBLIC_DATA__
#endif
#endif

extern __PUBLIC_DATA__ Sfio_t		_Stk_data;

#undef	__PUBLIC_DATA__

#if _BLD_ast && defined(__EXPORT__)
#define extern		__EXPORT__
#endif

extern Stk_t*		stkopen(int);
extern Stk_t*		stkinstall(Stk_t*, char*(*)(int));
extern int		stkclose(Stk_t*);
extern int		stklink(Stk_t*);
extern char*		stkalloc(Stk_t*, unsigned);
extern char*		stkcopy(Stk_t*,const char*);
extern char*		stkset(Stk_t*, char*, unsigned);
extern char*		_stkseek(Stk_t*, unsigned);
extern char*		stkfreeze(Stk_t*, unsigned);

#undef	extern

#endif
