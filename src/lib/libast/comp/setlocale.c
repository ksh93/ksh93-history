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

#include <ast.h>

#undef	setlocale
#undef	strcmp
#undef	strcoll

typedef struct
{
	int		category;
	int		set;
	char*		locale;
} Locale_t;

char*
_ast_setlocale(register int category, const char* locale)
{
#if _hdr_locale && _lib_setlocale
	register Locale_t*	lc;
	register char*		p;

	static Locale_t		def[] =
	{
		{	LC_ALL,		~0		},
		{	LC_COLLATE,	LC_SET_COLLATE	},
		{	LC_CTYPE,	LC_SET_CTYPE	},
#ifdef LC_MESSAGES
		{	LC_MESSAGES,	LC_SET_MESSAGES	},
#endif
		{	LC_MONETARY,	LC_SET_MONETARY	},
		{	LC_NUMERIC,	LC_SET_NUMERIC	},
		{	LC_TIME,	LC_SET_TIME	},
	};

	if (!def[0].locale)
		for (lc = def; lc < def + elementsof(def); lc++)
		{
			if (!(p = setlocale(lc->category, NiL)) || !(p = strdup(p)))
				p = "C";
			lc->locale = p;
		}
	if ((p = setlocale(category, locale)) && locale)
	{
		ast.locale.serial++;
		for (lc = def; lc < def + elementsof(def); lc++)
			if (lc->category == category)
			{
				if (streq(lc->locale, p))
				{
					ast.locale.set &= ~lc->set;
					if (lc->set & LC_SET_COLLATE)
						ast.collate = strcmp;
				}
				else
				{
					ast.locale.set |= lc->set;
					if (lc->set & LC_SET_COLLATE)
						ast.collate = strcoll;
				}
				break;
			}
	}
	return(p);
#else
	return(!locale || !*locale || !strcmp(locale, "C") || !strcmp(locale, "POSIX") ? "C" : (char*)0);
#endif
}
