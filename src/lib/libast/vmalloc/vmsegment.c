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
#ifdef _UWIN

int _STUB_vmsegment;

#else

#include	"vmhdr.h"

/*	Get the segment containing this address
**
**	Written by Kiem-Phong Vo, kpv@research.att.com, 02/07/95
*/

#if __STD_C
Void_t* vmsegment(Vmalloc_t* vm, Void_t* addr)
#else
Void_t* vmsegment(vm, addr)
Vmalloc_t*	vm;	/* region	*/
Void_t*		addr;	/* address	*/
#endif
{
	reg Seg_t*	seg;
	reg Vmdata_t*	vd = vm->data;

	if(!(vd->mode&VM_TRUST))
	{	if(ISLOCK(vd,0))
			return NIL(Void_t*);
		SETLOCK(vd,0);
	}

	for(seg = vd->seg; seg; seg = seg->next)
		if((Vmuchar_t*)addr >= (Vmuchar_t*)seg->addr &&
		   (Vmuchar_t*)addr <  (Vmuchar_t*)seg->baddr )
			break;

	CLRLOCK(vd,0);
	return seg ? (Void_t*)seg->addr : NIL(Void_t*);
}

#endif
