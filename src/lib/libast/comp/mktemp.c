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
 * mktemp,mkstemp implementation
 */

#define _def_map_ast	1

#if defined(__STDPP__directive) && defined(__STDPP__hide)
__STDPP__directive pragma pp:hide mktemp mkstemp
#else
#define mktemp		______mktemp
#define mkstemp		______mkstemp
#endif

#include <ast.h>
#include <stdio.h>

#if defined(__STDPP__directive) && defined(__STDPP__hide)
__STDPP__directive pragma pp:nohide mktemp mkstemp
#else
#undef	mktemp
#undef	mkstemp
#endif

#undef	_def_map_ast

#include <ast_map.h>

static char*
temp(char* buf, int* fdp)
{
	char*	s;
	char*	d;
	int	n;
	size_t	len;

	len = strlen(buf);
	if (s = strrchr(buf, '/'))
	{
		*s++ = 0;
		d = buf;
	}
	else
	{
		s = buf;
		d = "";
	}
	if ((n = strlen(s)) < 6 || strcmp(s + n - 6, "XXXXXX"))
		*buf = 0;
	else
	{
		*(s + n - 6) = 0;
		if (!pathtemp(buf, len, d, s, fdp))
			*buf = 0;
	}
	return buf;
}

#if defined(__EXPORT__)
#define extern	__EXPORT__
#endif

extern char*
mktemp(char* buf)
{
	return temp(buf, NiL);
}

extern int
mkstemp(char* buf)
{
	int	fd;

	return *temp(buf, &fd) ? fd : -1;
}
