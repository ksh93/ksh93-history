/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1985-2004 AT&T Corp.                  *
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
*                  David Korn <dgk@research.att.com>                   *
*                   Phong Vo <kpv@research.att.com>                    *
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Bell Laboratories
 *
 * generate ast traps for botched standard prototypes
 */

#include <sys/types.h>

#include "FEATURE/lib"
#include "FEATURE/types"

extern int		getgroups(int, gid_t*);
extern int		printf(const char*, ...);

int
main()
{
#if _lib_getgroups
	if (sizeof(int) > sizeof(gid_t))
	{
		int	n;
		int	i;
		int	r;
		gid_t	groups[32 * sizeof(int) / sizeof(gid_t)];

		r = sizeof(int) / sizeof(gid_t);
		if ((n = getgroups((sizeof(groups) / sizeof(groups[0])) / r, groups)) > 0)
			for (i = 1; i <= n; i++)
			{
				groups[i] = ((gid_t)0);
				if (getgroups(i, groups) != i)
					goto botched;
				if (groups[i] != ((gid_t)0))
					goto botched;
				groups[i] = ((gid_t)-1);
				if (getgroups(i, groups) != i)
					goto botched;
				if (groups[i] != ((gid_t)-1))
					goto botched;
			}
	}
	return 0;
 botched:
	printf("#undef	getgroups\n");
	printf("#define getgroups	_ast_getgroups /* implementation botches gid_t* arg */\n");
#endif
	return 0;
}
