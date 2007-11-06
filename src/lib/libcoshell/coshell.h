/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1990-2007 AT&T Intellectual Property          *
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
 * coshell library interface
 */

#ifndef _COSHELL_H
#define _COSHELL_H

#include <ast.h>

#if !_BLD_coshell

#undef	procrun
#define procrun(a,b,c)		coprocrun(a,b,c)
#undef	system
#define system(a)		cosystem(a)

#endif

struct Coshell_s; typedef struct Coshell_s Coshell_t;
struct Cojob_s; typedef struct Cojob_s Cojob_t;

/*
 * DEPRECATED names for compatibility
 */

#define COSHELL		Coshell_t
#define COJOB		Cojob_t

#define CO_ID		"coshell"	/* library/command id		*/

#define CO_ENV_ATTRIBUTES "COATTRIBUTES"/* coshell attributes env var	*/
#define CO_ENV_EXPORT	"COEXPORT"	/* coshell env var export list	*/
#define CO_ENV_HOST	"HOSTNAME"	/* coshell host name env var	*/
#define CO_ENV_MSGFD	"_COSHELL_msgfd"/* msg fd			*/
#define CO_ENV_OPTIONS	"COSHELL_OPTIONS"/* options environment var	*/
#define CO_ENV_PROC	"NPROC" 	/* concurrency environment var	*/
#define CO_ENV_SHELL	"COSHELL"	/* coshell path environment var	*/
#define CO_ENV_TEMP	"COTEMP"	/* 10 char temp file base	*/
#define CO_ENV_TYPE	"HOSTTYPE"	/* coshell host type env var	*/

#define CO_OPT_ACK	"ack"		/* wait for server coexec() ack	*/
#define CO_OPT_INDIRECT	"indirect"	/* indirect server connection	*/
#define CO_OPT_SERVER	"server"	/* server connection		*/

#define CO_QUANT	100		/* time quanta per sec		*/

#define CO_ANY		(1<<0)		/* return any open coshell	*/
#define CO_DEBUG	(1<<1)		/* library debug trace		*/
#define CO_EXPORT	(1<<2)		/* export everything		*/
#define CO_IGNORE	(1<<3)		/* ignore command errors	*/
#define CO_LOCAL	(1<<4)		/* local affinity		*/
#define CO_NONBLOCK	(1<<5)		/* don't block coexec if Q full	*/
#define CO_SILENT	(1<<6)		/* don't trace commands		*/

#define CO_KSH		(1<<7)		/* coshell is ksh (readonly)	*/
#define CO_SERVER	(1<<8)		/* coshell is server (readonly)	*/
#define CO_OSH		(1<<9)		/* coshell is OLD (readonly)	*/

#define CO_CROSS	(1<<10)		/* don't prepend local dirs	*/
#define CO_DEVFD	(1<<11)		/* coshell handles /dev/fd/#	*/

#define CO_SERIALIZE	(1<<12)		/* serialize stdout and stderr	*/
#define CO_SERVICE	(1<<13)		/* service callouts		*/

#define CO_APPEND	(1<<14)		/* append coexec() out/err	*/
#define CO_SEPARATE	(1L<<15)	/* 1 shell+wait per coexec()	*/

#define CO_USER		(1L<<16)	/* first user flag		*/

struct Cojob_s				/* coshell job info		*/
{
	int		id;		/* job id			*/
	int		status;		/* exit status			*/
	int		flags;		/* CO_* flags			*/
	void*		local;		/* local info			*/
	unsigned long	user;		/* user time in 1/CO_QUANT secs	*/
	unsigned long	sys;		/* sys time in 1/CO_QUANT secs	*/
#ifdef _CO_JOB_PRIVATE_
	_CO_JOB_PRIVATE_		/* library private additions	*/
#endif
};

struct Coshell_s			/* coshell connection info	*/
{
	int		flags;		/* flags			*/
	int		outstanding;	/* number of outstanding jobs	*/
	int		running;	/* number of running jobs	*/
	int		total;		/* number of coexec() jobs	*/
	unsigned long	user;		/* user time in 1/CO_QUANT secs	*/
	unsigned long	sys;		/* sys time in 1/CO_QUANT secs	*/
	Sfio_t*		msgfp;		/* message stream for sfpoll()	*/
#ifdef _CO_SHELL_PRIVATE_
	_CO_SHELL_PRIVATE_		/* library private additions	*/
#endif
};

extern int		coclose(Coshell_t*);
extern Cojob_t*		coexec(Coshell_t*, const char*, int, const char*, const char*, const char*);
extern char*		coinit(int);
extern int		cokill(Coshell_t*, Cojob_t*, int);
extern Coshell_t*	coopen(const char*, int, const char*);
extern void		coquote(Sfio_t*, const char*, int);
extern int		cosync(Coshell_t*, const char*, int, int);
extern Cojob_t*		cowait(Coshell_t*, Cojob_t*);

extern int		cojobs(Coshell_t*);
extern int		copending(Coshell_t*);
extern int		cozombie(Coshell_t*);

extern int		coprocrun(const char*, char**, int);
extern int		cosystem(const char*);

#endif
