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
 * ccmapchr(ccmap(CC_NATIVE,CC_ASCII),c) and strcmp
 */

#include <ast.h>
#include <ccode.h>

#if _lib_stracmp

NoN(stracmp)

#else

#include <ctype.h>

#undef	stracmp

int
stracmp(const char* aa, const char* ab)
{
	register unsigned char*	a;
	register unsigned char*	b;
	register unsigned char*	m;
	register int		c;
	register int		d;

	if (!(m = ccmap(CC_NATIVE, CC_ASCII)))
		return strcmp(aa, ab);
	a = (unsigned char*)aa;
	b = (unsigned char*)ab;
	for (;;)
	{
		c = m[*a++];
		if (d = c - m[*b++])
			return d;
		if (!c)
			return 0;
	}
}

#endif
