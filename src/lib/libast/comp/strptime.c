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
 * strptime implementation
 */

#define _def_map_ast	1

#if defined(__STDPP__directive) && defined(__STDPP__hide)
__STDPP__directive pragma pp:hide strptime
#else
#define strptime	______strptime
#endif

#include <ast.h>
#include <tm.h>

#if defined(__STDPP__directive) && defined(__STDPP__hide)
__STDPP__directive pragma pp:nohide strptime
#else
#undef	strptime
#endif

#undef	_def_map_ast

#include <ast_map.h>

#if _lib_strptime

NoN(strptime)

#else

#if defined(__EXPORT__)
#define extern	__EXPORT__
#endif

extern char*
strptime(const char* s, const char* format, struct tm* tm)
{
	char*	e;
	char*	f;
	time_t	t;

	t = tmtime(tm, TM_LOCALZONE);
	t = tmscan(s, &e, format, &f, &t, 0);
	if (e == (char*)s || *f)
		return 0;
	*tm = *tmmake(&t);
	return e;
}

#endif
