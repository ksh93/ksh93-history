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
 * Glenn Fowler
 * AT&T Bell Laboratories
 *
 * time conversion support
 */

#include <ast.h>
#include <tm.h>
#include <ctype.h>

/*
 * return minutes offset from absolute timezone expression
 *
 *	[[-+]hh[:mm[:ss]]]
 *	[-+]hhmm
 *
 * if e is non-null then it points to the first unrecognized char in s
 * d returned if no offset in s
 */

int
tmgoff(register const char* s, char** e, int d)
{
	register int	n = d;
	int		east;
	const char*	t = s;

	if ((east = *s == '+') || *s == '-')
	{
		s++;
		if (isdigit(*s) && isdigit(*(s + 1)))
		{
			n = ((*s - '0') * 10 + (*(s + 1) - '0')) * 60;
			s += 2;
			if (*s == ':')
				s++;
			if (isdigit(*s) && isdigit(*(s + 1)))
			{
				n += ((*s - '0') * 10 + (*(s + 1) - '0'));
				s += 2;
				if (*s == ':')
					s++;
				if (isdigit(*s) && isdigit(*(s + 1)))
					s += 2;
			}
			if (east)
				n = -n;
			t = s;
		}
	}
	if (e)
		*e = (char*)t;
	return n;
}
