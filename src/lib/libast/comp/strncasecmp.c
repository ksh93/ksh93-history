/***************************************************************
*                                                              *
*           This software is part of the ast package           *
*              Copyright (c) 1985-2000 AT&T Corp.              *
*      and it may only be used by you under license from       *
*                     AT&T Corp. ("AT&T")                      *
*       A copy of the Source Code Agreement is available       *
*              at the AT&T Internet web site URL               *
*                                                              *
*     http://www.research.att.com/sw/license/ast-open.html     *
*                                                              *
*     If you received this software without first entering     *
*       into a license with AT&T, you have an infringing       *
*           copy and cannot use it without violating           *
*             AT&T's intellectual property rights.             *
*                                                              *
*               This software was created by the               *
*               Network Services Research Center               *
*                      AT&T Labs Research                      *
*                       Florham Park NJ                        *
*                                                              *
*             Glenn Fowler <gsf@research.att.com>              *
*              David Korn <dgk@research.att.com>               *
*               Phong Vo <kpv@research.att.com>                *
*                                                              *
***************************************************************/
#pragma prototyped

#include <ast.h>

#if _lib_strncasecmp

NoN(strncasecmp)

#else

#include <ctype.h>

#undef	strncasecmp

int
strncasecmp(register const char* a, register const char* b, size_t n)
{
	register const char*	e;
	register int		ac;
	register int		bc;
	register int		d;

	e = a + n;
	for (;;)
	{
		if (a >= e)
			return 0;
		ac = *a++;
		if (isupper(ac))
			ac = tolower(ac);
		bc = *b++;
		if (isupper(bc))
			bc = tolower(bc);
		if (d = ac - bc)
			return d;
		if (!ac)
			return 0;
	}
}

#endif
