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
 * Glenn Fowler
 * AT&T Research
 *
 * close a proc opened by procopen()
 * -1 returned if procopen() had a problem
 * otherwise exit() status of process is returned
 */

#include "proclib.h"

#include <wait.h>

int
procclose(register Proc_t* p)
{
	int	pid;
	int	flags = 0;
	int	status = -1;

	if (p)
	{
		if (p->rfd >= 0)
			close(p->rfd);
		if (p->wfd >= 0 && p->wfd != p->rfd)
			close(p->wfd);
		if (p->flags & PROC_ZOMBIE)
		{
			/*
			 * process may leave a zombie behind
			 * give it a chance to do that but
			 * don't hang waiting for it
			 */

			flags |= WNOHANG;
			sleep(1);
		}
		if (!(p->flags & PROC_FOREGROUND))
			sigcritical(SIG_REG_EXEC|SIG_REG_PROC);
		while ((pid = waitpid(p->pid, &status, flags)) == -1 && errno == EINTR);
		if (pid != p->pid && (flags & WNOHANG))
			status = 0;
		if (!(p->flags & PROC_FOREGROUND))
			sigcritical(0);
		else
		{
			signal(SIGINT, p->sigint);
			signal(SIGQUIT, p->sigquit);
#if defined(SIGCHLD)
#if _lib_sigprocmask
			sigprocmask(SIG_SETMASK, &p->mask, NiL);
#else
#if _lib_sigsetmask
			sigsetmask(p->mask);
#endif
#endif
			signal(SIGCHLD, p->sigchld);
#endif
		}
		if (status != -1 && !(p->flags & PROC_FOREGROUND))
			status = WIFSIGNALED(status) ?
				EXIT_TERM(WTERMSIG(status)) :
				EXIT_CODE(WEXITSTATUS(status));
		procfree(p);
	}
	return status;
}
