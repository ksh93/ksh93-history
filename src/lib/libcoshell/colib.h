/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1990-2000 AT&T Corp.                *
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
*                                                                  *
*******************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Research
 *
 * coshell library private definitions
 */

#ifndef _COLIB_H
#define _COLIB_H

#define _CO_JOB_PRIVATE_		/* Cojob_t private additions	*/ \
	Cojob_t*	next;		/* next in list			*/ \
	int		pid;		/* pid				*/ \
					/* end of private additions	*/

#define _CO_SHELL_PRIVATE_		/* Coshell_t private additions	*/ \
	Coshell_t*	next;		/* next in list			*/ \
	Cojob_t*	jobs;		/* job list			*/ \
	int		cmdfd;		/* command pipe fd		*/ \
	int		mask;		/* CO_* flags to clear		*/ \
	int		mode;		/* connection modes		*/ \
	int		pid;		/* pid				*/ \
	int		slots;		/* number of job slots		*/ \
					/* end of private additions	*/

#include <ast.h>
#include <coshell.h>
#include <errno.h>
#include <sig.h>
#include <sfstr.h>
#include <wait.h>

#define state		_coshell_info_	/* hide external symbol		*/

#define CO_MODE_ACK		(1<<0)	/* wait for coexec() ack	*/
#define CO_MODE_INDIRECT	(1<<1)	/* indirect CO_SERVER		*/

#define CO_DEVFD	(CO_USER>>1)	/* coshell handles /dev/fd/#	*/
#define CO_INIT		(CO_USER>>2)	/* initial command		*/

#define CO_PID_FREE	(-3)		/* free job slot		*/
#define CO_PID_WARPED	(-2)		/* exit before start message	*/
#define CO_PID_ZOMBIE	(-1)		/* ready for wait		*/

#define CO_BUFSIZ	(PATH_MAX/2)	/* temporary buffer size	*/
#define CO_MAXEVAL	(PATH_MAX*8)	/* max eval'd action size	*/
#define CO_MSGFD	3		/* action side msg fd		*/

typedef struct				/* global coshell state		*/
{
	Coshell_t*	coshells;	/* list of all coshells		*/
	Coshell_t*	current;	/* current coshell		*/
	char*		pwd;		/* pwd				*/
	char*		sh;		/* sh from first coopen()	*/
	int		init;		/* 0 if first coopen()		*/
} Costate_t;

extern char 		coident[];	/* coshell ident script		*/
extern char 		cobinit[];	/* bsh initialition script	*/
extern char 		cokinit[];	/* ksh initialition script	*/
extern char* 		coexport[];	/* default export var list	*/

extern Costate_t	state;		/* global coshell info		*/

#ifndef errno
extern int		errno;
#endif

#endif
