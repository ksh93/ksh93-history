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
*             Glenn Fowler <gsf@research.att.com>              *
*              David Korn <dgk@research.att.com>               *
*               Phong Vo <kpv@research.att.com>                *
*                                                              *
***************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Research
 *
 * time conversion translation support
 */

#include <ast.h>
#include <cdt.h>
#include <mc.h>
#include <tm.h>
#include <loclib.h>

static struct
{
	char*	locale;
	char*	format;
	char	null[1];
} state;

/*
 * load the LC_TIME data for locale
 */

static void
load(char* locale)
{
	register char*	s;
	register char**	v;
	register char**	e;
	size_t		n;
	Sfio_t*		sp;
	char		path[PATH_MAX];

	if (tm_info.format != tm_data.format)
	{
		free(tm_info.format);
		tm_info.format = tm_data.format;
		tm_info.deformat = state.format;
	}
	if (mcfind(path, locale, NiL, LC_TIME, 0) && (sp = sfopen(NiL, path, "r")))
	{
		n = sfsize(sp);
		if (v = newof(0, char*, TM_NFORM, n + 2))
		{
			e = v + TM_NFORM;
			s = (char*)e;
			if (sfread(sp, s, n) == n)
			{
				s[n] = '\n';
				tm_info.format = v;
				while (v < e)
				{
					*v++ = s;
					if (!(s = strchr(s, '\n')))
						break;
					*s++ = 0;
				}
				while (v < e)
					*v++ = state.null;
				if (strchr(tm_info.format[TM_UT], '%'))
				{
					tm_info.deformat = tm_info.format[TM_UT];
					for (n = TM_UT; n < TM_DT; n++)
						tm_info.format[n] = state.null;
				}
				else
					tm_info.deformat = tm_info.format[TM_DEFAULT];
			}
			else
				free(v);
		}
		sfclose(sp);
	}
}

/*
 * check that tm_info.format matches the current locale
 */

char*
tmlocale(void)
{
	if (!ast.locale.serial)
		setlocale(LC_ALL, "");
	if (!tm_info.format)
	{
		tm_info.format = tm_data.format;
		if (!(state.format = tm_info.deformat))
			state.format = tm_data.format[TM_DEFAULT];
		tm_info.deformat = state.format;
	}
	if ((ast.locale.set & (1<<AST_LC_TIME)) && state.locale != locales[AST_LC_TIME].usr)
	{
		state.locale = locales[AST_LC_TIME].usr;
		load(state.locale);
	}
	return state.locale;
}
