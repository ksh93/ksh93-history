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
 * return error message string given errno
 */

#include <ast.h>
#include <error.h>

#include "FEATURE/errno"

#undef	strerror

#if !_def_errno_sys_nerr
extern int	sys_nerr;
#endif
#if !_def_errno_sys_errlist
extern char*	sys_errlist[];
#endif

#if _lib_strerror
extern char*	strerror(int);
#endif

char*
_ast_strerror(int err)
{
	static int	sys;
	static char	msg[28];

	if (err > 0 && err <= sys_nerr)
	{
		if (ERROR_translating())
		{
#if _lib_strerror
			if (!sys)
			{
				char*	s;
				char*	p;
				int	n;

				/*
				 * make sure strerror() translates
				 */

				if (!(s = strerror(1)))
					sys = -1;
				else
				{
					if ((n = strlen(s)) >= sizeof(msg))
						n = sizeof(msg) - 1;
					strncpy(msg, s, n);
					p = setlocale(LC_MESSAGES, "C");
					sys = (s = strerror(1)) && strncmp(s, msg, n) ? 1 : -1;
					setlocale(LC_MESSAGES, p);
				}
			}
			if (sys > 0)
				return strerror(err);
#endif
			return ERROR_translate(NiL, NiL, "errlist", (char*)sys_errlist[err]);
		}
		return (char*)sys_errlist[err];
	}
	sfsprintf(msg, sizeof(msg), ERROR_translate(NiL, NiL, "errlist", "Error %d"), err);
	return msg;
}

#if !_lib_strerror

#if defined(__EXPORT__)
#define extern		__EXPORT__
#endif

extern char*
strerror(int err)
{
	return _ast_strerror(err);
}

#endif
