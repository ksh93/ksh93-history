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
 * Glenn Fowler
 * AT&T Research
 *
 * command line option parse interface
 */

#ifndef _OPTION_H
#define _OPTION_H

#include <ast.h>

#define OPT_VERSION	20000101L

#define opt_info	_opt_info_

#define OPT_USER	(1L<<16)	/* first user flag bit		*/

struct Opt_s;
struct Optdisc_s;

typedef int (*Optinfo_f)(struct Opt_s*, Sfio_t*, const char*, struct Optdisc_s*);

typedef struct Optdisc_s
{
	unsigned long	version;	/* OPT_VERSION			*/
	unsigned long	flags;		/* OPT_* flags			*/
	char*		dictionary;	/* error dictionary id		*/
	Optinfo_f	infof;		/* runtime info function	*/
} Optdisc_t;

/* NOTE: Opt_t member order fixed by a previous binary release */

typedef struct Opt_s
{
	int		again;		/* see optjoin()		*/
	char*		arg;		/* {:,#} string argument	*/
	char**		argv;		/* most recent argv		*/
	int		index;		/* argv index			*/
	char*		msg;		/* error/usage message buffer	*/
	long		num;		/* # numeric argument		*/
	int		offset;		/* char offset in argv[index]	*/
	char		option[8];	/* current flag {-,+} + option  */
	char		name[64];	/* current long name or flag	*/
	Optdisc_t*	disc;		/* user discipline		*/

#ifdef _OPT_PRIVATE
	_OPT_PRIVATE
#endif

} Opt_t;

#if _BLD_ast && defined(__EXPORT__)
#define __PUBLIC_DATA__		__EXPORT__
#else
#if !_BLD_ast && defined(__IMPORT__)
#define __PUBLIC_DATA__		__IMPORT__
#else
#define __PUBLIC_DATA__
#endif
#endif

extern __PUBLIC_DATA__ Opt_t		opt_info;

#undef	__PUBLIC_DATA__

#if _BLD_ast && defined(__EXPORT__)
#define extern		__EXPORT__
#endif

extern int		optget(char**, const char*);
extern int		optjoin(char**, ...);
extern char*		opthelp(const char*, const char*);
extern char*		optusage(const char*);
extern int		optstr(const char*, const char*);

#undef	extern

#endif
