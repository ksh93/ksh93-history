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
 * return pointer to formatted clock() tics t
 * return value length is at most 6
 */

#include <ast.h>
#include <tm.h>

char*
fmtclock(register Sfulong_t t)
{
	register int		u;
	char*			buf;
	int			z;

	if (t == 0)
		return "0";
	if (t == ((Sfulong_t)~0))
		return "%";
	t = (t * 1000000) / CLOCKS_PER_SEC;
	if (t < 1000)
		u = 'u';
	else if ((t /= 1000) < 1000)
		u = 'm';
	else
		return fmtelapsed(t / 10, 100);
	buf = fmtbuf(z = 7);
	sfsprintf(buf, z, "%I*u%cs", sizeof(t), t, u);
	return buf;
}
