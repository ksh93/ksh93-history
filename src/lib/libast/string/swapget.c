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
 * Glenn Fowler
 * AT&T Research
 *
 * internal representation conversion support
 */

#include <ast.h>
#include <swap.h>

/*
 * get int_n from b according to op
 */

int_max
swapget(int op, const void* b, int n)
{
	register unsigned char*	p;
	register unsigned char*	d;
	int_max			v;
	unsigned char		tmp[sizeof(int_max)];

	if (n > sizeof(int_max))
		n = sizeof(int_max);
	if (op) swapmem(op, b, d = tmp, n);
	else d = (unsigned char*)b;
	p = d + n;
	v = 0;
	while (d < p)
	{
		v <<= CHAR_BIT;
		v |= *d++;
	}
	return v;
}
