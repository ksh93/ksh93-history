/***************************************************************
*                                                              *
*           This software is part of the ast package           *
*              Copyright (c) 1982-2000 AT&T Corp.              *
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
*              David Korn <dgk@research.att.com>               *
*                                                              *
***************************************************************/
#include	<ast.h>
#include	"shtable.h"
#include	"fault.h"

#if defined(SIGCLD) && !defined(SIGCHLD)
#   define SIGCHLD	SIGCLD
#endif

#define VAL(sig,mode)	((sig+1)|(mode)<<SH_SIGBITS)
#define TRAP(n)		(((n)|SH_TRAP)-1)

#if defined(DEBUGSIG)
#define SH_SIGDEBUG	SH_SIGOFF
#else
#define	SH_SIGDEBUG	SH_SIGDONE
#endif

/*
 * This is a table that gives numbers and default settings to each signal
 * The signal numbers go in the low bits and the attributes go in the high bits
 */

const struct shtable2 shtab_signals[] =
{
#ifdef SIGABRT
	"ABRT",		VAL(SIGABRT,SH_SIGDONE), 	"Abort",
#endif /*SIGABRT */
#ifdef SIGAIO
	"AIO",		VAL(SIGAIO,SH_SIGIGNORE), 	"Asynchronous I/O",
#endif /*SIGAIO */
#ifdef SIGALRM
	"ALRM",		VAL(SIGALRM,SH_SIGFAULT),	"Alarm call",
#endif /* SIGALRM */
#ifdef SIGAPOLLO
	"APOLLO",	VAL(SIGAPOLLO,0),		"SIGAPOLLO",
#endif /* SIGAPOLLO */
#ifdef SIGBUS
	"BUS",		VAL(SIGBUS,SH_SIGDEBUG),	"Bus error",
#endif /* SIGBUS */
#ifdef SIGCHLD
	"CHLD",		VAL(SIGCHLD,SH_SIGFAULT), 	"Death of Child",
#   ifdef SIGCLD
#	if SIGCLD!=SIGCHLD
	    "CLD",	VAL(SIGCLD,SH_SIGFAULT),	"Death of Child",
#	endif
#   endif	/* SIGCLD */
#else
#   ifdef SIGCLD
	"CLD",		VAL(SIGCLD,SH_SIGFAULT),	"Death of Child",
#   endif	/* SIGCLD */
#endif	/* SIGCHLD */
#ifdef SIGCONT
	"CONT",		VAL(SIGCONT,SH_SIGIGNORE),	"Stopped process continued",
#endif	/* SIGCONT */
	"DEBUG",	VAL(TRAP(SH_DEBUGTRAP),0),	"",
#ifdef SIGDIL
	"DIL",		VAL(SIGDIL,0),			"DIL signal",
#endif	/* SIGDIL */
#ifdef SIGEMT
	"EMT",		VAL(SIGEMT,SH_SIGDONE),		"EMT trap",
#endif	/* SIGEMT */
	"ERR",		VAL(TRAP(SH_ERRTRAP),0),	"",
	"EXIT",		VAL(0,0),			"",
	"FPE",		VAL(SIGFPE,SH_SIGDONE),		"Floating exception",
#ifdef SIGFREEZE
	"FREEZE",	VAL(SIGFREEZE,SH_SIGIGNORE),	"Special signal used by CPR",
#endif	/* SIGFREEZE */
	"HUP",		VAL(SIGHUP,SH_SIGDONE),		"Hangup",
	"ILL",		VAL(SIGILL,SH_SIGDEBUG),	"Illegal instruction",
#ifdef JOBS
	"INT",		VAL(SIGINT,SH_SIGINTERACTIVE),	"Interrupt",
#else
	"INT",		VAL(SIGINT,SH_SIGINTERACTIVE),	"",
#endif /* JOBS */
#ifdef SIGIO
	"IO",		VAL(SIGIO,SH_SIGIGNORE),	"IO signal",
#endif	/* SIGIO */
#ifdef SIGIOT
	"IOT",		VAL(SIGIOT,SH_SIGDONE),		"Abort",
#endif	/* SIGIOT */
	"KEYBD",	VAL(TRAP(SH_KEYTRAP),0),	"",
#ifdef SIGKILL
	"KILL",		VAL(SIGKILL,0),			"Killed",
#endif /* SIGKILL */
#ifdef SIGLAB
	"LAB",		VAL(SIGLAB,0),			"Security label changed",
#endif	/* SIGLAB */
#ifdef SIGLOST
	"LOST",		VAL(SIGLOST,SH_SIGDONE),	"Resources lost",
#endif	/* SIGLOST */
#ifdef SIGLWP
	"LWP",		VAL(SIGLWP,SH_SIGIGNORE),	"Special signal used by thread library",
#endif	/* SIGLWP */
#ifdef SIGPHONE
	"PHONE",	VAL(SIGPHONE,0),		"Phone interrupt",
#endif	/* SIGPHONE */
#ifdef SIGPIPE
#ifdef JOBS
	"PIPE",		VAL(SIGPIPE,SH_SIGDONE),	"Broken Pipe",
#else
	"PIPE",		VAL(SIGPIPE,SH_SIGDONE),	 "",
#endif /* JOBS */
#endif /* SIGPIPE */
#ifdef SIGPOLL
	"POLL",		VAL(SIGPOLL,SH_SIGDONE),	"Polling alarm",
#endif	/* SIGPOLL */
#ifdef SIGPROF
	"PROF",		VAL(SIGPROF,SH_SIGDONE), 	"Profiling time alarm",
#endif	/* SIGPROF */
#ifdef SIGPWR
#   if SIGPWR>0
	"PWR",		VAL(SIGPWR,SH_SIGIGNORE),	"Power fail",
#   endif
#endif	/* SIGPWR */
#ifdef SIGQUIT
	"QUIT",		VAL(SIGQUIT,SH_SIGDONE|SH_SIGINTERACTIVE),	"Quit",
#ifdef _SIGRTMIN
	"RTMIN",	VAL(_SIGRTMIN,0),		"Lowest priority realtime signal",
#else
#   ifdef SIGRTMIN
	"RTMIN",	VAL(SIGRTMIN,0),		"Lowest priority realtime signal",
#   endif /* SIGRTMIN */
#endif	/* _SIGRTMIN */
#ifdef _SIGRTMAX
	"RTMAX",	VAL(_SIGRTMAX,0),		"Highest priority realtime signal",
#else
#   ifdef SIGRTMAX
	"RTMAX",	VAL(SIGRTMAX,0),		"Highest priority realtime signal",
#   endif /* SIGRTMAX */
#endif	/* _SIGRTMAX */
#endif	/* SIGQUIT */
	"SEGV",		VAL(SIGSEGV,0),			"Memory fault",
#ifdef SIGSTOP
	"STOP",		VAL(SIGSTOP,0),			"Stopped (SIGSTOP)",
#endif	/* SIGSTOP */
#ifdef SIGSYS
	"SYS",		VAL(SIGSYS,SH_SIGDONE),		"Bad system call",
#endif	/* SIGSYS */
	"TERM",		VAL(SIGTERM,SH_SIGDONE|SH_SIGINTERACTIVE),	"Terminated",
#ifdef SIGTINT
#   ifdef JOBS
	"TINT",		VAL(SIGTINT,0),			"Interrupt",
#   else
	"TINT",		VAL(SIGTINT,0),			"".
#   endif /* JOBS */
#endif	/* SIGTINT */
#ifdef SIGTRAP
	"TRAP",		VAL(SIGTRAP,SH_SIGDONE),	"Trace/BPT trap",
#endif	/* SIGTRAP */
#ifdef SIGTSTP
	"TSTP",		VAL(SIGTSTP,0),			"Stopped",
#endif	/* SIGTSTP */
#ifdef SIGTTIN
	"TTIN",		VAL(SIGTTIN,0),			"Stopped (SIGTTIN)",
#endif	/* SIGTTIN */
#ifdef SIGTTOU
	"TTOU",		VAL(SIGTTOU,0),			"Stopped(SIGTTOU)",
#endif	/* SIGTTOU */
#ifdef SIGURG
	"URG",		VAL(SIGURG,SH_SIGIGNORE),	"Socket interrupt",
#endif	/* SIGURG */
#ifdef SIGUSR1
	"USR1",		VAL(SIGUSR1,SH_SIGDONE),	 "User signal 1",
#endif	/* SIGUSR1 */
#ifdef SIGUSR2
	"USR2",		VAL(SIGUSR2,SH_SIGDONE),	 "User signal 2",
#endif	/* SIGUSR2 */
#ifdef SIGVTALRM
	"VTALRM",	VAL(SIGVTALRM,SH_SIGDONE),	"Virtual time alarm",
#endif	/* SIGVTALRM */
#ifdef SIGWINCH
	"WINCH",	VAL(SIGWINCH,SH_SIGIGNORE),	"Window size change",
#endif	/* SIGWINCH */
#ifdef SIGWINDOW
	"WINDOW",	VAL(SIGWINDOW,SH_SIGIGNORE),	"Window size change",
#endif	/* SIGWINDOW */
#ifdef SIGWIND
	"WIND",		VAL(SIGWIND,SH_SIGIGNORE),	"Window size change",
#endif	/* SIGWIND */
#ifdef SIGMIGRATE
	"MIGRATE",		VAL(SIGMIGRATE,0),	"Migrate process",
#endif	/* SIGMIGRATE */
#ifdef SIGDANGER
	"DANGER",		VAL(SIGDANGER,0),	"System crash soon",
#endif	/* SIGDANGER */
#ifdef SIGSOUND
	"SOUND",		VAL(SIGSOUND,0),	"Sound completed",
#endif	/* SIGSOUND */
#ifdef SIGTHAW
	"THAW",			VAL(SIGTHAW,SH_SIGIGNORE),	"Special signal used by CPR",
#endif	/* SIGTHAW */
#ifdef SIGWAITING
	"WAITING",		VAL(SIGWAITING,SH_SIGIGNORE),	"All threads blocked",
#endif	/* SIGWAITING */
#ifdef SIGXCPU
	"XCPU",		VAL(SIGXCPU,SH_SIGDONE|SH_SIGINTERACTIVE),	"Exceeded CPU time limit",
#endif	/* SIGXCPU */
#ifdef SIGXFSZ
	"XFSZ",		VAL(SIGXFSZ,SH_SIGDONE|SH_SIGINTERACTIVE),	"Exceeded file size limit",
#endif	/* SIGXFSZ */
	"",	0,	0
};
