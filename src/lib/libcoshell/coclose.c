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
 * close a coshell
 */

#include "colib.h"

/*
 * called when coshell is hung
 */

static void
hung(int sig)
{
	NoP(sig);
	kill(state.current->pid, SIGKILL);
}

/*
 * shut down one coshell
 */

static int
shut(register Coshell_t* co)
{
	register Coshell_t*	cs;
	register Cojob_t*	cj;
	int			n;
	int			status;
	Coshell_t*		ps;
	Cojob_t*		pj;
	Sig_handler_t		handler;

	sfclose(co->msgfp);
	close(co->cmdfd);
	if (co->pid)
	{
		if (co->running > 0)
			killpg(co->pid, SIGTERM);
		state.current = co;
		handler = signal(SIGALRM, hung);
		n = alarm(3);
		if (waitpid(co->pid, &status, 0) != co->pid)
			status = -1;
		alarm(n);
		signal(SIGALRM, handler);
		killpg(co->pid, SIGTERM);
	}
	else status = 0;
	if (co->flags & CO_DEBUG)
		sfprintf(sfstderr, "%s: jobs %d user %s sys %s\n", CO_ID, co->total, fmtelapsed(co->user, CO_QUANT), fmtelapsed(co->sys, CO_QUANT));
	cj = co->jobs;
	while (cj)
	{
		pj = cj;
		cj = cj->next;
		free(pj);
	}
	cs = state.coshells;
	ps = 0;
	while (cs)
	{
		if (cs == co)
		{
			cs = cs->next;
			if (ps)
				ps->next = cs;
			else
				state.coshells = cs;
			free(co);
			break;
		}
		ps = cs;
		cs = cs->next;
	}
	return status;
}

/*
 * close coshell co
 */

int
coclose(register Coshell_t* co)
{
	if (co)
		return shut(co);
	while (state.coshells)
		shut(state.coshells);
	return 0;
}
