/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1985-2003 AT&T Corp.                *
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
 * Glenn Fowler
 * AT&T Bell Laboratories
 *
 * convert \x character constants in s in place
 * the length of the converted s is returned (may have embedded \0's)
 */

#include <ast.h>

int
stresc(register char* s)
{
	register char*	t;
	register int	c;
	char*		b;
	char*		p;

	b = t = s;
	for (;;)
	{
		switch (c = *s++)
		{
		case '\\':
			c = chresc(s - 1, &p);
			s = p;
			break;
		case 0:
			*t = 0;
			return(t - b);
		}
		*t++ = c;
	}
}
