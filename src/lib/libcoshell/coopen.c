/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1990-2001 AT&T Corp.                *
*        and it may only be used by you under license from         *
*                       AT&T Corp. ("AT&T")                        *
*         A copy of the Source Code Agreement is available         *
*                at the AT&T Internet web site URL                 *
*                                                                  *
*       http://www.research.att.com/sw/license/ast-open.html       *
*                                                                  *
*        If you have copied this software without agreeing         *
*        to the terms of the license you are infringing on         *
*           the license and copyright and are violating            *
*               AT&T's intellectual property rights.               *
*                                                                  *
*                 This software was created by the                 *
*                 Network Services Research Center                 *
*                        AT&T Labs Research                        *
*                         Florham Park NJ                          *
*                                                                  *
*               Glenn Fowler <gsf@research.att.com>                *
*******************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Research
 *
 * open a new coshell
 */

#include "colib.h"

#include <error.h>
#include <namval.h>
#include <proc.h>
#include <sfdisc.h>
#include <tok.h>

static char		lib[] = "libcoshell:coshell";

static const Namval_t	options[] =
{
	"cross",	CO_CROSS,
	"debug",	CO_DEBUG,
	"devfd",	CO_DEVFD,
	"ignore",	CO_IGNORE,
	"silent",	CO_SILENT,
	0,		0
};

Costate_t		state = { 0 };

/*
 * called when ident sequence hung
 */

static void
hung(int sig)
{
	NoP(sig);
	sfclose(state.current->msgfp);
}

/*
 * close all open coshells
 */

static void
clean(void)
{
	coclose(NiL);
}

#ifdef SIGCONT

/*
 * pass job control signals to the coshell and self
 */

static void
stop(int sig)
{
	cokill(NiL, NiL, sig);
	signal(sig, SIG_DFL);
	sigunblock(sig);
	kill(getpid(), sig);
	cokill(NiL, NiL, SIGCONT);
	signal(sig, stop);
}

#endif

/*
 * called by stropt() to set options
 */

static int
setopt(register void* a, register const void* p, int n, const char* v)
{
	NoP(v);
	if (p)
	{
		if (n)
			((Coshell_t*)a)->flags |= ((Namval_t*)p)->value;
		else
			((Coshell_t*)a)->mask |= ((Namval_t*)p)->value;
	}
	return 0;
}

