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
 * G. S. Fowler
 * D. G. Korn
 * AT&T Bell Laboratories
 *
 * shell library support
 */

#include <ast.h>

/*
 * return pointer to the full path name of the shell
 *
 * SHELL is read from the environment and must start with /
 *
 * if set-uid or set-gid then the executable and its containing
 * directory must not be writable by the real user
 *
 * astconf("SHELL",NiL,NiL) is returned by default
 *
 * NOTE: csh is rejected because the bsh/csh differentiation is
 *       not done for `csh script arg ...'
 */

char*
pathshell(void)
{
	register char*	s;
	register char*	sh;
	register int	i;

	static char*	val;

	if ((sh = getenv("SHELL")) && *sh == '/' && strmatch(sh, "*/(sh|*[!cC]sh)"))
	{
		if (!(i = getuid()))
		{
			if (!strmatch(sh, "?(/usr)?(/local)/?(l)bin/?([a-z])sh")) goto defshell;
		}
		else if (i != geteuid() || getgid() != getegid())
		{
			if (!access(sh, W_OK)) goto defshell;
			s = strrchr(sh, '/');
			*s = 0;
			i = access(sh, W_OK);
			*s = '/';
			if (!i) goto defshell;
		}
		return(sh);
	}
 defshell:
	if (!(sh = val))
	{
		if (!*(sh = astconf("SHELL", NiL, NiL)) || !(sh = strdup(sh)))
			sh = "/bin/sh";
		val = sh;
	}
	return(sh);
}
