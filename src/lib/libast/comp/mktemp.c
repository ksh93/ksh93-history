/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1985-2000 AT&T Corp.                *
*        and it may only be used by you under license from         *
*                       AT&T Corp. ("AT&T")                        *
*         A copy of the Source Code Agreement is available         *
*                at the AT&T Internet web site URL                 *
*                                                                  *
*       http://www.research.att.com/sw/license/ast-open.html       *
*                                                                  *
*        If you have copied this software without agreeing         *
*        to the terms of the license you are infringing on         *
*           the license and copyright and are violating            *
*               AT&T's intellectual property rights.               *
*                                                                  *
*                 This software was created by the                 *
*                 Network Services Research Center                 *
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
 * mktemp,mkstemp implementation
 */

#if defined(__EXPORT__)
__EXPORT__ char*	mktemp(char*);
__EXPORT__ int		mkstemp(char*);
#endif

#include <ast.h>
#include <stdio.h>

static char*
temp(char* buf, int* fdp)
{
	char*	s;
	char*	d;
	int	n;

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
		if (!pathtmp(buf, d, s, fdp))
			*buf = 0;
	}
	return buf;
}

char*
mktemp(char* buf)
{
	return temp(buf, NiL);
}

int
mkstemp(char* buf)
{
	int	fd;

	return *temp(buf, &fd) ? fd : -1;
}
