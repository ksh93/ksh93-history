/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1985-2002 AT&T Corp.                *
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
*            Information and Software Systems Research             *
*                        AT&T Labs Research                        *
*                         Florham Park NJ                          *
*                                                                  *
*               Glenn Fowler <gsf@research.att.com>                *
*                David Korn <dgk@research.att.com>                 *
*                 Phong Vo <kpv@research.att.com>                  *
*******************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Research
 *
 * return full path to p with mode access using $PATH
 * if a!=0 then it and $0 and $_ with $PWD are used for
 * related root searching
 * the related root must have a bin subdir
 * p==0 sets the cached relative dir to a
 * p==0 a=="" disables $0 $_ $PWD relative search
 * full path returned in path buffer
 * if path==0 then the space is malloc'd
 */

#include <ast.h>
#include <option.h>

char*
pathpath(register char* path, const char* p, const char* a, int mode)
{
	register char*	s;
	char*		x;
	char		buf[PATH_MAX];

	static char*	cmd;

	if (!path)
		path = buf;
	if (!p)
	{
		if (cmd)
			free(cmd);
		cmd = a ? strdup(a) : (char*)0;
		return 0;
	}
	if (*p == '/')
		a = 0;
	else if (s = (char*)a)
	{
		x = s;
		if (strchr(p, '/'))
		{
			a = p;
			p = "..";
		}
		else
			a = 0;
		if ((!cmd || *cmd) && (strchr(s, '/') ||
		    ((s = cmd) ||
		     opt_info.argv && (s = *opt_info.argv)) &&
			strchr(s, '/') && !strchr(s, '\n') && !access(s, F_OK) ||
		    environ && (s = *environ) &&
			*s++ == '_' && *s++ == '=' && strchr(s, '/') && !strneq(s, "/bin/", 5) && !strneq(s, "/usr/bin/", 9) ||
		    *x && !access(x, F_OK) &&
			(s = getenv("PWD")) && *s == '/'))
		{
			if (!cmd)
				cmd = strdup(s);
			if (strlen(s) < (sizeof(buf) - 6))
			{
				s = strcopy(path, s);
				for (;;)
				{
					do if (s <= path) goto normal; while (*--s == '/');
					do if (s <= path) goto normal; while (*--s != '/');
					strcpy(s + 1, "bin");
					if (pathexists(path, PATH_EXECUTE))
					{
						if (s = pathaccess(path, path, p, a, mode))
							return path == buf ? strdup(s) : s;
						goto normal;
					}
				}
			normal: ;
			}
		}
	}
	x = !a && strchr(p, '/') ? "" : pathbin();
	if (!(s = pathaccess(path, x, p, a, mode)) && !*x && (x = getenv("FPATH")))
		s = pathaccess(path, x, p, a, mode);
	return (s && path == buf) ? strdup(s) : s;
}
