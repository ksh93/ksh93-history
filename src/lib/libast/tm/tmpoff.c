/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1985-2001 AT&T Corp.                *
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
*******************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Bell Laboratories
 *
 * time conversion support
 */

#include <ast.h>
#include <tm.h>

/*
 * append p and hh:mm part of n to s
 *
 * n ignored if n==d
 * end of s is returned
 */

char*
tmpoff(register char* s, register const char* p, register int n, int d)
{
	while (*s = *p++) s++;
	if (n != d)
	{
		if (n < 0)
		{
			n = -n;
			*s++ = '-';
		}
		else *s++ = '+';
		s += sfsprintf(s, 16, "%02d", n / 60);
		if (n %= 60)
			s += sfsprintf(s, 16, ":%02d", n);
	}
	return(s);
}
