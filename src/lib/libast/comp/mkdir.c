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

#include <ast.h>

#if _lib_mkdir

NoN(mkdir)

#else

#include <ls.h>
#include <wait.h>
#include <error.h>

int
mkdir(const char* path, mode_t mode)
{
	register int	n;
	char*		av[3];

	static char*	cmd[] = { "/bin/mkdir", "/usr/5bin/mkdir", 0 };


	n = errno;
	if (!access(path, F_OK))
	{
		errno = EEXIST;
		return(-1);
	}
	if (errno != ENOENT) return(-1);
	errno = n;
	av[0] = "mkdir";
	av[1] = path;
	av[2] = 0;
	for (n = 0; n < elementsof(cmd); n++)
		if (procclose(procopen(cmd[n], av, NiL, NiL, 0)) != -1)
			break;
	return(chmod(path, mode));
}

#endif
