/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1985-2002 AT&T Corp.                *
*        and it may only be used by you under license from         *
*                       AT&T Corp. ("AT&T")                        *
*         A copy of the Source Code Agreement is available         *
*                at the AT&T Internet web site URL                 *
*                                                                  *
*       http://www.research.att.com/sw/license/ast-open.html       *
*                                                                  *
*        If you have copied this software without agreeing         *
*        to the terms of the license you are infringing on         *
*           the license and copyright and are violating            *
*               AT&T's intellectual property rights.               *
*                                                                  *
*            Information and Software Systems Research             *
*                        AT&T Labs Research                        *
*                         Florham Park NJ                          *
*                                                                  *
*               Glenn Fowler <gsf@research.att.com>                *
*                David Korn <dgk@research.att.com>                 *
*                 Phong Vo <kpv@research.att.com>                  *
*******************************************************************/
#include	"sfhdr.h"


/*	Push/pop streams
**
**	Written by Kiem-Phong Vo.
*/

#define STKMTXLOCK(f1,f2) \
	{ if(f1) SFMTXLOCK(f1); \
	  if(f2) SFMTXLOCK(f2); \
	}
#define STKMTXRETURN(f1,f2,rv) \
	{ if(f1) SFMTXUNLOCK(f1); \
	  if(f2) SFMTXUNLOCK(f2); \
	  return(rv); \
	}

#if __STD_C
Sfio_t* sfstack(Sfio_t* f1, Sfio_t* f2)
#else
Sfio_t* sfstack(f1,f2)
Sfio_t*	f1;	/* base of stack	*/
Sfio_t*	f2;	/* top of stack	*/
#endif
{
	reg int		n;
	reg Sfio_t*	rf;
	reg Sfrsrv_t*	rsrv;
	reg Vtmutex_t*	mtx;

	STKMTXLOCK(f1,f2);

	if(f1 && (f1->mode&SF_RDWR) != f1->mode && _sfmode(f1,0,0) < 0)
		STKMTXRETURN(f1,f2, NIL(Sfio_t*));
	if(f2 && (f2->mode&SF_RDWR) != f2->mode && _sfmode(f2,0,0) < 0)
		STKMTXRETURN(f1,f2, NIL(Sfio_t*));
	if(!f1)
		STKMTXRETURN(f1,f2, f2);

	/* give access to other internal functions */
	_Sfstack = sfstack;

	if(f2 == SF_POPSTACK)
	{	if(!(f2 = f1->push))
			STKMTXRETURN(f1,f2, NIL(Sfio_t*));
		f2->mode &= ~SF_PUSH;
	}
	else
	{	if(f2->push)
			STKMTXRETURN(f1,f2, NIL(Sfio_t*));
		if(f1->pool && f1->pool != &_Sfpool && f1->pool != f2->pool &&
		   f1 == f1->pool->sf[0])
		{	/* get something else to pool front since f1 will be locked */
			for(n = 1; n < f1->pool->n_sf; ++n)
			{	if(SFFROZEN(f1->pool->sf[n]) )
					continue;
				(*_Sfpmove)(f1->pool->sf[n],0);
				break;
			}
		}
	}

	if(f2->pool && f2->pool != &_Sfpool && f2 != f2->pool->sf[0])
		(*_Sfpmove)(f2,0);

	/* swap streams */
	sfswap(f1,f2);

	/* but the reserved buffer and mutex must remain the same */
	rsrv = f1->rsrv; f1->rsrv = f2->rsrv; f2->rsrv = rsrv;
	mtx = f1->mutex; f1->mutex = f2->mutex; f2->mutex = mtx;

	SFLOCK(f1,0);
	SFLOCK(f2,0);

	if(f2->push != f2)
	{	/* freeze the pushed stream */
		f2->mode |= SF_PUSH;
		f1->push = f2;
		rf = f1;
	}
	else
	{	/* unfreeze the just exposed stream */
		f1->mode &= ~SF_PUSH;
		f2->push = NIL(Sfio_t*);
		rf = f2;
	}

	SFOPEN(f1,0);
	SFOPEN(f2,0);

	STKMTXRETURN(f1,f2, rf);
}
