/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1990-2004 AT&T Corp.                  *
*                      and is licensed under the                       *
*          Common Public License, Version 1.0 (the "License")          *
*                        by AT&T Corp. ("AT&T")                        *
*      Any use, downloading, reproduction or distribution of this      *
*      software constitutes acceptance of the License.  A copy of      *
*                     the License is available at                      *
*                                                                      *
*         http://www.research.att.com/sw/license/cpl-1.0.html          *
*         (with md5 checksum 8a5e0081c856944e76c69a1cf29c2e8b)         *
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
 * coshell library private definitions
 */

#ifndef _COLIB_H
#define _COLIB_H

#define _CO_JOB_PRIVATE_		/* Cojob_t private additions	*/ \
	Cojob_t*	next;		/* next in list			*/ \
	int		pid;		/* pid				*/ \
	char*		out;		/* serialized stdout file	*/ \
	char*		err;		/* serialized stderr file	*/ \
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

#define CO_INIT		(CO_USER>>1)	/* initial command		*/

#define CO_PID_FREE	(-3)		/* free job slot		*/
#define CO_PID_WARPED	(-2)		/* exit before start message	*/
#define CO_PID_ZOMBIE	(-1)		/* ready for wait		*/

#define CO_BUFSIZ	(PATH_MAX/2)	/* temporary buffer size	*/
#define CO_MAXEVAL	(PATH_MAX*8)	/* max eval'd action size	*/
#define CO_MSGFD	3		/* action side msg fd [3..9]	*/

typedef struct				/* global coshell state		*/
{
	Coshell_t*	coshells;	/* list of all coshells		*/
	Coshell_t*	current;	/* current coshell		*/
	char*		pwd;		/* pwd				*/
	char*		sh;		/* sh from first coopen()	*/
	char*		type;		/* CO_ENV_TYPE value		*/
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
