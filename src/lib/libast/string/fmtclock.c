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

	static unsigned int	clk_tck;

	if (!clk_tck)
	{
#ifdef CLOCKS_PER_SEC
		clk_tck = CLOCKS_PER_SEC;
#else
		if (!(clk_tck = (unsigned int)strtoul(astconf("CLK_TCK", NiL, NiL), NiL, 10)))
			clk_tck = 60;
#endif
	}
	if (t == 0)
		return "0";
	if (t == ((Sfulong_t)~0))
		return "%";
	t = (t * 1000000) / clk_tck;
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
