/***************************************************************
*                                                              *
*           This software is part of the ast package           *
*              Copyright (c) 1990-2000 AT&T Corp.              *
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
*                                                              *
***************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Research
 *
 * if co==0 then kill all coshell jobs with sig
 * elif cj==0 then kill co jobs with sig
 * else kill cj with sig
 */

#include "colib.h"

/*
 * kill job cj in shell co with signal sig
 */

static int
killjob(register Coshell_t* co, register Cojob_t* cj, int sig)
{
	int	n;

	if (cj->pid <= 0)
	{
		errno = ESRCH;
		return(-1);
	}
	if (sig == SIGKILL)
	{
		co->running--;
		cj->pid = CO_PID_ZOMBIE;
		cj->status = EXIT_TERM(sig);
	}
	n = kill(cj->pid, sig);
	killpg(cj->pid, sig);
	return(n);
}

/*
 * kill cj (or all jobs if cj==0) in shell co with sig
 */

static int
killshell(register Coshell_t* co, register Cojob_t* cj, int sig)
{
	int	n;

	if (co->flags & CO_SERVER)
	{
		char	buf[CO_BUFSIZ];

		n = sfsprintf(buf, sizeof(buf), "#%05d\nk %d %d\n", 0, cj ? cj->id : 0, sig);
		sfsprintf(buf, 7, "#%05d\n", n - 7);
		return(write(co->cmdfd, buf, n) == n ? 0 : -1);
	}
	if (cj) return(killjob(co, cj, sig));
	n = 0;
	for (cj = co->jobs; cj; cj = cj->next)
		if (cj->pid > 0)
			n |= killjob(co, cj, sig);
	return(n);
}

int
cokill(register Coshell_t* co, register Cojob_t* cj, int sig)
{
	int	n;

	switch (sig)
	{
	case SIGINT:
		sig = SIGTERM;
		break;
#if defined(SIGSTOP) && defined(SIGTSTP)
	case SIGTSTP:
		sig = SIGSTOP;
		break;
#endif
	}
	if (co) return(killshell(co, cj, sig));
	n = 0;
	for (co = state.coshells; co; co = co->next)
		n |= killshell(co, NiL, sig);
	return(n);
}
