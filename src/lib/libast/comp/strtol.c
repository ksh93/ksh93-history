/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1985-2000 AT&T Corp.                *
*        and it may only be used by you under license from         *
*                       AT&T Corp. ("AT&T")                        *
*         A copy of the Source Code Agreement is available         *
*                at the AT&T Internet web site URL                 *
*                                                                  *
*       http://www.research.att.com/sw/license/ast-open.html       *
*                                                                  *
*        If you have copied this software without agreeing         *
*        to the terms of the license you are infringing on         *
*           the license and copyright and are violating            *
*               AT&T's intellectual property rights.               *
*                                                                  *
*                 This software was created by the                 *
*                 Network Services Research Center                 *
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
 * AT&T Research
 *
 * convert string to long integer
 */

#include <ast.h>

#if _lib_strtol

NoN(strtol)

#else

#include <ctype.h>
#include <error.h>

#include "sfhdr.h"

long
strtol(const char* a, char** e, register int base)
{
	register unsigned char*	s = (char*)a;
	register long		n;
	register int		c;
	register int		shift;
	register unsigned char*	cv;
	int			negative;

	if (base > 36 && base <= 64)
	{
		static int	conformance = -1;

		if (conformance < 0)
			conformance = !strcmp(astconf("CONFORMANCE", NiL, NiL), "standard");
		if (conformance)
			base = SF_RADIX + 1;
	}
	if (base < 0 || base > SF_RADIX)
	{
		errno = EINVAL;
		return 0;
	}
	while (isspace(*s))
		s++;
	if ((negative = (*s == '-')) || *s == '+')
		s++;
	if (base <= 1)
	{
		if (*s == '0')
		{
			if (*(s + 1) == 'x' || *(s + 1) == 'X')
			{
				s += 2;
				base = 16;
			}
			else base = 8;
		}
		else base = 10;
	}

	/*
	 * this part transcribed from sfvscanf()
	 */

	n = 0;
	if (base == 10)
	{
		while ((c = *s++) >= '0' && c <= '9')
			n = (n << 3) + (n << 1) + (c - '0');
	}
	else
	{
		SFCVINIT();
		cv = base <= 36 ? _Sfcv36 : _Sfcv64;
		if ((base & ~(base - 1)) == base)
		{	
			if (base < 8)
				shift = base <  4 ? 1 : 2;
			else if (base < 32)
				shift = base < 16 ? 3 : 4;
			else
				shift = base < 64 ? 5 : 6;
			while ((c = cv[*s++]) < base)
				n = (n << shift) + c;
		}
		else while ((c = cv[*s++]) < base)
			n = (n * base) + c;
	}
	if (e)
		*e = (char*)(s - 1);
	return negative ? -n : n;
}

#endif
