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
 * Glenn Fowler
 * AT&T Research
 *
 * return in path the full path name of the probe(1)
 * information for lang and tool using proc
 * if attr != 0 then path attribute assignments placed here
 *
 * if path==0 then the space is malloc'd
 *
 * op:
 *
 *	-3	return non-writable path name with no generation
 *	-2	return path name with no generation
 *	-1	return no $HOME path name with no generation
 *	0	verbose probe
 *	1	silent probe
 *
 * 0 returned if the info does not exist and cannot be generated
 */

#include <ast.h>
#include <error.h>
#include <ls.h>
#include <proc.h>

#ifndef PROBE
#define PROBE		"probe"
#endif

char*
pathprobe(char* path, char* attr, const char* lang, const char* tool, const char* aproc, int op)
{
	char*		proc = (char*)aproc;
	register char*	p;
	register char*	k;
	register char**	ap;
	int		n;
	char*		e;
	char*		probe;
	char		buf[PATH_MAX];
	char		cmd[PATH_MAX];
	char		exe[PATH_MAX];
	char		lib[PATH_MAX];
	char		key[16];
	char*		arg[6];
	unsigned long	ptime;
	struct stat	st;
	struct stat	ps;

	if (*proc != '/')
	{
		if (p = strchr(proc, ' '))
		{
			n = p - proc;
			proc = strncpy(buf, proc, n);
			*(proc + n) = 0;
		}
		if (!(proc = pathpath(cmd, proc, NiL, PATH_ABSOLUTE|PATH_REGULAR|PATH_EXECUTE)))
			proc = (char*)aproc;
		else if (p)
			strcpy(proc + strlen(proc), p);
	}
	if (!path)
		path = buf;
	probe = PROBE;
	p = strcopy(lib, "lib/");
	p = strcopy(p, probe);
	*p++ = '/';
	p = strcopy(k = p, lang);
	*p++ = '/';
	p = strcopy(p, tool);
	*p++ = '/';
	pathkey(key, attr, lang, proc);
	if (op >= -2)
	{
		strcpy(p, key);
		if (pathpath(path, lib, "", PATH_ABSOLUTE) && !stat(path, &st) && (st.st_mode & S_IWUSR))
			return path == buf ? strdup(path) : path;
	}
	e = strcopy(p, probe);
	if (!pathpath(path, lib, "", PATH_ABSOLUTE|PATH_EXECUTE) || stat(path, &ps))
		return 0;
	ptime = ps.st_mtime;
	n = strlen(path);
	if (n < (PATH_MAX + 5))
	{
		strcpy(path + n, ".ini");
		if (!stat(path, &st) && st.st_size && ptime < (unsigned long)st.st_mtime)
			ptime = st.st_mtime;
	}
	strcpy(p, key);
	p = path + n - (e - k);
	strcpy(p, probe);
	if (stat(path, &st))
		return 0;
	strcpy(exe, path);
	if (op >= -1 && !(st.st_mode & S_ISUID) && ps.st_uid != geteuid())
	{
		if (!(p = getenv("HOME")))
			return 0;
		p = strcopy(path, p);
		*p++ = '/';
		*p++ = '.';
		p = strcopy(p, probe);
		*p++ = '/';
	}
	strcpy(p, k);
	if (op >= 0 && !stat(path, &st))
	{
		if (ptime <= (unsigned long)st.st_mtime || ptime <= (unsigned long)st.st_ctime)
			op = -1;
		else if (st.st_mode & S_IWUSR)
		{
			if (op == 0)
				error(0, "%s probe information for %s language processor %s must be manually regenerated", tool, lang, proc);
			op = -1;
		}
	}
	if (op >= 0)
	{
		ap = arg;
		*ap++ = exe;
		if (op > 0)
			*ap++ = "-s";
		*ap++ = (char*)lang;
		*ap++ = (char*)tool;
		*ap++ = proc;
		*ap = 0;
		if (procrun(exe, arg))
			return 0;
		if (access(path, R_OK))
			return 0;
	}
	return path == buf ? strdup(path) : path;
}
