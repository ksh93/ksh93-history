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
 * Xopen 4.2 compatibility
 */

#if defined(__EXPORT__)
#define extern	__EXPORT__
#endif

extern int	getsubopt(char**, char* const*, char**);

#undef	extern

#include <ast.h>

#if _lib_getsubopt

NoN(getsubopt)

#else

#include <error.h>

extern int
getsubopt(register char** op, char* const* tp, char** vp)
{
	register char*	b;
	register char*	s;
	register char*	v;

	if (*(b = *op))
	{
		v = 0;
		s = b;
		for (;;)
		{
			switch (*s++)
			{
			case 0:
				s--;
				break;
			case ',':
				*(s - 1) = 0;
				break;
			case '=':
				if (!v)
				{
					*(s - 1) = 0;
					v = s;
				}
				continue;
			default:
				continue;
			}
			break;
		}
		*op = s;
		*vp = v;
		for (op = (char**)tp; *op; op++)
			if (streq(b, *op))
				return op - (char**)tp;
	}
	*vp = b;
	return -1;
}

#endif
