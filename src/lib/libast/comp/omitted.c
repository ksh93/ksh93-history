/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1985-2000 AT&T Corp.                *
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
*                                                                  *
*******************************************************************/
#pragma prototyped
/*
 * this file is reserved for brainless omissions from system libraries
 * usually the result of attempts to separate s5r4 from bsd or ucb
 */

#include <ast.h>

#if sun || _sun || __sun

#ifdef	_SC_PAGESIZE
#undef	PAGESIZE
#define PAGESIZE	(int)sysconf(_SC_PAGESIZE)
#else
#ifndef PAGESIZE
#define PAGESIZE	4096
#endif
#endif

void
bzero(void* b, size_t n)
{
	memset(b, 0, n);
}

int
getpagesize()
{
	return PAGESIZE;
}

int
killpg(pid_t pgrp, int sig)
{
	return kill(-pgrp, sig);
}

#else

NoN(compatibility)

#endif
