/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1990-2004 AT&T Corp.                  *
*                      and is licensed under the                       *
*          Common Public License, Version 1.0 (the "License")          *
*                        by AT&T Corp. ("AT&T")                        *
*      Any use, downloading, reproduction or distribution of this      *
*      software constitutes acceptance of the License.  A copy of      *
*                     the License is available at                      *
*                                                                      *
*         http://www.research.att.com/sw/license/cpl-1.0.html          *
*         (with md5 checksum 8a5e0081c856944e76c69a1cf29c2e8b)         *
*                                                                      *
*              Information and Software Systems Research               *
*                            AT&T Research                             *
*                           Florham Park NJ                            *
*                                                                      *
*                 Glenn Fowler <gsf@research.att.com>                  *
*                                                                      *
***********************************************************************/
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
	sfstrclose(tmp);
	return n;
}
