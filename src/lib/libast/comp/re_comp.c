/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1985-2004 AT&T Corp.                  *
*                      and is licensed under the                       *
*          Common Public License, Version 1.0 (the "License")          *
*                        by AT&T Corp. ("AT&T")                        *
*      Any use, downloading, reproduction or distribution of this      *
*      software constitutes acceptance of the License.  A copy of      *
*                     the License is available at                      *
*                                                                      *
*         http://www.research.att.com/sw/license/cpl-1.0.html          *
*         (with md5 checksum 8a5e0081c856944e76c69a1cf29c2e8b)         *
*                                                                      *
*              Information and Software Systems Research               *
*                            AT&T Research                             *
*                           Florham Park NJ                            *
*                                                                      *
*                 Glenn Fowler <gsf@research.att.com>                  *
*                  David Korn <dgk@research.att.com>                   *
*                   Phong Vo <kpv@research.att.com>                    *
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * re_comp implementation
 */

#include <ast.h>
#include <re_comp.h>
#include <regex.h>

#undef	error
#undef	valid

static struct
{
	char	error[64];
	regex_t	re;
	int	valid;
} state;

char*
re_comp(const char* pattern)
{
	register int	r;

	if (!pattern || !*pattern)
	{
		if (state.valid)
			return 0;
		r = REG_BADPAT;
	}
	else
	{
		if (state.valid)
		{
			state.valid = 0;
			regfree(&state.re);
		}
		if (!(r = regcomp(&state.re, pattern, REG_LENIENT|REG_NOSUB|REG_NULL)))
		{
			state.valid = 1;
			return 0;
		}
	}
	regerror(r, &state.re, state.error, sizeof(state.error));
	return state.error;
}

int
re_exec(const char* subject)
{
	if (state.valid && subject)
		switch (regexec(&state.re, subject, 0, NiL, 0))
		{
		case 0:
			return 1;
		case REG_NOMATCH:
			return 0;
		}
	return -1;
}
