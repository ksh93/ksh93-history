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
