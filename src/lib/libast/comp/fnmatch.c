/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1985-2001 AT&T Corp.                *
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
*******************************************************************/
#pragma prototyped
/*
 * fnmatch implementation
 */

#if defined(__EXPORT__)
__EXPORT__ int	fnmatch(const char*, const char*, int);
#endif

#include <ast.h>

#if _lib_fnmatch && 0

NoN(fnmatch)

#else

#include <regex.h>
#include <fnmatch.h>

typedef struct
{
	int	fnm;		/* fnmatch flag			*/
	int	reg;		/* regex flag			*/
} Map_t;

static const Map_t	map[] =
{
	FNM_AUGMENTED,	REG_AUGMENTED,
	FNM_ICASE,	REG_ICASE,
	FNM_NOESCAPE,	REG_SHELL_ESCAPED,
	FNM_PATHNAME,	REG_SHELL_PATH,
	FNM_PERIOD,	REG_SHELL_DOT,
};

#if defined(__EXPORT__)
#define extern	__EXPORT__
#endif

extern int
fnmatch(const char* pattern, const char* subject, register int flags)
{
	register int		reflags = REG_SHELL|REG_LEFT;
	register const Map_t*	mp;
	regex_t			re;

	for (mp = map; mp < &map[elementsof(map)]; mp++)
		if (flags & mp->fnm)
			reflags |= mp->reg;
	if (flags & FNM_LEADING_DIR)
	{
		regmatch_t	match;

		if (reflags = regcomp(&re, pattern, reflags))
			return reflags;
		if (reflags = regexec(&re, subject, 1, &match, 0))
			return reflags;
		return (!(reflags = subject[match.rm_eo]) || reflags == '/') ? 0 : FNM_NOMATCH;
	}
	if (reflags = regcomp(&re, pattern, reflags|REG_RIGHT))
		return reflags;
	return regexec(&re, subject, 0, NiL, 0);
}

#endif
