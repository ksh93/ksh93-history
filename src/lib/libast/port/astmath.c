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
#if LD
	long double	valuel = 0;
#endif
	double		value = 0;
	int		exp = 0;
	int		r = 0;

#if LD
	r |= ldexpl(valuel, exp) != 0;
	r |= frexpl(valuel, &exp) != 0;
#endif
	r |= ldexp(value, exp) != 0;
	r |= frexp(value, &exp) != 0;
	return r;
}
