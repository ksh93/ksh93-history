/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1990-2004 AT&T Corp.                  *
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
*                                                                      *
***********************************************************************/
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
cokilljob(register Coshell_t* co, register Cojob_t* cj, int sig)
{
	int	n;

	if (cj->pid <= 0)
	{
		errno = ESRCH;
		return -1;
	}
	if (sig == SIGKILL)
	{
		co->running--;
		cj->pid = CO_PID_ZOMBIE;
		cj->status = EXIT_TERM(sig);
	}
	n = kill(cj->pid, sig);
	killpg(cj->pid, sig);
	return n;
}

/*
 * kill cj (or all jobs if cj==0) in shell co with sig
 */

static int
cokillshell(register Coshell_t* co, register Cojob_t* cj, int sig)
{
	int	n;

	if (co->flags & CO_SERVER)
	{
		char	buf[CO_BUFSIZ];

		n = sfsprintf(buf, sizeof(buf), "#%05d\nk %d %d\n", 0, cj ? cj->id : 0, sig);
		sfsprintf(buf, 7, "#%05d\n", n - 7);
		return write(co->cmdfd, buf, n) == n ? 0 : -1;
	}
	if (cj)
		return cokilljob(co, cj, sig);
	n = 0;
	for (cj = co->jobs; cj; cj = cj->next)
		if (cj->pid > 0)
			n |= cokilljob(co, cj, sig);
	return n;
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
	if (co)
		return cokillshell(co, cj, sig);
	n = 0;
	for (co = state.coshells; co; co = co->next)
		n |= cokillshell(co, NiL, sig);
	return n;
}
