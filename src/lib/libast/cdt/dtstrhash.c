/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1985-2004 AT&T Corp.                  *
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
#include	"dthdr.h"

/*	Hashing a string
**
**	Written by Kiem-Phong Vo (05/22/96)
*/
#if __STD_C
uint dtstrhash(reg uint h, Void_t* args, reg int n)
#else
uint dtstrhash(h,args,n)
reg uint	h;
Void_t*		args;
reg int		n;
#endif
{
	reg unsigned char*	s = (unsigned char*)args;

	if(n <= 0)
	{	for(; (n = *s) != 0; ++s)
			h = dtcharhash(h,n);
	}
	else
	{	reg unsigned char*	ends;
		for(ends = s+n; s < ends; ++s)
		{	n = *s;
			h = dtcharhash(h,n);
		}
	}

	return h;
}
