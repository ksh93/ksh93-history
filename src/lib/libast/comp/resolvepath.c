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
 * resolvepath implementation
 */

#include <ast.h>
#include <error.h>

#if defined(__EXPORT__)
#define extern	__EXPORT__
#endif

extern char*
resolvepath(const char* file, char* path, size_t size)
{
	register char*	s;
	register int	n;
	register int	r;

	r = *file != '/';
	n = strlen(file) + r + 1;
	if (n >= size)
	{
#ifdef ENAMETOOLONG
		errno = ENAMETOOLONG;
#else
		errno = ENOMEM;
#endif
		return 0;
	}
	if (!r)
		s = path;
	else if (!getcwd(path, size - n))
		return 0;
	else
	{
		s = path + strlen(path);
		*s++ = '/';
	}
	strcpy(s, file);
	return pathcanon(path, PATH_PHYSICAL|PATH_DOTDOT|PATH_EXISTS) ? path : (char*)0;
}
