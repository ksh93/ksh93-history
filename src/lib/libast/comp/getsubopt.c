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
 * Xopen 4.2 compatibility
 */

#include <ast_lib.h>

#if _lib_getsubopt

#include <ast.h>

NoN(getsubopt)

#else

#define getsubopt	______getsubopt

#include <ast.h>

#undef	getsubopt

#include <error.h>

#if defined(__EXPORT__)
#define extern	__EXPORT__
#endif

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
