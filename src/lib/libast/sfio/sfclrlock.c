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

/*	Function to clear a locked stream.
**	This is useful for programs that longjmp from the mid of an sfio function.
**	There is no guarantee on data integrity in such a case.
**
**	Written by Kiem-Phong Vo (07/20/90).
*/
#if __STD_C
int sfclrlock(reg Sfio_t* f)
#else
int sfclrlock(f)
reg Sfio_t	*f;
#endif
{
	/* already closed */
	if(f->mode&SF_AVAIL)
		return 0;

	if(f->pool) /* clear pool lock */
		f->pool->mode &= ~SF_LOCK;

	/* clear error bits */
	f->flags &= ~(SF_ERROR|SF_EOF);

	if(!(f->mode&(SF_LOCK|SF_PEEK)) )
		return (f->flags&SF_FLAGS);

	/* clear peek locks */
	f->mode &= ~SF_PEEK;
	if(f->mode&SF_PKRD)
	{	f->here -= f->endb-f->next;
		f->endb = f->next;
		f->mode &= ~SF_PKRD;
	}

	f->mode &= (SF_RDWR|SF_INIT|SF_POOL|SF_PUSH|SF_SYNCED|SF_STDIO);

	SFCLRBITS(f);

	return _sfmode(f,0,0) < 0 ? 0 : (f->flags&SF_FLAGS);
}
