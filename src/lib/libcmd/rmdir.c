/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1992-2005 AT&T Corp.                  *
*                      and is licensed under the                       *
*                  Common Public License, Version 1.0                  *
*                            by AT&T Corp.                             *
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
 * rmdir
 */

static const char usage[] =
"[-?\n@(#)$Id: rmdir (AT&T Labs Research) 1999-04-20 $\n]"
USAGE_LICENSE
"[+NAME?rmdir - remove empty directories]"
"[+DESCRIPTION?\brmdir\b deletes each given directory.  The directory "
	"must be empty; containing no entries other than \b.\b or \b..\b.  "
	"If a directory and a subdirectory of that directory are specified "
	"as operands, the subdirectory must be specified before the parent "
	"so that the parent directory will be empty when \brmdir\b attempts "
	"to remove it.]"
"[p:parents?Remove each explicit \adirectory\a argument directory that "
	"becomes empty after its child directories are removed.]"
"\n"
"\ndirectory ...\n"
"\n"
"[+EXIT STATUS?]{"
        "[+0?All directories deleted successfully.]"
        "[+>0?One or more directories could not be deleted.]"
"}"
"[+SEE ALSO?\bmkdir\b(1), \brm\b(1)]"
;

#include <cmdlib.h>

int
b_rmdir(int argc, char** argv, void* context)
{
	register char*	dir;
	register char*	end;
	register int	n;
	int		pflag = 0;

	NoP(argc);
	cmdinit(argv, context, ERROR_CATALOG, 0);
	while (n = optget(argv, usage)) switch (n)
	{
	case 'p':
		pflag = 1;
		break;
	case ':':
		error(2, "%s", opt_info.arg);
		break;
	case '?':
		error(ERROR_usage(2), "%s", opt_info.arg);
		break;
	}
	argv += opt_info.index;
	if (error_info.errors || !*argv)
		error(ERROR_usage(2), "%s", optusage(NiL));
	while (dir = *argv++)
	{
		end = dir;
		if (pflag) end += strlen(dir);
		n = 0;
		for (;;)
		{
			if (rmdir(dir) < 0)
			{
				error(ERROR_system(0), "%s: cannot remove", dir);
				break;
			}
			if (n) *end = '/';
			else n = 1;
			do if (end <= dir) goto next; while (*--end != '/');
			do if (end <= dir) goto next; while (*(end - 1) == '/' && end--);
			*end = 0;
		}
	next:	;
	}
	return(error_info.errors != 0);
}

