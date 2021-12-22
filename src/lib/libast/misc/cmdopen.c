/*
 * CDE - Common Desktop Environment
 *
 * Copyright (c) 1993-2012, The Open Group. All rights reserved.
 *
 * These libraries and programs are free software; you can
 * redistribute them and/or modify them under the terms of the GNU
 * Lesser General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * These libraries and programs are distributed in the hope that
 * they will be useful, but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with these libraries and programs; if not, write
 * to the Free Software Foundation, Inc., 51 Franklin Street, Fifth
 * Floor, Boston, MA 02110-1301 USA
 */
/***************************************************************
*                                                              *
*                      AT&T - PROPRIETARY                      *
*                                                              *
*        THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF        *
*                    AT&T BELL LABORATORIES                    *
*         AND IS NOT TO BE DISCLOSED OR USED EXCEPT IN         *
*            ACCORDANCE WITH APPLICABLE AGREEMENTS             *
*                                                              *
*          Copyright (c) 1993 AT&T Bell Laboratories           *
*              Unpublished & Not for Publication               *
*                     All Rights Reserved                      *
*                                                              *
*       The copyright notice above does not evidence any       *
*      actual or intended publication of such source code      *
*                                                              *
*               This software was created by the               *
*           Software Engineering Research Department           *
*                    AT&T Bell Laboratories                    *
*                                                              *
*               For further information contact                *
*                   advsoft@research.att.com                   *
*                 Randy Hackbarth 908-582-5245                 *
*                  Dave Belanger 908-582-7427                  *
*                                                              *
***************************************************************/

/* : : generated by proto : : */

#line 1

#if !defined(__PROTO__)
#if defined(__STDC__) || defined(__cplusplus) || defined(_proto) || defined(c_plusplus)
#if defined(__cplusplus)
#define __MANGLE__	"C"
#else
#define __MANGLE__
#endif
#define __STDARG__
#define __PROTO__(x)	x
#define __OTORP__(x)
#define __PARAM__(n,o)	n
#if !defined(__STDC__) && !defined(__cplusplus)
#if !defined(c_plusplus)
#define const
#endif
#define signed
#define void		int
#define volatile
#define __V_		char
#else
#define __V_		void
#endif
#else
#define __PROTO__(x)	()
#define __OTORP__(x)	x
#define __PARAM__(n,o)	o
#define __MANGLE__
#define __V_		char
#define const
#define signed
#define void		int
#define volatile
#endif
#if defined(__cplusplus) || defined(c_plusplus)
#define __VARARG__	...
#else
#define __VARARG__
#endif
#define __VOID__	__V_
#if defined(__STDARG__)
#define __VA_START__(p,a)	va_start(p,a)
#else
#define __VA_START__(p,a)	va_start(p)
#endif
#endif

#line 14
#define DEBUG_EXEC	1

#include "cmdlib.h"

#if _lib_socketpair
#if _sys_socket
#include <sys/types.h>
#include <sys/socket.h>
#else
#undef	_lib_socketpair
#endif
#endif

#define CMD_ARGMOD	(1<<0)
#define CMD_DAEMON	(1<<1)
#define CMD_ENVCLEAR	(1<<2)
#define CMD_OVERLAY	(1<<3)
#define CMD_PARANOID	(1<<4)
#define CMD_REALGID	(1<<5)
#define CMD_REALUID	(1<<6)
#define CMD_SUPER	(1<<7)

struct cmdinfo*		cmds;		/* open cmd list		*/

extern __MANGLE__ char**		environ;

#if DEBUG_EXEC

#include <namval.h>

#define EX_ENV_OPTIONS	"EXEC_OPTIONS"

#define EX_ENVIRONMENT	(1<<0)
#define EX_EXEC		(1<<1)
#define EX_TRACE	(1<<2)
#define EX_VERBOSE	(1<<3)

static const Namval_t		options[] =
{
	"environment",	EX_ENVIRONMENT,
	"exec",		EX_EXEC,
	"trace",	EX_TRACE,
	"verbose",	EX_VERBOSE,
	0,		0
};

/*
 * called by stropt() to set options
 */

