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
 * G. S. Fowler
 * AT&T Bell Laboratories
 *
 * return modex canonical representation of file mode bits
 * given ls -l style file mode string
 */

#include "modelib.h"

int
strmode(register const char* s)
{
	register int		c;
	register char*		t;
	register struct modeop*	p;
	int			mode;

	mode = 0;
	for (p = modetab; (c = *s++) && p < &modetab[MODELEN]; p++)
		for (t = p->name; *t; t++)
			if (*t == c)
			{
				c = t - p->name;
				mode |= (p->mask1 & (c << p->shift1)) | (p->mask2 & (c << p->shift2));
				break;
			}
	return(mode);
}
