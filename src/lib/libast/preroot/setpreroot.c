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
 * AT&T Bell Laboratories
 * force current command to run under dir preroot
 */

#include <ast.h>
#include <preroot.h>

#if FS_PREROOT

#include <option.h>

void
setpreroot(register char** argv, const char* dir)
{
	register char*	s;
	register char**	ap;
	int		argc;
	char*		cmd;
	char**		av;
	char		buf[PATH_MAX];

	if ((argv || (argv = opt_info.argv)) && (dir || (dir = getenv(PR_BASE)) && *dir) && !ispreroot(dir) && (*(cmd = *argv++) == '/' || (cmd = pathpath(buf, cmd, NiL, PATH_ABSOLUTE|PATH_REGULAR|PATH_EXECUTE))))
	{
		argc = 3;
		for (ap = argv; *ap++; argc++);
		if (av = newof(0, char*, argc, 0))
		{
			ap = av;
			*ap++ = PR_COMMAND;
			*ap++ = dir;
			*ap++ = cmd;
			while (*ap++ = *argv++);
			if (!(s = getenv(PR_SILENT)) || !*s)
			{
				sfprintf(sfstderr, "+");
				ap = av;
				while (s = *ap++)
					sfprintf(sfstderr, " %s", s);
				sfprintf(sfstderr, "\n");
				sfsync(sfstderr);
			}
			execv(*av, av);
			free(av);
		}
	}
}

#else

NoN(setpreroot)

#endif
