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
 * return pointer to formatted elapsed time for t 1/n secs
 * compatible with strelapsed()
 * 8 consecutive calls before overwrite
 * return value length is at most 6
 */

#include <ast.h>

char*
fmtelapsed(register unsigned long t, register int n)
{
	register unsigned long	s;

	static int		amt[] = { 1, 60, 60, 24, 7, 4, 12, 20 };
	static char		chr[] = "smhdwMYS";

	static char		tms[8][7];
	static int		tm;

	if (t == 0L) return("0");
	if (t == ~0L) return("%");
	if (++tm >= elementsof(tms)) tm = 0;
	s = t / n;
	if (s < 60) sfsprintf(tms[tm], sizeof(tms[tm]), "%d.%02ds", s % 100, (t * 100 / n) % 100);
	else
	{
		for (n = 1; n < elementsof(amt) - 1; n++)
		{
			if ((t = s / amt[n]) < amt[n + 1]) break;
			s = t;
		}
		sfsprintf(tms[tm], sizeof(tms[tm]), "%d%c%02d%c", (s / amt[n]) % 100, chr[n], s % amt[n], chr[n - 1]);
	}
	return(tms[tm]);
}
