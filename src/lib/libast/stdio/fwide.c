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

#include "stdhdr.h"

int
fwide(Sfio_t* f, int mode)
{
	STDIO_INT(f, "fwide", int, (Sfio_t*, int), (f, mode))

	if (mode > 0)
	{
		f->bits &= ~SF_MB;
		f->bits |= SF_WC;
	}
	else if (mode < 0)
	{
		f->bits &= ~SF_WC;
		f->bits |= SF_MB;
	}
	if (f->bits & SF_MB)
		return -1;
	if (f->bits & SF_WC)
		return 1;
	if ((f->flags & SF_SYNCED) || f->next > f->data)
	{
		f->bits |= SF_MB;
		return -1;
	}
	return 0;
}