static int
setopt __PARAM__((__V_* a, const __V_* p, int n, const char* v), (a, p, n, v)) __OTORP__(__V_* a; const __V_* p; int n; const char* v;)
#line 66
{
	NoP(v);
	if (p)
	{
		if (n) *((int*)a) |= ((Namval_t*)p)->value;
		else *((int*)a) &= ~((Namval_t*)p)->value;
	}
	return(0);
}

#endif

/*
 * redirect nfd to fd
 * nfd==-1 means close(fd)
 */

static int
redirect __PARAM__((int nfd, int fd), (nfd, fd)) __OTORP__(int nfd; int fd;)
#line 85
{
	if (nfd != fd)
	{
		close(fd);
		if (nfd >= 0 && dup2(nfd, fd) != fd) return(-1);
	}
	return(0);
}

/*
 * fork and exec cmd(argv) according to mode combinations:
 *
 *	a	argv[-1] and argv[0] can be modified
 *	b	bidirectional cmd pipe fd 0/1 returned
 *	e	clear environment
 *	g	setgid(getgid())
 *	o	overlay -- no child process
 *	p	paranoid
 *	r	cmd pipe fd 1 returned
 *	s	if (geteuid()==0) setuid(0), setgid(getegid())
 *	w	cmd pipe fd 0 returned
 *	u	setuid(getuid())
 *
 * pipe not used when r and w are omitted (no io on return fd)
 * cmd==0 names the current shell
 * envv is the child environment
 * redv is the child redirection vector
 *      even elements duped from the next element
 *	-1 even terminates the list
 *	-1 odd element means close
 */

