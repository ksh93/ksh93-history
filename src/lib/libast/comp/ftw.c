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
 * ftw implementation
 */

#include <ast.h>
#include <ftw.h>

static int	(*ftw_userf)(const char*, const struct stat*, int);

static int
ftw_user(Ftw_t* ftw)
{
	register int	n = ftw->info;

	if (n & (FTW_C|FTW_NX))
		n = FTW_DNR;
	else if (n & FTW_SL)
		n = FTW_NS;
	return (*ftw_userf)(ftw->path, &ftw->statb, n);
}

int
ftw(const char* path, int(*userf)(const char*, const struct stat*, int), int depth)
{
	NoP(depth);
	ftw_userf = userf;
	return ftwalk(path, ftw_user, FTW_DOT, NiL);
}
