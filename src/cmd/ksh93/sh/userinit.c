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
*      If you have copied this software without agreeing       *
*      to the terms of the license you are infringing on       *
*         the license and copyright and are violating          *
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
 *  This is a hook for an additional initialiaztion routine
 *  A function of this name is called in main after sh_init().
 */

void	sh_userinit(int mode)
{
	if(sh.userinit)
		(*sh.userinit)(mode);
}