int
cmdopen __PARAM__((const char* cmd, char** argv, char** envv, int* redv, const char* mode), (cmd, argv, envv, redv, mode)) __OTORP__(const char* cmd; char** argv; char** envv; int* redv; const char* mode;)
#line 119
{
	struct cmdinfo*	proc;
	int			cmdfd = -1;
	char**			p;
	char**				v;
	int*				r;
	int				flags = 0;
	long				pos;
	char				path[PATH_MAX];
	char				env[PATH_MAX + 2];
	int				pio[2];
#if DEBUG_EXEC
	int				debug = EX_EXEC;
#endif

	if (cmd && !pathpath(path, cmd, NiL, PATH_REGULAR|PATH_EXECUTE)) return(-1);
	if (mode) while (*mode) switch (*mode++)
	{
	case 'a':
		flags |= CMD_ARGMOD;
		break;
	case 'b':
		cmdfd = 3;
		break;
	case 'd':
		flags |= CMD_DAEMON;
		break;
	case 'e':
		flags |= CMD_ENVCLEAR;
		break;
	case 'g':
		flags |= CMD_REALGID;
		break;
	case 'o':
		flags |= CMD_OVERLAY;
		break;
	case 'p':
		flags |= CMD_PARANOID;
		break;
	case 'r':
		if (cmdfd != 3) cmdfd = (cmdfd == 0) ? 3 : 1;
		break;
	case 's':
		if (!geteuid()) flags |= CMD_SUPER;
		break;
	case 'u':
		flags |= CMD_REALUID;
		break;
	case 'w':
		if (cmdfd != 3) cmdfd = (cmdfd == 1) ? 3 : 0;
		break;
	default:
		return(-1);
	}
	for (proc = cmds; proc; proc = proc->next)
		if (proc->fd < 0) break;
	if (!proc)
	{
		if (!(proc = newof(0, struct cmdinfo, 1, 0))) return(-1);
		proc->fd = -2;
		proc->next = cmds;
		cmds = proc;
	}
	if (!putenv(NiL)) return(-1);
	if (cmdfd >= 0)
	{
#if _lib_socketpair
		if ((cmdfd & 02) && socketpair(AF_UNIX, SOCK_STREAM, 0, pio)) return(-1);
		else
#endif
		if (pipe(pio)) return(-1);
	}
	else if ((proc->fd = dup(0)) < 0) return(-1);
	sfsync(sfstdout);
	sfsync(sfstderr);
	if (!(flags & CMD_OVERLAY))
	{
		sigcritical(1);
		proc->pid = fork();
		sigcritical(0);
	}
	else proc->pid = 0;
	switch (proc->pid)
	{
	case -1:
		if (cmdfd >= 0)
		{
			close(pio[0]);
			close(pio[1]);
		}
		else
		{
			close(proc->fd);
			proc->fd = -2;
		}
		return(-1);
	case 0:
#if DEBUG_EXEC
		stropt(getenv(EX_ENV_OPTIONS), options, sizeof(*options), setopt, &debug);
		if (debug & EX_TRACE)
		{
			if (!fork())
			{
				sfsprintf(path, sizeof(path), "%d", getppid());
				execlp("trace", "trace", "-p", path, NiL);
				_exit(EXIT_NOTFOUND);
			}
			sleep(2);
		}
#endif
		if (flags & CMD_DAEMON)
		{
			setsid();
			signal(SIGINT, SIG_IGN);
			signal(SIGTERM, SIG_DFL);
#ifdef SIGTTIN
			signal(SIGTTIN, SIG_IGN);
#endif
#ifdef SIGTTOU
			signal(SIGTTOU, SIG_IGN);
#endif
		}
		if (flags & CMD_SUPER)
		{
			setuid(geteuid());
			setgid(getegid());
		}
		if (flags & (CMD_PARANOID|CMD_REALGID)) setgid(getgid());
		if (flags & (CMD_PARANOID|CMD_REALUID)) setuid(getuid());
		if ((pos = sftell(sfstdin)) != lseek(sffileno(sfstdin), 0L, 1))
			lseek(sffileno(sfstdin), pos, 0);
		if (cmdfd >= 0)
		{
			if (cmdfd & 02)
			{
				cmdfd &= 01;
				if (redirect(pio[cmdfd], !cmdfd)) goto bad;
			}
			if (redirect(pio[cmdfd], cmdfd)) goto bad;
			close(pio[0]);
			close(pio[1]);
		}
		else close(proc->fd);
		if (r = redv) while (*r >= 0)
		{
			if (redirect(*(r + 1), *r)) goto bad;
			r += 2;
		}
		if (flags & CMD_ENVCLEAR) environ = 0;
		env[0] = '_';
		env[1] = '=';
		if (!putenv(env)) goto bad;
		if ((flags & CMD_PARANOID) && !putenv("PATH=:/bin:/usr/bin")) goto bad;
		if (p = envv)
			while (*p)
				if (!putenv(*p++)) goto bad;
#if DEBUG_EXEC
		if (!(debug & EX_EXEC) || (debug & EX_VERBOSE))
		{
			if ((debug & EX_ENVIRONMENT) && (p = environ))
				while (*p)
					sfprintf(sfstderr, "%s\n", *p++);
			sfprintf(sfstderr, "+ %s", cmd ? path : "sh");
			if ((p = argv) && *p)
				while (*++p) sfprintf(sfstderr, " %s", *p);
			sfprintf(sfstderr, "\n");
			if (!(debug & EX_EXEC)) _exit(0);
		}
#endif
		p = argv;
		if (cmd)
		{
			strcpy(env + 2, path);
			execv(path, p);
			if (errno != ENOEXEC) goto bad;

			/*
			 * try cmd as a shell script
			 */

			if (!(flags & CMD_ARGMOD))
			{
				while (*p++);
				if (!(v = newof(0, char*, p - argv + 2, 0))) goto bad;
				p = v + 2;
				if (*argv) argv++;
				while (*p++ = *argv++);
				p = v + 1;
			}
			*p = path;
			*--p = "sh";
		}
		if (!(flags & CMD_PARANOID))
		{
			strcpy(env + 2, getshell());
			execv(env + 2, p);
		}
		strcpy(env + 2, "/bin/sh");
		execv(env + 2, p);
	bad:
		if (!(flags & CMD_OVERLAY)) _exit(EXIT_NOTFOUND);
		return(-1);
	default:
		if (cmdfd >= 0)
		{
			close(pio[cmdfd & 01]);
			proc->fd = pio[!cmdfd];
		}
		return(proc->fd);
	}
}
