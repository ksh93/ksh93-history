/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1985-2006 AT&T Corp.                  *
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
*                   Phong Vo <kpv@research.att.com>                    *
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Bell Laboratories
 *
 * time conversion support
 */

#include <ast.h>
#include <tm.h>
#include <ctype.h>

/*
 * return the tab table index that matches s ignoring case and .'s
 * tm_data.format checked if tminfo.format!=tm_data.format
 *
 * ntab and nsuf are the number of elements in tab and suf,
 * -1 for 0 sentinel
 *
 * all isalpha() chars in str must match
 * suf is a table of nsuf valid str suffixes 
 * if e is non-null then it will point to first unmatched char in str
 * which will always be non-isalpha()
 */

int
tmlex(register const char* s, char** e, char** tab, int ntab, char** suf, int nsuf)
{
	register char**	p;
	register char*	x;
	register int	n;

	for (p = tab, n = ntab; n-- && (x = *p); p++)
		if (*x && *x != '%' && tmword(s, e, x, suf, nsuf))
			return p - tab;
	if (tm_info.format != tm_data.format && tab >= tm_info.format && tab < tm_info.format + TM_NFORM)
	{
		tab = tm_data.format + (tab - tm_info.format);
		if (suf && tab >= tm_info.format && tab < tm_info.format + TM_NFORM)
			suf = tm_data.format + (suf - tm_info.format);
		for (p = tab, n = ntab; n-- && (x = *p); p++)
			if (*x && *x != '%' && tmword(s, e, x, suf, nsuf))
				return p - tab;
	}
	return -1;
}
