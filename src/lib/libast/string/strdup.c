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

#if defined(__STDPP__directive) && defined(__STDPP__hide)
__STDPP__directive pragma pp:hide strdup
#else
#define strdup	______strdup
#endif

#include <ast.h>

#if defined(__STDPP__directive) && defined(__STDPP__hide)
__STDPP__directive pragma pp:nohide strdup
#else
#undef	strdup
#endif

/*
 * return a copy of s using malloc
 */

#if defined(__EXPORT__)
#define extern	__EXPORT__
#endif

extern char*
strdup(register const char* s)
{
	register char*	t;
	register int	n;

	return (t = newof(0, char, n = strlen(s) + 1, 0)) ? (char*)memcpy(t, s, n) : 0;
}
