/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*           Copyright (c) 1985-2006 AT&T Knowledge Ventures            *
*                      and is licensed under the                       *
*                  Common Public License, Version 1.0                  *
*                      by AT&T Knowledge Ventures                      *
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
 * return number n scaled to metric multiples of k { 1000 1024 }
 * return string length is at most 7 chars + terminating nul
 */

#include <ast.h>
#include <lclib.h>

char*
fmtscale(register Sfulong_t n, int k)
{
	register Sfulong_t	m;
	int			r;
	int			z;
	const char*		u;
	char			suf[3];
	char*			s;
	char*			buf;
	Lc_numeric_t*		p = (Lc_numeric_t*)LCINFO(AST_LC_NUMERIC)->data;

	static const char	scale[] = "bkMGTPE";

	m = 0;
	u = scale;
	while (n >= 1000 && *(u + 1))
	{
		m = n;
		n /= k;
		u++;
	}
	buf = fmtbuf(z = 8);
	r = (m % k) / (k / 10 + 1);
	s = suf;
	if (u > scale)
	{
		if (k == 1024)
		{
			*s++ = *u == 'k' ? 'K' : *u;
			*s++ = 'i';
		}
		else
			*s++ = *u;
	}
	*s = 0;
	if (n > 0 && n < 10)
		sfsprintf(buf, z, "%I*u%c%d%s", sizeof(n), n, p->decimal >= 0 ? p->decimal : '.', r, suf);
	else
	{
		if (r >= 5)
			n++;
		sfsprintf(buf, z, "%I*u%s", sizeof(n), n, suf);
	}
	return buf;
}
