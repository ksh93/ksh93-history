/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1985-2001 AT&T Corp.                *
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
*                David Korn <dgk@research.att.com>                 *
*                 Phong Vo <kpv@research.att.com>                  *
*******************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Research
 *
 * command line option parser and usage formatter private definitions
 */

#ifndef _OPTLIB_H
#define _OPTLIB_H

#include <ast.h>
#include <cdt.h>
#include <sfstr.h>

#define OPT_functions		0x01
#define OPT_ignore		0x02
#define OPT_long		0x04
#define OPT_old			0x08
#define OPT_proprietary		0x10

struct Optdisc_s;

typedef struct
{
	struct Optdisc_s*	disc;
	char*			opts;
	char*			oopts;
	char*			catalog;
	unsigned char		version;
	unsigned char		prefix;
	unsigned char		flags;
	unsigned char		section;
} Optpass_t;

typedef struct
{
	char*			argv[3];
	char*			str;
	int			colon;
} Optstr_t;

typedef struct
{
	Sfio_t*		mp;		/* opt_info.msg string stream	*/
	Sfio_t*		xp;		/* translation string stream	*/
	Optpass_t	pass[8];	/* optjoin() list		*/
	char*		argv[2];	/* initial argv copy		*/
	char*		strv[3];	/* optstr() argv		*/
	char*		str;		/* optstr() string		*/
	Sfio_t*		strp;		/* optstr() stream		*/
	int		colon;		/* optstr : state		*/
	int		force;		/* force this style		*/
	int		pindex;		/* prev index for backup	*/
	int		poffset;	/* prev offset for backup	*/
	int		npass;		/* # optjoin() passes		*/
	int		join;		/* optjoin() pass #		*/
	int		plus;		/* + ok				*/
	int		style;		/* default opthelp() style	*/
	int		width;		/* format line width		*/
	int		flags;		/* display flags		*/
	int		emphasis;	/* ansi term emphasis ok	*/
	Dtdisc_t	msgdisc;	/* msgdict discipline		*/
	Dt_t*		msgdict;	/* default ast.id catalog msgs	*/
} Optstate_t;

#define _OPT_PRIVATE_	Optstate_t* state;

#include <error.h>

#endif
