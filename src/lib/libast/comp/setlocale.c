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
 * setlocale() intercept
 * keeps a bitmask of non-default categories
 * and a permanent locale namespace for pointer comparison
 */

#include "loclib.h"

#include <mc.h>
#include <namval.h>

#undef	setlocale
#undef	strcmp
#undef	strcoll

#if !_lib_setlocale
#define setlocale(i,v)		local.name
#endif

typedef struct Name_s
{
	struct Name_s*	next;
	int		checked;
	int		valid;
	char		name[2];
} Name_t;

/*
 * this table order must correspond to the AST_LC_[A-Z]* indexes
 */

Locale_t		locales[] =
{
	{	"LC_COLLATE",	LC_COLLATE	},
	{	"LC_CTYPE",	LC_CTYPE	},
	{	"LC_MESSAGES",	LC_MESSAGES	},
	{	"LC_MONETARY",	LC_MONETARY	},
	{	"LC_NUMERIC",	LC_NUMERIC	},
	{	"LC_TIME",	LC_TIME		},
	{	0,		0		},
};

static const Namval_t	options[] =
{
	"debug",		AST_LC_debug,
	"setlocale",		AST_LC_setlocale,
	"translate",		AST_LC_translate,
};

static Name_t		local = { 0, 0, 0, "C" };

static Name_t*		names = &local;

static char*		all;

/*
 * called by stropt() to set options
 */

static int
setopt(void* a, const void* p, int n, const char* v)
{
	if (p)
	{
		if (n)
			ast.locale.set |= ((Namval_t*)p)->value;
		else
			ast.locale.set &= ~((Namval_t*)p)->value;
	}
	return 0;
}

/*
 * unique locale name cache
 */

static char*
unique(register char* s)
{
	register Name_t*	p;

	if (!s)
		return local.name;
	for (p = names; p; p = p->next)
		if (streq(s, p->name))
			return p->name;
	if (!(p = newof(0, Name_t, 1, strlen(s))))
		return local.name;
	strcpy(p->name, s);

	/*
	 * ast currently handles LC_MESSAGES and LC_TIME overrides
	 * so omit the file override search in valid() for the others
	 */

	p->valid = ~0;
	if (!streq(s, "debug") && !strchr(s, '.'))
		p->valid &= ~((1<<AST_LC_MESSAGES)|(1<<AST_LC_TIME));
	p->checked = p->valid;
	p->next = names;
	names = p;
	return p->name;
}

/*
 * return locale name if it is a valid user locale for category lc
 */

static char*
valid(register Locale_t* lc, register char* s)
{
	register Name_t*	p;
	char			path[PATH_MAX];

	for (p = names; p; p = p->next)
		if (s == p->name)
		{
			if (!(p->checked & lc->set))
			{
				p->checked |= lc->set;
				if (mcfind(path, s, NiL, lc->category, 0))
					p->valid |= lc->set;
			}
			return (p->valid & lc->set) ? p->name : (char*)0;
		}
	return 0;
}

char*
_ast_setlocale(register int category, const char* locale)
{
	register Locale_t*	lc;
	register char*		p;
	register char*		u;
	int			set;

#if 0
sfprintf(sfstderr, "AHA#%d:%s %d %s (0x%04x)\n", __LINE__, __FILE__, category, locale, ast.locale.set);
#endif
	set = ast.locale.set;
	if (!locales[AST_LC_MESSAGES].sys)
	{
		stropt(getenv("LC_OPTIONS"), options, sizeof(*options), setopt, NiL);
		if (!(p = getenv("LC_ALL")))
			p = getenv("LANG");
		all = unique(p);
#if 0
sfprintf(sfstderr, "AHA#%d:%s %s %s\n", __LINE__, __FILE__, p, all);
#endif
		setlocale(LC_ALL, "");
		for (lc = locales; lc->name; lc++)
		{
			lc->set = 1<<(lc-locales);
			lc->sys = unique(setlocale(lc->category, NiL));
			if ((p = (p = getenv(lc->name)) ? unique(p) : all) != local.name && (lc->usr = valid(lc, p)) || lc->sys != local.name)
				ast.locale.set |= lc->set;
			if (!lc->usr)
				lc->usr = lc->sys;
			if ((ast.locale.set & AST_LC_setlocale) && lc->usr != local.name)
				sfprintf(sfstderr, "setlocale %11s %-28s %-28s\n", lc->name, lc->usr, lc->sys);
		}
	}
	if (locale && *locale)
	{
		u = unique((char*)locale);
		p = unique(setlocale(category, u));
		for (lc = locales; lc->name; lc++)
			if (category == LC_ALL || lc->category == category && (p = lc->usr))
			{
				if (u == lc->old_usr)
					setlocale(lc->category, lc->old_sys);
				lc->old_usr = lc->usr;
				lc->old_sys = lc->sys;
				lc->usr = 0;
				lc->sys = unique(setlocale(lc->category, NiL));
				if (lc->sys != local.name || u != local.name && (lc->usr = valid(lc, u)))
					ast.locale.set |= lc->set;
				else
					ast.locale.set &= ~lc->set;
				if (!lc->usr)
					lc->usr = lc->sys;
				if (ast.locale.set & AST_LC_setlocale)
					sfprintf(sfstderr, "setlocale %11s %-28s %-28s\n", lc->name, lc->usr, lc->sys);
				if (category != LC_ALL)
					break;
			}
	}
	else if (category == LC_ALL)
		p = unique(setlocale(category, NiL));
	else
	{
		p = 0;
		for (lc = locales; lc->name; lc++)
			if (lc->category == category)
			{
				p = lc->usr;
				break;
			}
		if (!p)
			p = unique(setlocale(category, NiL));
	}
	if (ast.locale.set != set)
	{
		ast.locale.serial++;
		ast.collate = (ast.locale.set & (1<<AST_LC_COLLATE)) ? strcoll : strcmp;
#if MB_LEN_MAX > 1
		if (MB_CUR_MAX > 1 && (ast.locale.set & (1<<AST_LC_CTYPE)))
			ast.locale.set |= AST_LC_multibyte;
		else
			ast.locale.set &= ~AST_LC_multibyte;
#endif
	}
	return p;
}