Coshell_t*
coopen(const char* path, int flags, const char* attributes)
{
	register Coshell_t*	co;
	register char*		s;
	register int		i;
	char*			t;
	int			n;
	int			msgfd;
	Proc_t*			proc;
	Cojob_t*		cj;
	Sfio_t*			sp;
	Sig_handler_t		handler;
	int			pio[2];
	int			pex[2];
	long			ops[5];
	char			devfd[16];
	char*			av[8];

	static char*	sh[] = { 0, 0, "ksh", "sh", "/bin/sh" };

	if (!state.type && (!(s = getenv(CO_ENV_TYPE)) || !(state.type = strdup(s))))
		state.type = "";
	if ((flags & CO_ANY) && (co = state.coshells))
		return co;
	if (!(co = newof(0, Coshell_t, 1, 0)))
	{
		errormsg(lib, ERROR_LIBRARY|2, "out of space");
		return 0;
	}
	pex[0] = -1;
	pex[1] = -1;
	pio[0] = -1;
	pio[1] = -1;
	stropt(getenv(CO_ENV_OPTIONS), options, sizeof(*options), setopt, co);
	co->flags |= CO_INIT | ((flags | CO_DEVFD) & ~co->mask);
	if ((msgfd = fcntl(0, F_DUPFD, CO_MSGFD)) != CO_MSGFD)
	{
		if (msgfd < 0)
		{
			errormsg(lib, ERROR_LIBRARY|ERROR_SYSTEM|2, "cannot reserve fd %d", CO_MSGFD);
			errormsg(lib, ERROR_LIBRARY|2, "fd status: %d[%s] %d[%s]",
				0, fcntl(0, F_GETFD, 0) >= 0 ? "open" : fmterror(errno),
				CO_MSGFD, fcntl(CO_MSGFD, F_GETFD, 0) >= 0 ? "open" : fmterror(errno));
			goto bad;
		}
		close(msgfd);
		msgfd = -1;
	}
	if (pipe(pex) < 0 || pipe(pio) < 0)
	{
		errormsg(lib, ERROR_LIBRARY|ERROR_SYSTEM|2, "cannot allocate pipes");
		goto bad;
	}
	if (msgfd >= 0)
		close(msgfd);
	co->cmdfd = pio[1];
	if (!(co->msgfp = sfnew(NiL, NiL, 256, pex[0], SF_READ)))
	{
		errormsg(lib, ERROR_LIBRARY|ERROR_SYSTEM|2, "cannot allocate message stream");
		goto bad;
	}
	sfdcslow(co->msgfp);
	ops[0] = PROC_FD_DUP(pio[0], 0, PROC_FD_PARENT);
	ops[1] = PROC_FD_CLOSE(pio[1], PROC_FD_CHILD);
	ops[2] = PROC_FD_CLOSE(pex[0], PROC_FD_CHILD);
	ops[3] = PROC_FD_DUP(pex[1], CO_MSGFD, PROC_FD_PARENT|PROC_FD_CHILD);
	ops[4] = 0;
	sfsprintf(devfd, sizeof(devfd), "/dev/fd/%d", pio[0]);
	flags = !access(devfd, F_OK);
	sh[0] = (char*)path;
	sh[1] = getenv(CO_ENV_SHELL);
	for (i = 0; i < elementsof(sh); i++)
		if ((s = sh[i]) && *s && (s = strdup(s)) && (n = tokscan(s, NiL, " %v ", av, elementsof(av) - 1)) > 0)
		{
			if (t = strrchr(s = av[0], '/'))
				av[0] = t + 1;
			if (flags || (co->flags & CO_DEVFD) && strmatch(s, "*ksh*"))
				av[n++] = devfd;
			av[n] = 0;
			proc = procopen(s, av, NiL, ops, PROC_DAEMON|PROC_IGNORE);
			if (proc)
			{
				if (!state.sh)
					state.sh = strdup(s);
				free(s);
				co->pid = proc->pid;
				procfree(proc);
				break;
			}
			free(s);
		}
	if (i >= elementsof(sh))
	{
		errormsg(lib, ERROR_LIBRARY|ERROR_SYSTEM|2, "cannot execute");
		goto bad;
	}

	/*
	 * send the shell identification sequence
	 */

	if (!(sp = sfstropen()))
	{
		errormsg(lib, ERROR_LIBRARY|2, "out of buffer space");
		goto bad;
	}
	sfprintf(sp, "#%05d\n%s='", 0, CO_ENV_ATTRIBUTES);
	if (t = getenv(CO_ENV_ATTRIBUTES))
	{
		coquote(sp, t, 0);
		if (attributes)
			sfprintf(sp, ",");
	}
	if (attributes)
		coquote(sp, attributes, 0);
	sfprintf(sp, "'\n%s", coident);
	i = sfstrtell(sp);
	sfstrset(sp, 0);
	sfprintf(sp, "#%05d\n", i - 7);
	i = write(co->cmdfd, sfstrbase(sp), i) != i;
	sfstrclose(sp);
	if (i)
	{
		errormsg(lib, ERROR_LIBRARY|ERROR_SYSTEM|2, "cannot write initialization message");
		goto nope;
	}
	state.current = co;
	handler = signal(SIGALRM, hung);
	i = alarm(30);
	if (!(s = sfgetr(co->msgfp, '\n', 1)))
	{
		if (errno == EINTR)
			errormsg(lib, ERROR_LIBRARY|ERROR_SYSTEM|2, "identification message read timeout");
		goto nope;
	}
	alarm(i);
	signal(SIGALRM, handler);
	switch (*s)
	{
	case 'o':
		co->flags |= CO_OSH;
		/*FALLTHROUGH*/
	case 'b':
		s = cobinit;
		break;
	case 'k':
		co->flags |= CO_KSH;
		s = cokinit;
		break;
	case 'i':	/* NOTE: 'i' is obsolete */
	case 's':
		co->flags |= CO_SERVER;
		co->pid = 0;
		for (;;)
		{
			if (t = strchr(s, ','))
				*t = 0;
			if (streq(s, CO_OPT_ACK))
				co->mode |= CO_MODE_ACK;
			else if (streq(s, CO_OPT_INDIRECT))
				co->mode |= CO_MODE_INDIRECT;
			if (!(s = t))
				break;
			s++;
		}
		if (!(co->mode & CO_MODE_INDIRECT))
			wait(NiL);
		break;
	default:
		goto nope;
	}
	if (s)
	{
		if (!(cj = coexec(co, s, 0, NiL, NiL, NiL)) || cowait(co, cj) != cj)
		{
			errormsg(lib, ERROR_LIBRARY|ERROR_SYSTEM|2, "initialization message exec error");
			goto nope;
		}
		co->total = 0;
		co->user = 0;
		co->sys = 0;
	}
	co->flags &= ~CO_INIT;
#ifdef SIGCONT
#ifdef SIGTSTP
	signal(SIGTSTP, stop);
#endif
#ifdef SIGTTIN
	signal(SIGTTIN, stop);
#endif
#ifdef SIGTTOU
	signal(SIGTTOU, stop);
#endif
#endif
	co->next = state.coshells;
	state.coshells = co;
	if (!state.init)
	{
		state.init = 1;
		atexit(clean);
	}
	return co;
 bad:
	i = errno;
	close(pio[0]);
	close(pio[1]);
	if (co->msgfp)
		sfclose(co->msgfp);
	else
		close(pex[0]);
	close(pex[1]);
	close(msgfd);
	coclose(co);
	errno = i;
	return 0;
 nope:
	i = errno;
	if (!(s = getenv(CO_ENV_SHELL)) || (s = (t = strrchr(s, '/')) ? (t + 1) : s) && !strmatch(s, "?(k)sh") && !streq(s, CO_ID))
		error(2, "export %s={ksh,sh,%s}", CO_ENV_SHELL, CO_ID);
	coclose(co);
	errno = i;
	return 0;
}
