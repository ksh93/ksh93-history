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
 *  strsort - sort an array pointers using fn
 *
 *	fn follows strcmp(3) conventions
 *
 *   David Korn
 *   AT&T Bell Laboratories
 *
 *  derived from Bourne Shell
 */

#include <ast.h>

void
strsort(char** argv, int n, int(*fn)(const char*, const char*))
{
	register int 	i;
	register int 	j;
	register int 	m;
	register char**	ap;
	char*		s;
	int 		k;

	for (j = 1; j <= n; j *= 2);
	for (m = 2 * j - 1; m /= 2;)
		for (j = 0, k = n - m; j < k; j++)
			for (i = j; i >= 0; i -= m)
			{
				ap = &argv[i];
				if ((*fn)(ap[m], ap[0]) >= 0) break;
				s = ap[m];
				ap[m] = ap[0];
				ap[0] = s;
			}
}
