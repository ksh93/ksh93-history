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
 * use tw to recurse on argc,argv with pfxc,pfxv prefix args
 */

#include <cmdlib.h>
#include <proc.h>
#include <ftwalk.h>

int
cmdrecurse(int argc, char** argv, int pfxc, char** pfxv)
{
	register char**	v;
	register char**	a;
	int		resolve = 'L';
	char		arg[16];

	if (!(a = (char**)stakalloc((argc + pfxc + 4) * sizeof(char**))))
		error(ERROR_exit(1), "out of space");
	v = a;
	*v++ = "tw";
	*v++ = arg;
	*v++ = *(argv - opt_info.index);
	while (*v = *pfxv++)
	{
		if (streq(*v, "-H"))
			resolve = 'H';
		else if (streq(*v, "-P"))
			resolve = 'P';
		v++;
	}
	while (*v++ = *argv++);
	sfsprintf(arg, sizeof(arg), "-%cc%d", resolve, pfxc + 2);
	procopen(*a, a, NiL, NiL, PROC_OVERLAY);
	return(-1);
}
