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

#if defined(__EXPORT__)

#include <sys/types.h>

#define extern	__EXPORT__

extern pid_t	spawnlp(const char*, const char*, ...);

#undef	extern

#endif

#include <ast.h>
#include <errno.h>

#if _lib_spawnlp

NoN(spawnlp)

#else

pid_t
spawnlp(const char* name, const char* arg, ...)
{
	va_list	ap;
	pid_t	pid;

	va_start(ap, arg);
	pid = spawnvp(name, (char* const*)&arg);
	va_end(ap);
	return(pid);
}

#endif
