/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1985-2000 AT&T Corp.                *
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
*                                                                  *
*******************************************************************/
#pragma prototyped

#if defined(__EXPORT__)

#include <sys/types.h>

#define extern	__EXPORT__

extern pid_t	spawnvpe(const char*, char* const[], char* const[]);

#undef	extern

#endif

#include <ast.h>
#include <errno.h>

#if _lib_spawnvpe

NoN(spawnvpe)

#else

pid_t
spawnvpe(const char* name, char* const argv[], char* const envv[])
{
	register const char*	path = name;
	pid_t			pid;
	char			buffer[PATH_MAX];

	if (*path != '/')
		path = pathpath(buffer, name, NULL, X_OK|PATH_REGULAR);
	if ((pid = spawnve(path, argv, envv)) >= 0)
		return(pid);
	if (errno == ENOEXEC)
	{
		register char**	newargv;
		register char**	av = (char**)argv;

		while (*av++);
		if (newargv = newof(0, char*, av + 1 - argv, 0))
		{
			av = newargv;
			*av++ = "sh";
			*av++ = (char*)path;
			while (*av++ = *++argv);
			path = pathshell();
			pid = spawnve(path, newargv, environ);
			free(newargv);
		}
		else errno = ENOMEM;
	}
	return(pid);
}

#endif
