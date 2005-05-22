/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1990-2005 AT&T Corp.                  *
*                      and is licensed under the                       *
*                  Common Public License, Version 1.0                  *
*                            by AT&T Corp.                             *
*                                                                      *
*                A copy of the License is available at                 *
*            http://www.opensource.org/licenses/cpl1.0.txt             *
*         (with md5 checksum 059e8cd6165cb4c31e351f2b69388fd9)         *
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
 * wait for and return status of job or the next coshell job that completes
 * job==co for non-blocking wait
 */

#include "colib.h"

#include <ctype.h>

/*
 * cat and remove fd {1,2} serialized output
 */

static void
cat(Cojob_t* job, char** path, Sfio_t* op)
{
	Sfio_t*		sp;

	if (sp = sfopen(NiL, *path, "r"))
	{
		sfmove(sp, op, SF_UNBOUND, -1);
		sfclose(sp);
	}
	else
		errormsg(state.lib, ERROR_LIBRARY|2, "%s: cannot open job %d serialized output", *path, job->id);
	remove(*path);
	free(*path);
	*path = 0;
}

Cojob_t*
cowait(register Coshell_t* co, Cojob_t* job)
{
	register char*		s;
	register Cojob_t*	cj;
	register ssize_t	n;
	char*			b;
	char*			e;
	int			id;
	int			noblock;
	char			buf[32];

	if (noblock = job == (Cojob_t*)co)
		job = 0;
	if (co->outstanding > co->running)
		for (cj = co->jobs; cj; cj = cj->next)
			if (cj->pid == CO_PID_ZOMBIE && (!job || cj == job))
			{
				cj->pid = CO_PID_FREE;
				co->outstanding--;
				return cj;
			}
	if (co->running <= 0)
		return 0;
	for (;;)
	{
		if (noblock && sfpoll(&co->msgfp, 1, 0) != 1)
			return 0;
		if (!(s = b = sfgetr(co->msgfp, '\n', 1)))
			return 0;

		/*
		 * read and parse a coshell message packet of the form
		 *
		 *	<type> <id> <args> <newline>
		 *        %c    %d    %s      %c
		 */

		while (isspace(*s))
			s++;
		if (!(n = *s) || n != 'a' && n != 'j' && n != 'x')
			break;
		while (*++s && !isspace(*s));
		id = strtol(s, &e, 10);
		if (!isspace(*e))
			break;
		for (s = e; isspace(*s); s++);

		/*
		 * locate id in the job list
		 */

		for (cj = co->jobs; cj; cj = cj->next)
			if (id == cj->id)
				break;
		if ((co->flags | (cj ? cj->flags : 0)) & CO_DEBUG)
			errormsg(state.lib, 2, "message \"%c %d %s\"", n, id, s);
		if (!cj)
		{
			errormsg(state.lib, 2, "job id %d not found [%s]", id, b);
			return 0;
		}

		/*
		 * now interpret the message
		 */

		switch (n)
		{

		case 'a':
			/*
			 * coexec() ack
			 */

			if (cj == job)
				return cj;
			break;

		case 'j':
			/*
			 * <s> is the job pid
			 */

			n = cj->pid;
			cj->pid = strtol(s, NiL, 10);
			if (n == CO_PID_WARPED)
				goto nuke;
			break;

		case 'x':
			/*
			 * <s> is the job exit code and user,sys times
			 */

			cj->status = strtol(s, &e, 10);
			for (;;)
			{
				if (e <= s)
					break;
				for (s = e; isalpha(*s) || isspace(*s); s++);
				cj->user += strelapsed(s, &e, CO_QUANT);
				if (e <= s)
					break;
				for (s = e; isalpha(*s) || isspace(*s); s++);
				cj->sys += strelapsed(s, &e, CO_QUANT);
			}
			co->user += cj->user;
			co->sys += cj->sys;
			if (cj->out)
				cat(cj, &cj->out, sfstdout);
			if (cj->err)
				cat(cj, &cj->err, sfstderr);
			if (cj->pid > 0 || (co->flags & (CO_INIT|CO_SERVER)))
			{
			nuke:
				if (cj->pid > 0)
				{
					/*
					 * nuke the zombies
					 */

					n = sfsprintf(buf, sizeof(buf), "wait %d\n", cj->pid);
					write(co->cmdfd, buf, n);
				}
				co->running--;
				if (!job || cj == job)
				{
					cj->pid = CO_PID_FREE;
					co->outstanding--;
					return cj;
				}
				cj->pid = CO_PID_ZOMBIE;
			}
			else
				cj->pid = CO_PID_WARPED;
			break;

		}
	}
	errormsg(state.lib, 2, "invalid message \"%-.*s>>>%s<<<\"", s - b, b, s);
	return 0;
}
