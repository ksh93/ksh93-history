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
/*
 * used to test if -last requires -lm
 *
 *	arch		-last			-lm
 *	----		-----			---
 *	linux.sparc	sfdlen,sfputd		frexp,ldexp	
 */

#include <math.h>

int
main()
{
#if N & 1
	long double	valuel = 0;
#else
	double		value = 0;
#endif
#if N < 5
	int		exp = 0;
#endif

#if N == 1
	return ldexpl(valuel, exp) != 0;
#endif
#if N == 2
	return ldexp(value, exp) != 0;
#endif
#if N == 3
	return frexpl(valuel, &exp) != 0;
#endif
#if N == 4
	return frexp(value, &exp) != 0;
#endif
#if N == 5
	return isnanl(valuel);
#endif
#if N == 6
	return isnan(value);
#endif
}
