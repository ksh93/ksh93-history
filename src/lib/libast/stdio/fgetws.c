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

#include "stdhdr.h"

wchar_t*
fgetws(wchar_t* s, int n, Sfio_t* f)
{
	register wchar_t*	p = s;
	register wchar_t*	e = s + n - 1;
	register wint_t		c;

	STDIO_PTR(f, "fgets", wchar_t*, (wchar_t*, int, Sfio_t*), (s, n, f))

	FWIDE(f, 0);
	while (p < e && (c = fgetwc(f)) != WEOF && (*p++ = c) != '\n');
	*p = 0;
	return s;
}

wchar_t*
getws(wchar_t* s)
{
	register wchar_t*	p = s;
	register wchar_t*	e = s + BUFSIZ - 1;
	register wint_t		c;

	FWIDE(sfstdin, 0);
	while (p < e && (c = fgetwc(sfstdin)) != WEOF && (*p++ = c) != '\n');
	*p = 0;
	return s;
}
