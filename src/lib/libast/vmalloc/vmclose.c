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
#ifdef _UWIN

void _STUB_vmclose(){}

#else

#include	"vmhdr.h"

/*	Close down a region.
**
**	Written by Kiem-Phong Vo, kpv@research.att.com, 01/16/94.
*/
#if __STD_C
int vmclose(Vmalloc_t* vm)
#else
int vmclose(vm)
Vmalloc_t*	vm;
#endif
{
	reg Seg_t	*seg, *vmseg;
	reg Vmemory_f	memoryf;
	reg Vmdata_t*	vd = vm->data;
	reg Vmalloc_t	*v, *last;

	if(vm == Vmheap)
		return -1;

	if(!(vd->mode&VM_TRUST) && ISLOCK(vd,0))
		return -1;

	if(vm->disc->exceptf &&
	   (*vm->disc->exceptf)(vm,VM_CLOSE,NIL(Void_t*),vm->disc) < 0)
		return -1;

	/* make this region inaccessible until it disappears */
	vd->mode &= ~VM_TRUST;
	SETLOCK(vd,0);

	if((vd->mode&VM_MTPROFILE) && _Vmpfclose)
		(*_Vmpfclose)(vm);

	/* remove from linked list of regions	*/
	for(last = Vmheap, v = last->next; v; last = v, v = v->next)
	{	if(v == vm)
		{	last->next = v->next;
			break;
		}
	}

	/* free all non-region segments */
	memoryf = vm->disc->memoryf;
	vmseg = NIL(Seg_t*);
	for(seg = vd->seg; seg; )
	{	reg Seg_t*	next = seg->next;
		if(seg->extent != seg->size)
			(void)(*memoryf)(vm,seg->addr,seg->extent,0,vm->disc);
		else	vmseg = seg;
		seg = next;
	}

	/* this must be done here because even though this region is freed,
	   there may still be others that share this space.
	*/
	CLRLOCK(vd,0);

	/* free the segment that contains the region data */
	if(vmseg)
		(void)(*memoryf)(vm,vmseg->addr,vmseg->extent,0,vm->disc);

	/* free the region itself */
	vmfree(Vmheap,vm);

	return 0;
}

#endif
