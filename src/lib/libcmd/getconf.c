/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*           Copyright (c) 1992-2006 AT&T Knowledge Ventures            *
*                      and is licensed under the                       *
*                  Common Public License, Version 1.0                  *
*                      by AT&T Knowledge Ventures                      *
*                                                                      *
*                A copy of the License is available at                 *
*            http://www.opensource.org/licenses/cpl1.0.txt             *
*         (with md5 checksum 059e8cd6165cb4c31e351f2b69388fd9)         *
*                                                                      *
*              Information and Software Systems Research               *
*                            AT&T Research                             *
*                           Florham Park NJ                            *
*                                                                      *
*                 Glenn Fowler <gsf@research.att.com>                  *
*                  David Korn <dgk@research.att.com>                   *
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Research
 *
 * getconf - get configuration values
 */

static const char usage[] =
"[-?\n@(#)$Id: getconf (AT&T Research) 2006-10-11 $\n]"
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
"	\aname\a is valid but undefined then \bundefined\b is written to"
"	the standard output. If \aname\a is invalid or an error occurs in"
"	determining its value, then a diagnostic written to the standard error"
"	and \bgetconf\b exits with a non-zero exit status.]"
"[+?More than one variable may be set or queried by providing the \aname\a"
"	\apath\a \avalue\a 3-tuple for each variable, specifying \b-\b for"
"	\avalue\a when querying.]"
"[+?If no operands are specified then all known variables are written in"
"	\aname\a=\avalue\a form to the standard output, one per line."
"	Only one of \b--call\b, \b--name\b or \b--standard\b may be specified.]"
"[+?This implementation uses the \bastgetconf\b(3) string interface to the native"
"	\bsysconf\b(2), \bconfstr\b(2), \bpathconf\b(2), and \bsysinfo\b(2)"
"	system calls. If \bgetconf\b on \b$PATH\b is not the default native"
"	\bgetconf\b, named by \b$(getconf GETCONF)\b, then \bastgetconf\b(3)"
"	checks only \bast\b specific extensions and the native system calls;"
"	invalid options and/or names not supported by \bastgetconf\b(3) cause"
"	the \bgetconf\b on \b$PATH\b to be executed.]"

"[a:all?Call the native \bgetconf\b(1) with option \b-a\b.]"
"[b:base?List base variable name sans call and standard prefixes.]"
"[c:call?Display variables with call prefix that matches \aRE\a. The call"
"	prefixes are:]:[RE]{"
"		[+CS?\bconfstr\b(2)]"
"		[+PC?\bpathconf\b(2)]"
"		[+SC?\bsysconf\b(2)]"
"		[+SI?\bsysinfo\b(2)]"
"		[+XX?Constant value.]"
"}"
"[d:defined?Only display defined values when no operands are specified.]"
"[l:lowercase?List variable names in lower case.]"
"[n:name?Display variables with name that match \aRE\a.]:[RE]"
"[p:portable?Display the named \bwritable\b variables and values in a form that"
"	can be directly executed by \bsh\b(1) to set the values. If \aname\a"
"	is omitted then all \bwritable\b variables are listed.]"
"[q:quote?\"...\" quote values.]"
"[r:readonly?Display the named \breadonly\b variables in \aname\a=\avalue\a form."
"	If \aname\a is omitted then all \breadonly\b variables are listed.]"
"[s:standard?Display variables with standard prefix that matches \aRE\a."
"	Use the \b--table\b option to view all standard prefixes, including"
"	local additions. The standard prefixes available on all systems"
"	are:]:[RE]{"
"		[+AES]"
"		[+AST]"
"		[+C]"
"		[+GNU]"
"		[+POSIX]"
"		[+SVID]"
"		[+XBS5]"
"		[+XOPEN]"
"		[+XPG]"
"}"
"[t:table?Display the internal table that contains the name, standard,"
"	standard section, and system call symbol prefix for each variable.]"
"[w:writable?Display the named \bwritable\b variables in \aname\a=\avalue\a"
"	form. If \aname\a is omitted then all \bwritable\b variables are"
"	listed.]"
"[v:specification?Call the native \bgetconf\b(1) with option"
"	\b-v\b \aname\a.]:[name]"

"\n"
"\n[ name [ path [ value ] ] ... ]\n"
"\n"

