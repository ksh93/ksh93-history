/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1990-2008 AT&T Intellectual Property          *
*                      and is licensed under the                       *
*                  Common Public License, Version 1.0                  *
*                    by AT&T Intellectual Property                     *
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
	register Coservice_t*	cs;
	register ssize_t	n;
	char*			b;
	char*			e;
	unsigned long		user;
	unsigned long		sys;
	int			id;
	int			noblock;
	char			buf[32];

	if (co)
	{
		if (noblock = job == (Cojob_t*)co)
			job = 0;
	zombies:
		if ((co->outstanding + co->svc_outstanding) > (co->running + co->svc_running))
			for (cj = co->jobs; cj; cj = cj->next)
				if (cj->pid == CO_PID_ZOMBIE && (!job || cj == job))
				{
					cj->pid = CO_PID_FREE;
					if (cj->service)
						co->svc_outstanding--;
					else
						co->outstanding--;
					return cj;
				}
				else if (cj->service && !cj->service->pid)
				{
					cj->pid = CO_PID_ZOMBIE;
					cj->status = 2;
					cj->service = 0;
					co->svc_running--;
				}
		if (co->running <= 0)
		{
			if (co->svc_running <= 0)
				return 0;
			n = 0;
			for (cs = co->service; cs; cs = cs->next)
				if (cs->pid && kill(cs->pid, 0))
				{
					cs->pid = 0;
					close(cs->fd);
					cs->fd = -1;
					n = 1;
				}
			if (n)
				goto zombies;
		}
	}
	else if (!(co = (Coshell_t*)job))
		return 0;
	else
	{
		job = 0;
		noblock = 1;
	}
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
		if (*e && !isspace(*e))
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
			user = sys = 0;
			for (;;)
			{
				if (e <= s)
					break;
				for (s = e; isalpha(*s) || isspace(*s); s++);
				user += strelapsed(s, &e, CO_QUANT);
				if (e <= s)
					break;
				for (s = e; isalpha(*s) || isspace(*s); s++);
				sys += strelapsed(s, &e, CO_QUANT);
			}
			cj->user += user;
			cj->sys += sys;
			co->user += user;
			co->sys += sys;
			if (cj->out)
				cat(cj, &cj->out, sfstdout);
			if (cj->err)
				cat(cj, &cj->err, sfstderr);
			if (cj->pid > 0 || cj->service || (co->flags & (CO_INIT|CO_SERVER)))
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
				if (cj->service)
					co->svc_running--;
				else
					co->running--;
				if (!job || cj == job)
				{
					cj->pid = CO_PID_FREE;
					if (cj->service)
						co->svc_outstanding--;
					else
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

/*
 * the number of running+zombie jobs
 * these would count against --jobs or NPROC
 */

int
cojobs(Coshell_t* co)
{
	return co->outstanding;
}

/*
 * the number of pending cowait()'s
 */

int
copending(Coshell_t* co)
{
	return co->outstanding + co->svc_outstanding;
}

/*
 * the number of completed jobs not cowait()'d for
 * cowait() always reaps the zombies first
 */

int
cozombie(Coshell_t* co)
{
	return (co->outstanding + co->svc_outstanding) - (co->running + co->svc_running);
}
