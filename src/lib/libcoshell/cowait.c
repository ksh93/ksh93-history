/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1990-2004 AT&T Corp.                *
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
*                          AT&T Research                           *
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
		sfprintf(sfstderr, "%s: %s: cannot open job %d serialized output\n", CO_ID, *path, job->id);
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
			break;
		if (!(s = sfgetr(co->msgfp, '\n', 1)) || !*s)
			break;

		/*
		 * read and parse a coshell message packet of the form
		 *
		 *	<type> <id> <args> <newline>
		 *        %c    %d    %s      %c
		 */

		while (isspace(*s))
			s++;
		if (n = *s)
			s++;
		while (*s && !isspace(*s))
			s++;
		id = strtol(s, &e, 10);
		for (s = e; isspace(*s); s++);

		/*
		 * locate id in the job list
		 */

		for (cj = co->jobs; cj; cj = cj->next)
			if (id == cj->id)
				break;
		if ((co->flags | (cj ? cj->flags : 0)) & CO_DEBUG)
			sfprintf(sfstderr, "%s: message: %c %d %s\n", CO_ID, n, id, s);
		if (!cj)
			return 0;

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

		default:
			/*
			 * unknown message
			 */

			sfprintf(sfstderr, "%s: %c: unknown message type\n", CO_ID, n);
			break;

		}
	}
	return 0;
}
