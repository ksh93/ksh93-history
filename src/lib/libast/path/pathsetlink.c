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
*/

#include "univlib.h"

/*
 * create symbolic name from external representation text in buf
 * the arg order matches link(2)
 */

int
pathsetlink(const char* buf, const char* name)
{
	register char*	t = (char*)buf;
#ifdef UNIV_MAX
	register char*	s = (char*)buf;
	register char*	v;
	int		n;
	char		tmp[PATH_MAX];

	while (*s)
	{
		if (*s++ == univ_cond[0] && !strncmp(s - 1, univ_cond, univ_size))
		{
			s--;
			t = tmp;
			for (n = 0; n < UNIV_MAX; n++)
				if (*univ_name[n])
			{
				*t++ = ' ';
#ifdef ATT_UNIV
				*t++ = '1' + n;
				*t++ = ':';
#else
				for (v = univ_name[n]; *t = *v++; t++);
				*t++ = '%';
#endif
				for (v = (char*)buf; v < s; *t++ = *v++);
				for (v = univ_name[n]; *t = *v++; t++);
				for (v = s + univ_size; *t = *v++; t++);
			}
			t = tmp;
			break;
		}
	}
#endif
	return(symlink(t, name));
}
