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
 * AT&T Research
 *
 * sfio discipline interface definitions
 */

#ifndef _SFDISC_H
#define _SFDISC_H

#include <ast.h>

#define SFDCEVENT(a,b,n)	((((a)-'A'+1)<<11)^(((b)-'A'+1)<<6)^(n))

#if _BLD_ast && defined(__EXPORT__)
#define extern		__EXPORT__
#endif

/*
 * %(...) printf support
 */

typedef int (*Sf_key_lookup_t)(void*, const char*, const char*, int, char**, Sflong_t*);
typedef char* (*Sf_key_convert_t)(void*, const char*, const char*, int, char*, Sflong_t);

extern int		sfkeyprintf(Sfio_t*, void*, const char*, Sf_key_lookup_t, Sf_key_convert_t);

/*
 * pure sfio read and/or write disciplines
 */

extern int		sfdcdio(Sfio_t*, size_t);
extern int		sfdcdos(Sfio_t*);
extern int		sfdcfilter(Sfio_t*, const char*);
extern int		sfdcmore(Sfio_t*, const char*, int, int);
extern int		sfdcprefix(Sfio_t*, const char*);
extern int		sfdcseekable(Sfio_t*);
extern int		sfdcslow(Sfio_t*);
extern int		sfdctee(Sfio_t*, Sfio_t*);
extern int		sfdcunion(Sfio_t*, Sfio_t**, int);

extern Sfio_t*		sfdcsubstream(Sfio_t*, Sfio_t*, Sfoff_t, Sfoff_t);

#undef	extern

#endif
