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
 * posix syslog interface definitions
 */

#ifndef _SYSLOG_H
#define _SYSLOG_H

#include <stdarg.h>

/* syslog priority severity levels */

#define LOG_EMERG	0	/* panic condition			*/
#define LOG_ALERT	1	/* should be corrected immediately	*/
#define LOG_CRIT	2	/* critical condition			*/
#define LOG_ERR		3	/* error condition			*/
#define LOG_WARNING	4	/* warning condition			*/
#define LOG_NOTICE	5	/* no error but may need intervention	*/
#define LOG_INFO	6	/* informational message		*/
#define LOG_DEBUG	7	/* debug message			*/

/* setlogmask masks */

#define	LOG_MASK(s)	(1<<(s))	/* individual severity s	*/
#define	LOG_UPTO(s)	((1<<((s)+1))-1)/* up to and including s	*/

/* syslog facilities */

#define LOG_USER	(1<<3)	/* random user process -- default	*/
#define LOG_KERN	(2<<3)	/* kernel				*/
#define LOG_MAIL	(3<<3)	/* mail					*/
#define LOG_DAEMON	(4<<3)	/* daemon				*/
#define LOG_AUTH	(5<<3)	/* security/authorization		*/
#define LOG_SYSLOG	(6<<3)	/* syslog internal			*/
#define LOG_LPR		(7<<3)	/* line printer				*/
#define LOG_NEWS	(8<<3)	/* network news				*/
#define LOG_UUCP	(9<<3)	/* uucp					*/
#define LOG_CRON	(10<<3)	/* cron					*/
#define LOG_AUDIT	(11<<3)	/* audit daemon				*/
#define LOG_LFMT	(12<<3)	/* logalert				*/
#define LOG_SYSTEM2	(13<<3)	/* reserved for system use		*/
#define LOG_SYSTEM1	(14<<3)	/* reserved for system use		*/
#define LOG_SYSTEM0	(15<<3)	/* reserved for system use		*/
#define LOG_LOCAL0	(16<<3)	/* reserved for local use		*/
#define LOG_LOCAL1	(17<<3)	/* reserved for local use		*/
#define LOG_LOCAL2	(18<<3)	/* reserved for local use		*/
#define LOG_LOCAL3	(19<<3)	/* reserved for local use		*/
#define LOG_LOCAL4	(20<<3)	/* reserved for local use		*/
#define LOG_LOCAL5	(21<<3)	/* reserved for local use		*/
#define LOG_LOCAL6	(22<<3)	/* reserved for local use		*/
#define LOG_LOCAL7	(23<<3)	/* reserved for local use		*/

/* openlog flags */

#define	LOG_PID		0x01	/* log the pid with each message	*/
#define	LOG_CONS	0x02	/* log to console if errors in sending	*/
#define LOG_NDELAY	0x08	/* open right now			*/
#define	LOG_ODELAY	0x04	/* delay open until syslog() is called	*/
#define LOG_NOWAIT	0x10	/* don't wait() for any child processes	*/
#define LOG_PERROR	0x20	/* log to stderr too			*/

#ifdef LOG_TABLES

/* encoding support */

#include <namval.h>

#define log_facility	_log_facility
#define log_severity	_log_severity

#define LOG_FACILITY(p)	(((p)&03770)>>3)/* get facility index from pri	*/
#define LOG_SEVERITY(p)	((p)&00007)	/* get severity from pri	*/

#if _BLD_ast && defined(__EXPORT__)
#define extern		__EXPORT__
#endif
#if !_BLD_ast && defined(__IMPORT__)
#define extern		__IMPORT__
#endif

extern const Namval_t	log_facility[];
extern const Namval_t	log_severity[];

#undef	extern

#endif

#if _BLD_ast && defined(__EXPORT__)
#define extern		__EXPORT__
#endif

extern void	closelog(void);
extern void	openlog(const char*, int, int);
extern int	setlogmask(int);
extern void	syslog(int, const char*, ...);
extern void	vsyslog(int, const char*, va_list);

#undef	extern

#endif
