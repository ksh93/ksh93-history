/***************************************************************
*                                                              *
*           This software is part of the ast package           *
*              Copyright (c) 1992-2000 AT&T Corp.              *
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
*                                                              *
***************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Labs Research
 *
 * getconf - get configuration values
 */

static const char usage[] =
"[-?\n@(#)getconf (AT&T Labs Research) 2000-03-17\n]"
USAGE_LICENSE
"[+NAME?getconf - get configuration values]"
"[+DESCRIPTION?\bgetconf\b displays the system configuration value for"
"	\aname\a. If \aname\a is a filesystem specific variable then"
"	the value is determined relative to \apath\a or the current"
"	directory if \apath\a is omitted. If \avalue\a is specified then"
"	\bgetconf\b attempts to change the process local value to \avalue\a."
"	\b-\b may be used in place of \apath\a when it is not relevant."
"	Only \bwritable\b variables may be set; \breadonly\b variables"
"	cannot be changed.]"
"[+?The current value for \aname\a is written to the standard output. If"
"	\aname\a s valid but undefined then \bundefined\b is written to"
"	the standard output. If \aname\a is invalid or an error occurs in"
"	determining its value then a diagnostic written to the standard error"
"	and \bgetconf\b exits with a non-zero exit status.]"
"[+?More than one variable may be set or queried by providing the \aname\a"
"	\apath\a \avalue\a 3-tuple for each variable, specifying \b-\b for"
"	\avalue\a when querying.]"
"[+?If no arguments are specified then all known variables are written in"
"	\aname\a=\avalue\a form to the standard output, one per line.]"

"[a:all?All known variables are written in \aname\a=\avalue\a form to the"
"	standard output, one per line. Present for compatibility with other"
"	implementations.]"
"[p:portable?Display the named \bwritable\b variables and values in a form that"
"	can be directly executed by \bsh\b(1) to set the values. If \aname\a"
"	is omitted then all \bwritable\b variables are listed.]"
"[r:readonly?Display the named \breadonly\b variables in \aname\a=\avalue\a form."
"	If \aname\a is omitted then all \breadonly\b variables are listed.]"
"[w:writable?Display the named \bwritable\b variables in \aname\a=\avalue\a"
"	form. If \aname\a is omitted then all \bwritable\b variables are"
"	listed.]"
"[v:specification?Ignored by this implementation.]:[name]"

"\n"
"\n[ name [ path [ value ] ] ... ]\n"
"\n"

"[+SEE ALSO?\bpathchk\b(1), \bconfstr\b(2), \bpathconf\b(2),"
"	\bsysconf\b(2), \bastgetconf\b(3)]"
;

#include <cmdlib.h>

int
b_getconf(int argc, char** argv, void* context)
{
	register char*	name;
	register char*	path;
	register char*	value;
	register char*	s;
	int		all;
	int		flags;

	static char	empty[] = "-";

	NoP(argc);
	cmdinit(argv, context, ERROR_CATALOG);
	all = 0;
	flags = 0;
	for (;;)
	{
		switch (optget(argv, usage))
		{
		case 'a':
			all = opt_info.num;
			continue;
		case 'p':
			flags |= X_OK;
			continue;
		case 'r':
			flags |= R_OK;
			continue;
		case 'w':
			flags |= W_OK;
			continue;
		case ':':
			error(2, "%s", opt_info.arg);
			break;
		case '?':
			error(ERROR_usage(2), "%s", opt_info.arg);
			break;
		}
		break;
	}
	argv += opt_info.index;
	if (error_info.errors || (name = *argv) && all)
		error(ERROR_usage(2), "%s", optusage(NiL));
	do
	{
		if (!name)
		{
			path = 0;
			value = 0;
		}
		else
		{
			if (streq(name, empty))
				name = 0;
			if (!(path = *++argv))
				value = 0;
			else
			{
				if (streq(path, empty))
					path = 0;
				if ((value = *++argv) && (streq(value, empty)))
					value = 0;
			}
		}
		if (!name)
			astconflist(sfstdout, path, flags);
		else if (!(s = astgetconf(name, path, value, errorf)))
			break;
		else if (!value)
		{
			if (flags & X_OK)
			{
				sfputr(sfstdout, name, ' ');
				sfputr(sfstdout, path ? path : empty, ' ');
			}
			sfputr(sfstdout, *s ? s : "undefined", '\n');
		}
	} while (*argv && (name = *++argv));
	error_info.flags &= ~ERROR_LIBRARY;
	return error_info.errors != 0;
}
