/***************************************************************
*                                                              *
*           This software is part of the ast package           *
*              Copyright (c) 1992-2000 AT&T Corp.              *
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
*                                                              *
***************************************************************/
#pragma prototyped
/*
 * AT&T Research
 *
 * builtin cmd definitions
 */

#ifndef _CMD_H
#define _CMD_H

#include <ast.h>
#include <error.h>
#include <stak.h>

#if _BLD_cmd && defined(__EXPORT__)
#define extern		__EXPORT__
#endif

extern int	b_basename(int, char**, void*);
extern int	b_cat(int, char**, void*);
extern int	b_chgrp(int, char**, void*);
extern int	b_chmod(int, char**, void*);
extern int	b_chown(int, char**, void*);
extern int	b_cmp(int, char**, void*);
extern int	b_comm(int, char**, void*);
extern int	b_cp(int, char**, void*);
extern int	b_cut(int, char**, void*);
extern int	b_dirname(int, char**, void*);
extern int	b_expr(int, char**, void*);
extern int	b_fold(int, char**, void*);
extern int	b_getconf(int, char**, void*);
extern int	b_head(int, char**, void*);
extern int	b_id(int, char**, void*);
extern int	b_join(int, char**, void*);
extern int	b_ln(int, char**, void*);
extern int	b_logname(int, char**, void*);
extern int	b_mkdir(int, char**, void*);
extern int	b_mkfifo(int, char**, void*);
extern int	b_mv(int, char**, void*);
extern int	b_paste(int, char**, void*);
extern int	b_pathchk(int, char**, void*);
extern int	b_rev(int, char**, void*);
extern int	b_rmdir(int, char**, void*);
extern int	b_tail(int, char**, void*);
extern int	b_tee(int, char**, void*);
extern int	b_tty(int, char**, void*);
extern int	b_uname(int, char**, void*);
extern int	b_uniq(int, char**, void*);
extern int	b_wc(int, char**, void*);

#undef	extern

#if defined(BUILTIN) && !defined(STANDALONE)
#define STANDALONE	BUILTIN
#endif

#ifdef	STANDALONE

#ifndef BUILTIN

/*
 * command initialization
 */

static void
cmdinit(register char** argv, void* context)
{
	register char*	cp;

	NoP(context);
	if (cp = strrchr(argv[0], '/'))
		cp++;
	else
		cp = argv[0];
	error_info.id = cp;
	opt_info.index = 0;
}

extern int	STANDALONE(int, char**, void*);

#endif

int
main(int argc, char** argv)
{
	return STANDALONE(argc, argv, NiL);
}

#else

#if _BLD_cmd && defined(__EXPORT__)
#define extern		__EXPORT__
#endif

extern int	cmdrecurse(int, char**, int, char**);
extern void	cmdinit(char**, void*);

#undef	extern

#endif

#endif
