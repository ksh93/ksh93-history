/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1985-2002 AT&T Corp.                *
*        and it may only be used by you under license from         *
*                       AT&T Corp. ("AT&T")                        *
*         A copy of the Source Code Agreement is available         *
*                at the AT&T Internet web site URL                 *
*                                                                  *
*       http://www.research.att.com/sw/license/ast-open.html       *
*                                                                  *
*    If you have copied or used this software without agreeing     *
*        to the terms of the license you are infringing on         *
*           the license and copyright and are violating            *
*               AT&T's intellectual property rights.               *
*                                                                  *
*            Information and Software Systems Research             *
*                        AT&T Labs Research                        *
*                         Florham Park NJ                          *
*                                                                  *
*               Glenn Fowler <gsf@research.att.com>                *
*                David Korn <dgk@research.att.com>                 *
*                 Phong Vo <kpv@research.att.com>                  *
*                                                                  *
*******************************************************************/
#pragma prototyped

/*
 * ccmapc(c, CC_NATIVE, CC_ASCII) and strncmp
 */

#include <ast.h>
#include <ccode.h>

#if _lib_strnacmp

NoN(strnacmp)

#else

#include <ctype.h>

#undef	strnacmp

int
strnacmp(register const char* a, register const char* b, size_t n)
{
	register const char*	e;
	register int		c;
	register int		d;

	if (CC_NATIVE == CC_ASCII)
		return strncmp(a, b, n);
	e = a + n;
	for (;;)
	{
		if (a >= e)
			return 0;
		c = ccmapc(*a++, CC_NATIVE, CC_ASCII);
		if (d = c - ccmapc(*b++, CC_NATIVE, CC_ASCII))
			return d;
		if (!c)
			return 0;
	}
}

#endif
