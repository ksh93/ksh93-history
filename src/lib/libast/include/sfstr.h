/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1985-2004 AT&T Corp.                  *
*                      and is licensed under the                       *
*                  Common Public License, Version 1.0                  *
*                            by AT&T Corp.                             *
*                                                                      *
*                A copy of the License is available at                 *
*            http://www.opensource.org/licenses/cpl1.0.txt             *
*         (with md5 checksum 059e8cd6165cb4c31e351f2b69388fd9)         *
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

#define sfstrtell(f)	((f)->_next - (f)->_data)
#define sfstrpend(f)	((f)->_endb - (f)->_next)
#define sfstrrel(f,p)	((p) == (0) ? (char*)(f)->_next : \
			 ((f)->_next += (p), \
			  ((f)->_next >= (f)->_data && (f)->_next  <= (f)->_endb) ? \
				(char*)(f)->_next : ((f)->_next -= (p), (char*)0) ) )

#define sfstrset(f,p)	(((p) >= 0 && (p) <= (f)->_size) ? \
				(char*)((f)->_next = (f)->_data+(p)) : (char*)0 )

#define sfstrbase(f)	((char*)(f)->_data)
#define sfstrsize(f)	((f)->_size)

#define sfstrrsrv(f,n)	(sfreserve(f,(long)(n),1)?(sfwrite(f,(char*)(f)->_next,0),(char*)(f)->_next):(char*)0)

#define sfstruse(f)	(sfputc(f,0), (char*)((f)->_next = (f)->_data) )

#if _BLD_ast && defined(__EXPORT__)
#define extern		__EXPORT__
#endif

extern int		sfstrtmp(Sfio_t*, int, void*, size_t);

#undef	extern

#endif
