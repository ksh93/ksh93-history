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
*        If you have copied this software without agreeing         *
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
*******************************************************************/
#pragma prototyped

#if defined(__STDPP__directive) && defined(__STDPP__hide)
__STDPP__directive pragma pp:hide strstr
#else
#define strstr		______strstr
#endif

#include <ast.h>

#if defined(__STDPP__directive) && defined(__STDPP__hide)
__STDPP__directive pragma pp:nohide strstr
#else
#undef	strstr
#endif

#if _lib_strstr

NoN(strstr)

#else

#if defined(__EXPORT__)
#define extern	__EXPORT__
#endif

extern char*
strstr(register const char* p, register const char* s)
{
	register int		b;
	register int		c;
	register const char*	tp;
	register const char*	ts;
	
	if (s)
	{
		if (!*s)
			return (char*)p;
		b = *p++;
		while (c = *s++)
			if (c == b)
			{
				for (tp = p, ts = s; *tp == *ts; ts++)
					if (!*tp++)
						return (char*)ts;
				if (!*tp)
					return (char*)ts;
			}
	}
	return 0;
}

#endif