"[+ENVIRONMENT]{"
"	[+_AST_FEATURES?Process local writable values that are different from"
"		the default are stored in the \b_AST_FEATURES\b environment"
"		variable. The \b_AST_FEATURES\b value is a space-separated"
"		list of \aname\a \apath\a \avalue\a 3-tuples, where"
"		\aname\a is the system configuration name, \apath\a is the"
"		corresponding path, \b-\b if no path is applicable, and"
"		\avalue\a is the system configuration value.]"
"}"
"[+SEE ALSO?\bpathchk\b(1), \bconfstr\b(2), \bpathconf\b(2),"
"	\bsysconf\b(2), \bastgetconf\b(3)]"
;

#include <cmdlib.h>
#include <proc.h>

int
b_getconf(int argc, char** argv, void* context)
{
	register char*	name;
	register char*	path;
	register char*	value;
	register char*	s;
	char*		pattern;
	char*		native;
	int		flags;
	char**		a;
	char**		oargv;
	char		cmd[PATH_MAX];

	static char	empty[] = "-";

	NoP(argc);
	oargv = argv;
	cmdinit(argv, context, ERROR_CATALOG, 0);
	if (*(native = astconf("GETCONF", NiL, NiL)) != '/')
		native = 0;
	flags = 0;
	name = 0;
	pattern = 0;
	for (;;)
	{
		switch (optget(argv, usage))
		{
		case 'a':
			if (native)
				goto defer;
			continue;
		case 'b':
			flags |= ASTCONF_base;
			continue;
		case 'c':
			flags |= ASTCONF_matchcall;
			pattern = opt_info.arg;
			continue;
		case 'd':
			flags |= ASTCONF_defined;
			continue;
		case 'l':
			flags |= ASTCONF_lower;
			continue;
		case 'n':
			flags |= ASTCONF_matchname;
			pattern = opt_info.arg;
			continue;
		case 'p':
			flags |= ASTCONF_parse;
			continue;
		case 'q':
			flags |= ASTCONF_quote;
			continue;
		case 'r':
			flags |= ASTCONF_read;
			continue;
		case 's':
			flags |= ASTCONF_matchstandard;
			pattern = opt_info.arg;
			continue;
		case 't':
			flags |= ASTCONF_table;
			continue;
		case 'v':
			if (native)
				goto defer;
			continue;
		case 'w':
			flags |= ASTCONF_write;
			continue;
		case ':':
			if (native)
				goto defer;
			error(2, "%s", opt_info.arg);
			break;
		case '?':
			error(ERROR_usage(2), "%s", opt_info.arg);
			break;
		}
		break;
	}
	argv += opt_info.index;
	if (!(name = *argv))
		path = 0;
	else if (streq(name, empty))
	{
		name = 0;
		if (path = *++argv)
		{
			argv++;
			if (streq(path, empty))
				path = 0;
		}
	}
	if (error_info.errors || !name && *argv)
		error(ERROR_usage(2), "%s", optusage(NiL));
	if (!name)
		astconflist(sfstdout, path, flags, pattern);
	else
	{
		flags = native ? (ASTCONF_system|ASTCONF_error) : 0;
		do
		{
			if (!(path = *++argv))
				value = 0;
			else
			{
				if (streq(path, empty))
				{
					path = 0;
					flags = 0;
				}
				if ((value = *++argv) && (streq(value, empty)))
				{
					value = 0;
					flags = 0;
				}
			}
			s = astgetconf(name, path, value, flags, errorf);
			if (error_info.errors)
				break;
			if (!s)
				goto defer;
			if (!value)
			{
				if (flags & ASTCONF_write)
				{
					sfputr(sfstdout, name, ' ');
					sfputr(sfstdout, path ? path : empty, ' ');
				}
				sfputr(sfstdout, *s ? s : "undefined", '\n');
			}
		} while (*argv && (name = *++argv));
	}
	return error_info.errors != 0;
 defer:
	if (!pathaccess(cmd, astconf("PATH", NiL, NiL), error_info.id, NiL, PATH_EXECUTE|PATH_REGULAR) &&
	    !pathaccess(cmd, "/usr/sbin:/sbin", error_info.id, NiL, PATH_EXECUTE|PATH_REGULAR))
	{
		if (name)
			error(3, "%s: unknown name -- no native getconf(1) to defer to", name);
		else
			error(3, "no native getconf(1) to defer to");
		flags = 2;
	}
	else if ((flags = procrun(cmd, oargv)) >= EXIT_NOEXEC)
		error(ERROR_SYSTEM|2, "%s: exec error [%d]", cmd, flags);
	return flags;
}
