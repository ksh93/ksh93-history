/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1990-2002 AT&T Corp.                *
*        and it may only be used by you under license from         *
*                       AT&T Corp. ("AT&T")                        *
*         A copy of the Source Code Agreement is available         *
*                at the AT&T Internet web site URL                 *
*                                                                  *
*       http://www.research.att.com/sw/license/ast-open.html       *
*                                                                  *
*    If you have copied or used this software without agreeing     *
*        to the terms of the license you are infringing on         *
*           the license and copyright and are violating            *
*               AT&T's intellectual property rights.               *
*                                                                  *
*            Information and Software Systems Research             *
*                        AT&T Labs Research                        *
*                         Florham Park NJ                          *
*                                                                  *
*               Glenn Fowler <gsf@research.att.com>                *
*                                                                  *
*******************************************************************/
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
