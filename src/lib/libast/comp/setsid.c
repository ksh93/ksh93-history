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

#if _lib_setsid

NoN(setsid)

#else

#include <ast_tty.h>
#include <error.h>

/*
 * become new process group leader and drop control tty
 */

pid_t
setsid(void)
{
	int	pg;
#ifdef TIOCNOTTY
	int	fd;
#endif

	/*
	 * become a new process group leader
	 */

	if ((pg = getpid()) == getpgrp())
	{
		errno = EPERM;
		return(-1);
	}
	setpgid(pg, pg);
#ifdef TIOCNOTTY

	/*
	 * drop the control tty
	 */

	if ((fd = open("/dev/tty", O_RDONLY)) >= 0)
	{
		ioctl(fd, TIOCNOTTY, 0);
		close(fd);
	}
#else

	/*
	 * second child in s5 to avoid reacquiring the control tty
	 */

#if _lib_fork && HUH920711 /* some s5's botch this */
	switch (fork())
	{
	case -1:
		exit(1);
	case 0:
		break;
	default:
		exit(0);
	}
#endif

#endif
	return(pg);
}

#endif
