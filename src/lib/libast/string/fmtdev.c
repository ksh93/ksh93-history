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
*     If you received this software without first entering     *
*       into a license with AT&T, you have an infringing       *
*           copy and cannot use it without violating           *
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
 * AT&T Bell Laboratories
 *
 * idevice() formatter
 */

#include <ast.h>
#include <ctype.h>
#include <ls.h>

char*
fmtdev(struct stat* st)
{
	unsigned int	ma = major(idevice(st));
	unsigned int	mi = minor(idevice(st));

	static char	buf[18];

	if (ma == '#' && isalnum(mi))
	{
		/*
		 * Plan? Nein!
		 */

		buf[0] = ma;
		buf[1] = mi;
		buf[2] = 0;
	}
	else sfsprintf(buf, sizeof(buf), "%03d,%03d", ma, mi);
	return(buf);
}
