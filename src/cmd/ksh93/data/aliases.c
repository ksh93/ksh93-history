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
#pragma prototyped
#include	<ast.h>
#include	<signal.h>
#include	"FEATURE/options"
#include	"FEATURE/dynamic"
#include	"shtable.h"
#include	"name.h"

/*
 * This is the table of built-in aliases.  These should be exported.
 */

const struct shtable2 shtab_aliases[] =
{
#ifdef SHOPT_FS_3D
	"2d",		NV_NOFREE|NV_EXPORT,	"set -f;_2d",
#endif /* SHOPT_FS_3D */
	"autoload",	NV_NOFREE|NV_EXPORT,	"typeset -fu",
	"command",	NV_NOFREE|NV_EXPORT,	"command ",
	"fc",		NV_NOFREE|NV_EXPORT,	"hist",
	"float",	NV_NOFREE|NV_EXPORT,	"typeset -E",
	"functions",	NV_NOFREE|NV_EXPORT,	"typeset -f",
	"hash",		NV_NOFREE|NV_EXPORT,	"alias -t --",
	"history",	NV_NOFREE|NV_EXPORT,	"hist -l",
	"integer",	NV_NOFREE|NV_EXPORT,	"typeset -i",
	"nameref",	NV_NOFREE|NV_EXPORT,	"typeset -n",
	"nohup",	NV_NOFREE|NV_EXPORT,	"nohup ",
	"r",		NV_NOFREE|NV_EXPORT,	"hist -s",
	"redirect",	NV_NOFREE|NV_EXPORT,	"command exec",
	"times",	NV_NOFREE|NV_EXPORT,	"{ { time;} 2>&1;}",
	"type",		NV_NOFREE|NV_EXPORT,	"whence -v",
#ifdef SIGTSTP
	"stop",		NV_NOFREE|NV_EXPORT,	"kill -s STOP",
	"suspend", 	NV_NOFREE|NV_EXPORT,	"kill -s STOP $$",
#endif /*SIGTSTP */
	"",		0,			(char*)0
};

