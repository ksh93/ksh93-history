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
			sigcritical(1);
		while ((pid = waitpid(p->pid, &status, flags)) == -1 && errno == EINTR);
		if (pid != p->pid && (flags & WNOHANG))
			status = 0;
		if (!(p->flags & PROC_FOREGROUND))
			sigcritical(0);
		else
		{
			signal(SIGINT, p->sigint);
			signal(SIGQUIT, p->sigquit);
		}
		procfree(p);
		if (status != -1)
			status = WIFSIGNALED(status) ?
				EXIT_TERM(WTERMSIG(status)) :
				EXIT_CODE(WEXITSTATUS(status));
	}
	return status ;
}
