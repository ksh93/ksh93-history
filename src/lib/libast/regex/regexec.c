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
 * posix regex executor
 * single unsized-string interface
 */

#include "reglib.h"

/*
 * standard wrapper for the sized-record interface
 */

int
regexec(const regex_t* p, const char* s, size_t nmatch, regmatch_t* match, int flags)
{
	if (flags & REG_STARTEND)
	{
		int		m = match->rm_so;
		regmatch_t*	e;

		if (!(flags = regnexec(p, s + m, match->rm_eo - m, nmatch, match, flags)) && m > 0)
			for (e = match + nmatch; match < e; match++)
				if (match->rm_so >= 0)
				{
					match->rm_so += m;
					match->rm_eo += m;
				}
		return flags;
	}
	return regnexec(p, s, s ? strlen(s) : 0, nmatch, match, flags);
}
