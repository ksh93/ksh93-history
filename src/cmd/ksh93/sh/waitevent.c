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

#include	"defs.h"
/*
 *  This installs a hook to allow the processing of events when
 *  the shell is waiting for input and when the shell is
 *  waiting for job completion.
 *  The previous waitevent hook function is returned
 */


void	*sh_waitnotify(int(*newevent)(int,long,int))
{
	int (*old)(int,long,int);
	old = sh.waitevent;
	sh.waitevent = newevent;
	return((void*)old);
}

