/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1985-2001 AT&T Corp.                *
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
*                 This software was created by the                 *
*                 Network Services Research Center                 *
*                        AT&T Labs Research                        *
*                         Florham Park NJ                          *
*                                                                  *
*               Glenn Fowler <gsf@research.att.com>                *
*                David Korn <dgk@research.att.com>                 *
*                 Phong Vo <kpv@research.att.com>                  *
*******************************************************************/
#include	"dthdr.h"

/*	Close a dictionary
**
**	Written by Kiem-Phong Vo (05/25/96)
*/
#if __STD_C
int dtclose(reg Dt_t* dt)
#else
int dtclose(dt)
reg Dt_t*	dt;
#endif
{
	if(dt->nview > 0 ) /* can't close if being viewed */
		return -1;

	if(dt->view)	/* turn off viewing */
		dtview(dt,NIL(Dt_t*));

	/* announce the close event */
	if(dt->disc->eventf &&
	   (*dt->disc->eventf)(dt,DT_CLOSE,NIL(Void_t*),dt->disc) < 0)
		return -1;

	/* release all allocated data */
	(void)(*(dt->meth->searchf))(dt,NIL(Void_t*),DT_CLEAR);
	if(dtsize(dt) > 0)
		return -1;

	if(dt->data->ntab > 0)
		(*dt->memoryf)(dt,(Void_t*)dt->data->htab,0,dt->disc);
	(*dt->memoryf)(dt,(Void_t*)dt->data,0,dt->disc);

	free((Void_t*)dt);

	return 0;
}
