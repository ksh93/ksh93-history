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
 * macro interface for sfio write strings
 *
 * NOTE: see <stak.h> for an alternative interface
 *	 read operations require sfseek()
 */

#ifndef _SFSTR_H
#define _SFSTR_H

#include <sfio.h>

#define sfstropen()	sfnew((Sfio_t*)0,(char*)0,-1,-1,SF_WRITE|SF_STRING)
#define sfstrnew(m)	sfnew((Sfio_t*)0,(char*)0,-1,-1,(m)|SF_STRING)
#define sfstrclose(f)	sfclose(f)

#define sfstrtell(f)	((f)->next - (f)->data)
#define sfstrrel(f,p)	((p) == (0) ? (char*)(f)->next : \
			 ((f)->next += (p), \
			  ((f)->next >= (f)->data && (f)->next  <= (f)->endb) ? \
				(char*)(f)->next : ((f)->next -= (p), (char*)0) ) )

#define sfstrset(f,p)	(((p) >= 0 && (p) <= (f)->size) ? \
				(char*)((f)->next = (f)->data+(p)) : (char*)0 )

#define sfstrbase(f)	((char*)(f)->data)
#define sfstrsize(f)	((f)->size)

#define sfstrrsrv(f,n)	(sfreserve(f,(long)(n),1)?(sfwrite(f,(char*)(f)->next,0),(char*)(f)->next):(char*)0)

#define sfstruse(f)	(sfputc(f,0), (char*)((f)->next = (f)->data) )

#if _BLD_ast && defined(__EXPORT__)
#define extern		__EXPORT__
#endif

extern int		sfstrtmp(Sfio_t*, int, void*, size_t);

#undef	extern

#endif
