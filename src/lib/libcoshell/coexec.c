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
 * send an action to the coshell for execution
 */

#include "colib.h"

Cojob_t*
coexec(register Coshell_t* co, const char* action, int flags, const char* out, const char* err, const char* att)
{
	register Cojob_t*	cj;
	register Sfio_t*	sp;
	int			n;
	int			i;
	char*			s;
	char*			env;

	/*
	 * get a free job slot
	 */

	for (cj = co->jobs; cj; cj = cj->next)
		if (cj->pid == CO_PID_FREE)
			break;
	if (!cj)
	{
		if (!(cj = newof(0, Cojob_t, 1, 0)))
			return 0;
		cj->pid = CO_PID_FREE;
		cj->id = ++co->slots;
		cj->next = co->jobs;
		co->jobs = cj;
	}

	/*
	 * set the flags
	 */

	flags &= ~co->mask;
	flags |= co->flags;
	cj->flags = flags;

	/*
	 * package the action
	 */

	if (!(env = coinit(co->flags)))
		return 0;
	if (!(sp = sfstropen()))
		return 0;
	n = strlen(action);
	if (co->flags & CO_SERVER)
	{
		/*
		 * leave it to server
		 */

		sfprintf(sp, "#%05d\ne %d %d %s %s %s",
			0,
			cj->id,
			cj->flags,
			state.pwd,
			out,
			err);
		if (att)
			sfprintf(sp, " (%d:%s)", strlen(att), att);
		else
			sfprintf(sp, " %s", att);
		sfprintf(sp, " (%d:%s) (%d:%s)\n", strlen(env), env, n, action);
	}
	else if (co->flags & CO_INIT)
	{
		if (flags & CO_DEBUG)
			sfprintf(sp, "set -x\n");
		sfprintf(sp, "%s%s\necho x %d $? >&%d\n",
			env,
			action,
			cj->id,
			CO_MSGFD);
	}
	else if (flags & CO_KSH)
	{
#if !_lib_fork && defined(_map_spawnve)
		Sfio_t*	tp;

		tp = sp;
		if (!(sp = sfstropen()))
			sp = tp;
#endif
		sfprintf(sp, "{\ntrap 'set %s$?; trap \"\" 0; IFS=\"\n\"; print -u%d x %d $1 $(times); exit $1' 0 HUP INT QUIT TERM%s\n%s%s%s",
			(flags & CO_SILENT) ? "" : "+x ",
			CO_MSGFD,
			cj->id,
			(flags & CO_IGNORE) ? "" : " ERR",
			env,
			n > CO_MAXEVAL ? "" : "eval '",
			(flags & CO_SILENT) ? "" : "set -x\n");
		if (n > CO_MAXEVAL)
			sfputr(sp, action, -1);
		else
		{
			coquote(sp, action, 0);
			sfprintf(sp, "\n'");
		}
		sfprintf(sp, "\n} </dev/null");
		if (out)
		{
			if (*out == '/')
				sfprintf(sp, " >%s", out);
			else
				sfprintf(sp, " >%s/%s", state.pwd, out);
		}
		if (err)
		{
			if (out && streq(out, err))
				sfprintf(sp, " 2>&1");
			else if (*err == '/')
				sfprintf(sp, " 2>%s", err);
			else
				sfprintf(sp, " 2>%s/%s", state.pwd, err);
		}
#if !_lib_fork && defined(_map_spawnve)
		if (sp != tp)
		{
			sfprintf(tp, "%s -c '", state.sh);
			coquote(tp, sfstruse(sp), 0);
			sfprintf(tp, "' %d>&%d", CO_MSGFD, CO_MSGFD);
			sfstrclose(sp);
			sp = tp;
		}
#endif
		sfprintf(sp, " &\nprint -u%d j %d $!\n",
			CO_MSGFD,
			cj->id);
	}
	else
	{
#if !_lib_fork && defined(_map_spawnve)
		Sfio_t*	tp;

		tp = sp;
		if (!(sp = sfstropen())) sp = tp;
#endif
		flags |= CO_IGNORE;
		sfprintf(sp, "(\n%s%sset -%s%s\n",
			env,
			n > CO_MAXEVAL ? "" : "eval '",
			(flags & CO_IGNORE) ? "" : "e",
			(flags & CO_SILENT) ? "" : "x");
		if (n > CO_MAXEVAL)
			sfprintf(sp, "%s", action);
		else
		{
			coquote(sp, action, 0);
			sfprintf(sp, "\n'");
		}
		sfprintf(sp, "\n) </dev/null");
		if (out)
		{
			if (*out == '/')
				sfprintf(sp, " >%s", out);
			else
				sfprintf(sp, " >%s/%s", state.pwd, out);
		}
		if (err)
		{
			if (out && streq(out, err))
				sfprintf(sp, " 2>&1");
			else if (*err == '/')
				sfprintf(sp, " 2>%s", err);
			else
				sfprintf(sp, " 2>%s/%s", state.pwd, err);
		}
		if (flags & CO_OSH)
			sfprintf(sp, " && echo x %d 0 >&%d || echo x %d $? >&%d",
				cj->id,
				CO_MSGFD,
				cj->id,
				CO_MSGFD);
		else
			sfprintf(sp, " && echo x %d 0 `times` >&%d || echo x %d $? `times` >&%d",
				cj->id,
				CO_MSGFD,
				cj->id,
				CO_MSGFD);
#if !_lib_fork && defined(_map_spawnve)
		if (sp != tp)
		{
			sfprintf(tp, "%s -c '", state.sh);
			coquote(tp, sfstruse(sp), 0);
			sfprintf(tp, "'");
			sfstrclose(sp);
			sp = tp;
		}
#endif
		sfprintf(sp, " &\necho j %d $! >&%d\n",
			cj->id,
			CO_MSGFD);
	}
	n = sfstrtell(sp);
	sfstruse(sp);
	if (flags & CO_SERVER)
		sfprintf(sp, "#%05d\n", n - 7);
	s = sfstrset(sp, 0);
	if (flags & CO_DEBUG)
		sfprintf(sfstderr, "%s: job %d commands:\n\n%s\n", CO_ID, cj->id, s);

	/*
	 * send it off
	 */

	while ((i = write(co->cmdfd, s, n)) > 0 && (n -= i) > 0)
		s += i;
	sfstrclose(sp);
	if (n)
		return 0;

	/*
	 * it's a job
	 */

	cj->pid = 0;
	cj->status = 0;
	cj->local = 0;
	co->outstanding++;
	co->running++;
	co->total++;
	if (co->mode & CO_MODE_ACK)
		cj = cowait(co, cj);
	return cj;
}
