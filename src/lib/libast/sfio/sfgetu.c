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
#include	"sfhdr.h"

/*	Read an unsigned long value coded in a portable format.
**
**	Written by Kiem-Phong Vo (06/27/90)
*/

#if __STD_C
Sfulong_t _sfgetu(reg Sfio_t* f)
#else
Sfulong_t _sfgetu(f)
reg Sfio_t*	f;
#endif
{
	Sfulong_t	v;
	reg uchar	*s, *ends, c;
	reg int		p;

	if(f->mode != SF_READ && _sfmode(f,SF_READ,0) < 0)
		return (ulong)(-1L);

	SFLOCK(f,0);

	v = SFUVALUE(f->val);
	for(;;)
	{	if(SFRPEEK(f,s,p) <= 0)
		{	f->flags |= SF_ERROR;
			v = (ulong)(-1L);
			goto done;
		}
		for(ends = s+p; s < ends;)
		{	c = *s++;
			v = (v << SF_UBITS) | SFUVALUE(c);
			if(!(c&SF_MORE))
			{	f->next = s;
				goto done;
			}
		}
		f->next = s;
	}
done:
	SFOPEN(f,0);
	return v;
}
