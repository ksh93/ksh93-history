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
