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
 * strftime implementation
 */

#if defined(__STDPP__directive) && defined(__STDPP__hide)
__STDPP__directive pragma pp:hide strftime
#else
#define strftime	______strftime
#endif

#include <ast.h>
#include <tm.h>

#if defined(__STDPP__directive) && defined(__STDPP__hide)
__STDPP__directive pragma pp:nohide strftime
#else
#undef	strftime
#endif

#undef	_lib_strftime	/* we can pass X/Open */

#if _lib_strftime

NoN(strftime)

#else

#if defined(__EXPORT__)
#define extern	__EXPORT__
#endif

extern size_t
strftime(char* buf, size_t len, const char* format, const struct tm* tm)
{
	register char*	s;
	time_t		t;

	t = tmtime((struct tm*)tm, TM_LOCALZONE);
	if (!(s = tmfmt(buf, len, format, &t)))
		return 0;
	return s - buf;
}

#endif
