/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1990-2002 AT&T Corp.                *
*        and it may only be used by you under license from         *
*                       AT&T Corp. ("AT&T")                        *
*         A copy of the Source Code Agreement is available         *
*                at the AT&T Internet web site URL                 *
*                                                                  *
*       http://www.research.att.com/sw/license/ast-open.html       *
*                                                                  *
*    If you have copied or used this software without agreeing     *
*        to the terms of the license you are infringing on         *
*           the license and copyright and are violating            *
*               AT&T's intellectual property rights.               *
*                                                                  *
*            Information and Software Systems Research             *
*                        AT&T Labs Research                        *
*                         Florham Park NJ                          *
*                                                                  *
*               Glenn Fowler <gsf@research.att.com>                *
*                                                                  *
*******************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Research
 *
 * coshell procrun(3)
 */

#include "colib.h"

#include <proc.h>
#include <sfstr.h>

int
procrun(const char* path, char** argv)
{
	register char*		s;
	register char**		a;
	register Sfio_t*	tmp;
	int			n;

	if (!(a = argv))
		return procclose(procopen(path, a, NiL, NiL, PROC_FOREGROUND|PROC_GID|PROC_UID));
	if (!(tmp = sfstropen()))
		return -1;
	sfputr(tmp, path ? path : "sh", -1);
	while (s = *++a)
	{
		sfputr(tmp, " '", -1);
		coquote(tmp, s, 0);
		sfputc(tmp, '\'');
	}
	n = system(sfstruse(tmp));
	free(tmp);
	return n;
}
