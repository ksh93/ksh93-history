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

/*	Read a long value coded in a portable format.
**
**	Written by Kiem-Phong Vo (06/27/90)
*/

#if __STD_C
Sflong_t _sfgetl(reg Sfio_t* f)
#else
Sflong_t _sfgetl(f)
reg Sfio_t*	f;
#endif
{
	Sflong_t	v;
	reg uchar	*s, *ends, c;
	reg int		p;

	if(f->mode != SF_READ && _sfmode(f,SF_READ,0) < 0)
		return (Sflong_t)(-1);
	SFLOCK(f,0);

	v = (Sflong_t)f->val;
	if(!(v&SF_MORE))
	{	/* must be a small negative number */
		v = -SFSVALUE(v)-1;
		goto done;
	}

	v = SFUVALUE(v);
	for(;;)
	{	if(SFRPEEK(f,s,p) <= 0)
		{	f->flags |= SF_ERROR;
			v = (Sflong_t)(-1);
			goto done;
		}
		for(ends = s+p; s < ends;)
		{	c = *s++;
			if(c&SF_MORE)
				v = ((Sfulong_t)v << SF_UBITS) | SFUVALUE(c);
			else
			{	/* special translation for this byte */
				v = ((Sfulong_t)v << SF_SBITS) | SFSVALUE(c);
				f->next = s;
				v = (c&SF_SIGN) ? -v-1 : v;
				goto done;
			}
		}
		f->next = s;
	}
done :
	SFOPEN(f,0);
	return v;
}
