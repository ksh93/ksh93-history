/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1985-2001 AT&T Corp.                *
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
*******************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Bell Laboratories
 *
 * return path to file a/b with access mode using : separated dirs
 * both a and b may be 0
 * if (mode&PATH_REGULAR) then path must not be a directory
 * if (mode&PATH_ABSOLUTE) then path must be rooted
 * path returned in path buffer
 */

#include <ast.h>
#include <ls.h>

char*
pathaccess(register char* path, register const char* dirs, const char* a, const char* b, register int mode)
{
	register int	m = 0;
	int		sep = ':';
	char		cwd[PATH_MAX];
	struct stat	st;

#ifdef EFF_ONLY_OK
	m |= EFF_ONLY_OK;
#endif
#ifdef EX_OK
	if (mode == (PATH_EXECUTE|PATH_REGULAR))
	{
		mode &= ~PATH_REGULAR;
		m |= EX_OK;
	}
	else
#endif
	{
		if (mode & PATH_READ) m |= R_OK;
		if (mode & PATH_WRITE) m |= W_OK;
		if (mode & PATH_EXECUTE) m |= X_OK;
	}
	do
	{
		dirs = pathcat(path, dirs, sep, a, b);
		pathcanon(path, 0);
		if (!access(path, m))
		{
			if ((mode & PATH_REGULAR) && (stat(path, &st) || S_ISDIR(st.st_mode))) continue;
			if (*path == '/' || !(mode & PATH_ABSOLUTE)) return(path);
			dirs = getcwd(cwd, sizeof(cwd));
			sep = 0;
		}
	} while (dirs);
	return(0);
}
