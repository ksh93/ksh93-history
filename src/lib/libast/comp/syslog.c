/***************************************************************
*                                                              *
*           This software is part of the ast package           *
*              Copyright (c) 1985-2000 AT&T Corp.              *
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
*              David Korn <dgk@research.att.com>               *
*               Phong Vo <kpv@research.att.com>                *
*                                                              *
***************************************************************/
#pragma prototyped
/*
 * syslog implementation
 */

#include <ast.h>

#if _lib_syslog

NoN(syslog)

#else

#define LOG_TABLES

#include "sysloglib.h"

#include <sfstr.h>
#include <error.h>
#include <tm.h>

Syslog_state_t		log = { 0, LOG_USER, -1, 0, ~0 };

static const Namval_t	attempt[] =
{
#if 0
	"/dev/tcp/share/syslog",	0,
	"/dev/tcp/local/syslog",	0,
#endif
	"lib/syslog/log",		0,
	"/dev/console",			LOG_CONS,
};

const Namval_t		log_facility[] =
{
	"default",	0,
	"user",		LOG_USER,
	"kernel",	LOG_KERN,
	"mail",		LOG_MAIL,
	"daemon",	LOG_DAEMON,
	"security",	LOG_AUTH,
	"syslog",	LOG_SYSLOG,
	"lpr",		LOG_LPR,
	"news",		LOG_NEWS,
	"uucp",		LOG_UUCP,
	"cron",		LOG_CRON,
	"audit",	LOG_AUDIT,
	"logalert",	LOG_LFMT,
	"system2",	LOG_SYSTEM2,
	"system1",	LOG_SYSTEM1,
	"system0",	LOG_SYSTEM0,
	0,		0
};

const Namval_t		log_severity[] =
{
	"panic",	LOG_EMERG,
	"alert",	LOG_ALERT,
	"critical",	LOG_CRIT,
	"error",	LOG_ERR,
	"warning",	LOG_WARNING,
	"notice",	LOG_NOTICE,
	"info",		LOG_INFO,
	"debug",	LOG_DEBUG,
	0,		0
};

void
sendlog(const char* msg)
{
	register char*		s;
	register Namval_t*	p;
	register int		n;

	n = msg ? strlen(msg) : 0;
	for (;;)
	{
		if (log.fd < 0)
		{
			char	buf[PATH_MAX];

			if (log.attempt >= elementsof(attempt))
				break;
			p = (Namval_t*)&attempt[log.attempt++];
			if (p->value && !(p->value & log.flags))
				continue;
			if (*(s = p->name) != '/' && !(s = pathpath(buf, s, "", PATH_REGULAR|PATH_READ)))
				continue;
			if ((log.fd = open(s, O_CREAT|O_WRONLY|O_APPEND|O_NOCTTY)) < 0)
				continue;
			fcntl(log.fd, F_SETFD, FD_CLOEXEC);
		}
		if (!n || write(log.fd, msg, n) > 0)
			break;
		close(log.fd);
		log.fd = -1;
	}
	if (n && (log.flags & LOG_PERROR))
		write(2, msg, n);
}

static int
extend(Sfio_t* sp, void* vp, Sffmt_t* dp)
{
	if (dp->fmt == 'm')
	{
		dp->flags |= SFFMT_VALUE;
		dp->fmt = 's';
		dp->size = -1;
		*((char**)vp) = fmterror(errno);
	}
	return 0;
}

void
vsyslog(int priority, const char* format, va_list ap)
{
	register int	c;
	register char*	s;
	Sfio_t*		sp;
	Sffmt_t		fmt;
	char		buf[16];

	if (!LOG_FACILITY(priority))
		priority |= log.facility;
	if (!(priority & log.mask))
		return;
	if (sp = sfstropen())
	{
		sfputr(sp, fmttime("%b %d %H:%M:%S", time(NiL)), -1);
		if ((c = LOG_SEVERITY(priority)) < elementsof(log_severity))
			s = (char*)log_severity[c].name;
		else
			sfsprintf(s = buf, sizeof(buf), "debug%d", c);
		sfprintf(sp, " %-8s ", s);
		if ((c = LOG_FACILITY(priority)) < elementsof(log_facility))
			s = (char*)log_facility[c].name;
		else
			sfsprintf(s = buf, sizeof(buf), "local%d", c);
		sfprintf(sp, " %-8s ", s);
		if (log.flags & LOG_PID)
			sfprintf(sp, "%05d ", getpid());
		if (log.ident)
			sfprintf(sp, "%s: ", log.ident);
		if (format)
		{
			memset(&fmt, 0, sizeof(fmt));
			fmt.version = SFIO_VERSION;
			fmt.form = (char*)format;
			fmt.extf = extend;
			va_copy(fmt.args, ap);
			sfprintf(sp, "%!", &fmt);
		}
		sfputc(sp, '\n');
		sendlog(sfstruse(sp));
		sfstrclose(sp);
	}
}

void
syslog(int priority, const char* format, ...)
{
	va_list		ap;

	va_start(ap, format);
	vsyslog(priority, format, ap);
	va_end(ap);
}

#endif
