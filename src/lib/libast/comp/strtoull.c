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

#if defined(__STDPP__directive) && defined(__STDPP__hide)
__STDPP__directive pragma pp:hide strtoll strtoull
#else
#define strtoll		______strtoll
#define strtoull	______strtoull
#endif

#include <ast.h>
#include <int.h>

#if defined(__STDPP__directive) && defined(__STDPP__hide)
__STDPP__directive pragma pp:nohide strtoll strtoull
#else
#undef	strtoll
#undef	strtoull
#endif

#if _lib_strtoull

NoN(strtoull)

#else

#if defined(__EXPORT__)
#define extern	__EXPORT__
#endif

extern int_max	strtoll(const char*, char**, int);

extern unsigned int_max
strtoull(const char* str, char** ptr, int base)
{
	return (unsigned int_max)strtoll(str, ptr, base);
}

#endif
