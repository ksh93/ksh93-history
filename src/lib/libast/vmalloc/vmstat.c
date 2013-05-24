/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1985-2013 AT&T Intellectual Property          *
*                      and is licensed under the                       *
*                 Eclipse Public License, Version 1.0                  *
*                    by AT&T Intellectual Property                     *
*                                                                      *
*                A copy of the License is available at                 *
*          http://www.eclipse.org/org/documents/epl-v10.html           *
*         (with md5 checksum b35adb5213ca9657e911e9befb180842)         *
*                                                                      *
*              Information and Software Systems Research               *
*                            AT&T Research                             *
*                           Florham Park NJ                            *
*                                                                      *
*                 Glenn Fowler <gsf@research.att.com>                  *
*                  David Korn <dgk@research.att.com>                   *
*                     Phong Vo <phongvo@gmail.com>                     *
*                                                                      *
***********************************************************************/
#if defined(_UWIN) && defined(_BLD_ast)

void _STUB_vmstat(){}

#else

#include	"vmhdr.h"

/*	Get statistics of a region.
**
**	Written by Kiem-Phong Vo, phongvo@gmail.com, 01/16/94, 03/31/2012.
*/

#if __STD_C
int vmstat(Vmalloc_t* vm, Vmstat_t* st)
#else
int vmstat(vm, st)
Vmalloc_t*	vm;
Vmstat_t*	st;
#endif
{
	Seg_t	*seg;
	char	*bufp;
	ssize_t	z, p;
	int	rv;

	if(!st)
		return _vmheapbusy();
	if(!vm) /* getting stats for Vmregion */
	{	if(_vmheapinit(Vmheap) != Vmheap) /* initialize heap if not done yet */
			return -1;
		vm = Vmregion;
	}

	memset(st, 0, sizeof(Vmstat_t));
	for(seg = vm->data->seg; seg; seg = seg->next)
	{	st->n_seg += 1;
		st->extent += seg->size;
	}
	if((rv = (*vm->meth.statf)(vm, st, 0)) >= 0 )
	{	
		bufp = st->mesg;
		bufp = (*_Vmstrcpy)(bufp, "region(size", '=');
		bufp = (*_Vmstrcpy)(bufp, (*_Vmitoa)(VMLONG(st->extent),-1), ',');

		bufp = (*_Vmstrcpy)(bufp, "segs", '=');
		bufp = (*_Vmstrcpy)(bufp, (*_Vmitoa)(VMLONG(st->n_seg),-1), ',');

		bufp = (*_Vmstrcpy)(bufp, "packs", '=');
		bufp = (*_Vmstrcpy)(bufp, (*_Vmitoa)(VMLONG(st->n_pack),-1), ',');

		bufp = (*_Vmstrcpy)(bufp, "\n\t\tbusy(n", '=');
		bufp = (*_Vmstrcpy)(bufp, (*_Vmitoa)(VMLONG(st->n_busy),-1), ',');

		bufp = (*_Vmstrcpy)(bufp, "size", '=');
		z = st->s_busy;
		p = (ssize_t)((((double)z)/((double)st->extent))*100 + 0.5);
		bufp = (*_Vmstrcpy)(bufp, (*_Vmitoa)(VMLONG(st->s_busy),-1), '[');
		bufp = (*_Vmstrcpy)(bufp, (*_Vmitoa)(VMLONG(p),-1), '%');
		bufp = (*_Vmstrcpy)(bufp, "]", ',');

		bufp = (*_Vmstrcpy)(bufp, "size+head", '=');
		z = st->s_busy + st->n_busy*sizeof(Head_t);
		p = (ssize_t)((((double)z)/((double)st->extent))*100 + 0.5);
		bufp = (*_Vmstrcpy)(bufp, (*_Vmitoa)(VMLONG(z),-1), '[');
		bufp = (*_Vmstrcpy)(bufp, (*_Vmitoa)(VMLONG(p),-1), '%');
		bufp = (*_Vmstrcpy)(bufp, "]", ')');

		bufp = (*_Vmstrcpy)(bufp, "\n\t\tfree(n", '=');
		bufp = (*_Vmstrcpy)(bufp, (*_Vmitoa)(VMLONG(st->n_free),-1), ',');

		bufp = (*_Vmstrcpy)(bufp, "size", '=');
		z = st->s_free;
		p = (ssize_t)((((double)z)/((double)st->extent))*100 + 0.5);
		bufp = (*_Vmstrcpy)(bufp, (*_Vmitoa)(VMLONG(st->s_free),-1), '[');
		bufp = (*_Vmstrcpy)(bufp, (*_Vmitoa)(VMLONG(p),-1), '%');
		bufp = (*_Vmstrcpy)(bufp, "]", ',');

		bufp = (*_Vmstrcpy)(bufp, "size+head", '=');
		z = st->s_free + st->n_free*sizeof(Head_t);
		p = (ssize_t)((((double)z)/((double)st->extent))*100 + 0.5);
		bufp = (*_Vmstrcpy)(bufp, (*_Vmitoa)(VMLONG(z),-1), '[');
		bufp = (*_Vmstrcpy)(bufp, (*_Vmitoa)(VMLONG(p),-1), '%');
		bufp = (*_Vmstrcpy)(bufp, "]", ')');

		bufp = (*_Vmstrcpy)(bufp, "\n\t\tcache(n", '=');
		bufp = (*_Vmstrcpy)(bufp, (*_Vmitoa)(VMLONG(st->n_cache),-1), ',');
		bufp = (*_Vmstrcpy)(bufp, "size", '=');
		bufp = (*_Vmstrcpy)(bufp, (*_Vmitoa)(VMLONG(st->s_cache),-1), ')');

		*bufp = 0;

		st->mode = vm->data->mode;
	}

	return rv;
}

#endif
