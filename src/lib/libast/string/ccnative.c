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
 * copy table with element size n
 * indexed by CC_ASCII to table
 * indexed by CC_NATIVE
 */

#include <ast.h>
#include <ccode.h>

void*
ccnative(void* b, const void* a, size_t n)
{
#if CC_ASCII == CC_NATIVE
	return memcpy(b, a, n * (UCHAR_MAX + 1));
#else
	register int			c;
	register const unsigned char*	m;
	register unsigned char*		cb = (unsigned char*)b;
	register unsigned char*		ca = (unsigned char*)a;

	m = CCMAP(CC_ASCII, CC_NATIVE);
	if (n == sizeof(char))
		for (c = 0; c <= UCHAR_MAX; c++)
			cb[c] = ca[m[c]];
	else
		for (c = 0; c <= UCHAR_MAX; c++)
			memcpy(cb + n * c, ca + n * m[c], n);
	return b;
#endif
}
