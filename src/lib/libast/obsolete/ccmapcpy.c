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
 * character code string map
 */

#include <ast.h>
#include <ccode.h>

#undef	ccmaps

void*
ccmapcpy(void* b, const void* a, size_t n, int in, int out)
{
	register unsigned char*		ub = (unsigned char*)b;
	register unsigned char*		ue = ub + n;
	register const unsigned char*	ua = (const unsigned char*)a;
	register const unsigned char*	m = CCMAP(in, out);

	while (ub < ue)
		*ub++ = m[*ua++];
	return b;
}
