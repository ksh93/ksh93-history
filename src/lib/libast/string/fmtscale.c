/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1985-2005 AT&T Corp.                  *
*                      and is licensed under the                       *
*                  Common Public License, Version 1.0                  *
*                            by AT&T Corp.                             *
*                                                                      *
*                A copy of the License is available at                 *
*            http://www.opensource.org/licenses/cpl1.0.txt             *
*         (with md5 checksum 059e8cd6165cb4c31e351f2b69388fd9)         *
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
 * Glenn Fowler
 * AT&T Research
 *
 * return number n scaled to metric powers of k { 1000 1024 }
 */

#include <ast.h>

char*
fmtscale(register Sfulong_t n, int k)
{
	register Sfulong_t	m;
	int			r;
	int			z;
	const char*		u;
	char*			buf;

	static const char	scale[] = "bKMGTPX";

	m = 0;
	u = scale;
	while (n > k && *(u + 1))
	{
		m = n;
		n /= k;
		u++;
	}
	buf = fmtbuf(z = 8);
	r = (m % k) / (k / 10 + 1);
	if (n > 0 && n < 10)
		sfsprintf(buf, z, "%I*u.%d%c", sizeof(n), n, r, *u);
	else
	{
		if (r >= 5)
			n++;
		sfsprintf(buf, z, "%I*u%c", sizeof(n), n, *u);
	}
	return buf;
}
