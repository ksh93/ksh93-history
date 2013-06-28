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
#include	"vmhdr.h"

/* The below implements the discipline Vmdcsystem and the heap region Vmheap.
** There are 6 alternative ways to get raw memory:
**	win32, sbrk, safe_sbrk, mmap_anon, mmap_zero and reusing the native malloc
**
** Written by Kiem-Phong Vo, phongvo@gmail.com, 03/31/2012
*/

#define FD_INIT		(-1)		/* uninitialized file desc	*/

typedef struct Memdisc_s
{	Vmdisc_t	disc;
	int		fd;
	off_t		offset;
} Memdisc_t;

#define STR(x)		#x
#define XTR(x)		STR(x)

#define GETMEMCHK(vm,caddr,csize,nsize,disc) \
	/**/DEBUG_ASSERT(csize > 0 || nsize > 0); \
	if((csize > 0 && !caddr) || (csize == 0 && nsize == 0)) \
		return NIL(Void_t*)

#define GETMEMUSE(getmem,disc) \
	if(_Vmassert & VM_verbose) debug_printf(2,"vmalloc: getmemory=%s\n",XTR(getmem)); \
	(disc)->memoryf = _Vmemoryf = (getmem)

static Vmemory_f	_Vmemoryf = 0;

#if _std_malloc
#undef	_mem_mmap_anon
#undef	_mem_mmap_zero
#undef	_mem_sbrk
#undef	_mem_win32
#endif

#if _mem_win32
#undef	_mem_mmap_anon
#undef	_mem_mmap_zero
#undef	_mem_sbrk
#endif

#if _mem_mmap_anon || _mem_mmap_zero /* may get space using mmap */
#include		<sys/mman.h>
#ifndef MAP_ANON
#ifdef	MAP_ANONYMOUS
#define	MAP_ANON	MAP_ANONYMOUS
#else
#define MAP_ANON	0
#endif /*MAP_ANONYMOUS*/
#endif /*MAP_ANON*/
#endif /*_mem_mmap_anon || _mem_mmap_zero*/

#if _mem_win32 /* getting memory on a window system */
#if _PACKAGE_ast
#include	<ast_windows.h>
#else
#include	<windows.h>
#endif

static Void_t* win32mem(Vmalloc_t* vm, Void_t* caddr, size_t csize, size_t nsize, Vmdisc_t* disc)
{	
	GETMEMCHK(vm, caddr, csize, nsize, disc);
	if(csize == 0)
	{	caddr = (Void_t*)VirtualAlloc(0,nsize,MEM_COMMIT,PAGE_READWRITE);
		return caddr;
	}
	else if(nsize == 0)
	{	(void)VirtualFree((LPVOID)caddr,0,MEM_RELEASE);
		return caddr;
	}
	else	return NIL(Void_t*);
}
#endif /* _mem_win32 */

#if _mem_sbrk /* getting space via sbrk() */
/*
** this may be unsafe due to code external to Vmalloc that may also use sbrk()
*/
static Void_t* sbrkmem(Vmalloc_t* vm, Void_t* caddr, size_t csize, size_t nsize, Vmdisc_t* disc)
{
	Vmuchar_t*	newm;
	unsigned int	key;

	GETMEMCHK(vm, caddr, csize, nsize, disc);
	if((csize%_Vmpagesize) != 0)
		return NIL(Void_t*); /* bad call! */
	else if((nsize = ROUND(nsize, _Vmpagesize)) == csize )
		return caddr; /* nothing to do */
	else if(nsize < csize) /* no memory reduction */
		return NIL(Void_t*);

	key = asothreadid(); /**/DEBUG_ASSERT(key > 0);
	asolock(&_Vmsbrklock, key, ASO_LOCK);

	nsize -= csize; /* amount of new memory needed */

	if(csize > 0 && (newm = sbrk(0)) != ((Vmuchar_t*)caddr + csize) )
		newm = NIL(Void_t*); /* non-contiguous memory */
	else if(!(newm = sbrk(nsize)) || newm == (Vmuchar_t*)(-1) )
		newm = NIL(Void_t*); /* sbrk() failed */
	else if(csize > 0 && newm != ((Vmuchar_t*)caddr + csize + nsize) )
		newm = NIL(Void_t*); /* non-contiguous memory again */

	asolock(&_Vmsbrklock, key, ASO_UNLOCK);

	return (newm && caddr) ? caddr : newm;
}
#endif /* _mem_sbrk */

#if _mem_mmap_anon /* getting space via mmap(MAP_ANON) emulation of sbrk() */
/*
** concurrency-safe memory allocation via mmap-anonymous providing an emulation of sbrk()
*/
static Void_t* safebrkmem(Vmalloc_t* vm, Void_t* caddr, size_t csize, size_t nsize, Vmdisc_t* disc)
{
	Vmuchar_t*	newm;
	unsigned int	key;

	GETMEMCHK(vm, caddr, csize, nsize, disc);
	if((csize%_Vmpagesize) != 0)
		return NIL(Void_t*); /* bad call! */
	else if((nsize = ROUND(nsize, _Vmpagesize)) == csize )
		return caddr; /* nothing to do */
	else if(nsize < csize) /* no memory reduction */
		return NIL(Void_t*);

	key = asothreadid(); /**/DEBUG_ASSERT(key > 0);
	asolock(&_Vmsbrklock, key, ASO_LOCK);

	newm = NIL(Vmuchar_t*);
	nsize -= csize; /* amount of new memory needed */

	VMBOUNDARIES();
	if(_Vmmemsbrk && _Vmmemsbrk < _Vmmemmax)
	{	if(_Vmchkmem) /* mmap must use MAP_FIXED (eg, solaris) */
		{	for(;;) /* search for a mappable address */
			{	newm = (*_Vmchkmem)(_Vmmemsbrk, nsize) ? _Vmmemsbrk : NIL(Vmuchar_t*);
				if(csize > 0 && newm != ((Vmuchar_t*)caddr+csize) )
				{	newm = NIL(Vmuchar_t*);
					break;
				}
				else if(newm)
					break;
				else	_Vmmemsbrk += nsize;
			}
			if(newm)
			{	int flags = MAP_ANON|MAP_PRIVATE|MAP_FIXED;
				newm = (Void_t*)mmap((Void_t*)newm, nsize, PROT_READ|PROT_WRITE, flags, -1, 0);
			}
		}
		else	newm = (Void_t*)mmap((Void_t*)_Vmmemsbrk, nsize, PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE, -1, 0);

		if(!newm || newm == (Vmuchar_t*)(-1))
			newm = NIL(Void_t*);
		else if(csize > 0 && newm != ((Vmuchar_t*)caddr+csize)) /* new memory is not contiguous */
		{	munmap((Void_t*)newm, nsize); /* remove it and wait for a call for new memory */
			newm = NIL(Void_t*);
		}
		else	_Vmmemsbrk = newm+nsize;
	}

	if(newm == (Vmuchar_t*)(-1) )
		newm = NIL(Vmuchar_t*);

	asolock(&_Vmsbrklock, key, ASO_UNLOCK);

	return (newm && caddr) ? caddr : newm;
}
#endif /* _mem_mmap_anon */

#if _mem_mmap_anon /* getting space via mmap(MAP_ANON) */
static Void_t* mmapanonmem(Vmalloc_t* vm, Void_t* caddr, size_t csize, size_t nsize, Vmdisc_t* disc)
{	
	GETMEMCHK(vm, caddr, csize, nsize, disc);
	if(csize == 0)
	{	nsize = ROUND(nsize, _Vmpagesize);
		caddr = (Void_t*)mmap(NIL(Void_t*), nsize, PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE, -1, 0);
		if(caddr == (Void_t*)(-1))
			caddr = NIL(Void_t*);
		return caddr;
	}
	else if(nsize == 0)
	{	(void)munmap(caddr, csize);
		return caddr;
	}
	else	return NIL(Void_t*);
}
#endif /* _mem_mmap_anon */

#if _mem_mmap_zero /* get space by mmapping from /dev/zero */
#include		<fcntl.h>
#ifndef OPEN_MAX
#define	OPEN_MAX	64
#endif
#define FD_PRIVATE	(3*OPEN_MAX/4)	/* private file descriptor	*/
#define FD_NONE		(-2)		/* no mapping with file desc	*/

/* this is called after an initial successful call of mmapzeromeminit() */
static Void_t* mmapzeromem(Vmalloc_t* vm, Void_t* caddr, size_t csize, size_t nsize, Vmdisc_t* disc)
{
	Memdisc_t*	mmdc = (Memdisc_t*)disc;
	off_t		offset;

	GETMEMCHK(vm, caddr, csize, nsize, disc);
	if(csize == 0)
	{	nsize = ROUND(nsize, _Vmpagesize);
		offset = asoaddoff(&mmdc->offset, nsize);
		caddr = mmap(NIL(Void_t*), nsize, PROT_READ|PROT_WRITE, MAP_PRIVATE, mmdc->fd, offset);
		if(caddr == (Void_t*)(-1))
			caddr = NIL(Void_t*);
		return caddr;
	}
	else if(nsize == 0)
	{	Vmuchar_t	*addr = (Vmuchar_t*)sbrk(0);
		if(addr < (Vmuchar_t*)caddr ) /* in sbrk space */
			return NIL(Void_t*);
		(void)munmap(caddr, csize);
		return caddr;
	}
	else	return NIL(Void_t*);
}

/* if this call succeeds then mmapzeromem() is the implementation */
static Void_t* mmapzeromeminit(Vmalloc_t* vm, Void_t* caddr, size_t csize, size_t nsize, Vmdisc_t* disc)
{
	Memdisc_t*	mmdc = (Memdisc_t*)disc;
	int		fd;

	GETMEMCHK(vm, caddr, csize, nsize, disc);
	if(mmdc->fd != FD_INIT)
		return NIL(Void_t*);
	if((fd = open("/dev/zero", O_RDONLY|O_CLOEXEC)) < 0)
	{	mmdc->fd = FD_NONE;
		return NIL(Void_t*);
	}
#if O_CLOEXEC == 0
	else
		SETCLOEXEC(fd);
#endif
	if(fd >= FD_PRIVATE || (mmdc->fd = fcntl(fd, F_DUPFD_CLOEXEC, FD_PRIVATE)) < 0)
		mmdc->fd = fd;
	else
	{	close(fd);
#if F_DUPFD_CLOEXEC == F_DUPFD
		SETCLOEXEC(mmdc->fd);
#endif
	}
	if(!(caddr = mmapzeromem(vm, caddr, csize, nsize, disc)))
	{	close(mmdc->fd);
		mmdc->fd = FD_NONE;
	}
	return caddr;
}
#endif /* _mem_mmap_zero */

#if _std_malloc /* using native malloc as a last resort */
static Void_t* mallocmem(Vmalloc_t* vm, Void_t* caddr, size_t csize, size_t nsize, Vmdisc_t* disc)
{
	GETMEMCHK(vm, caddr, csize, nsize, disc);
	if(csize == 0)
		return (Void_t*)malloc(nsize);
	else if(nsize == 0)
	{	free(caddr);
		return caddr;
	}
	else	return NIL(Void_t*);
}
#endif

/* A discipline to get raw memory */
static Void_t* getmemory(Vmalloc_t* vm, Void_t* caddr, size_t csize, size_t nsize, Vmdisc_t* disc)
{
	Vmuchar_t*	addr;

	if(_Vmemoryf)
	{	(disc)->memoryf = _Vmemoryf;
		return (*_Vmemoryf)(vm, caddr, csize, nsize, disc);
	}
	GETMEMCHK(vm, caddr, csize, nsize, disc);
#if _mem_mmap_anon
	if((_Vmassert & VM_anon) && (addr = mmapanonmem(vm, caddr, csize, nsize, disc)))
	{	GETMEMUSE(mmapanonmem, disc);
		return (Void_t*)addr;
	}
#endif
#if _mem_mmap_zero
	if((_Vmassert & VM_zero) && (addr = mmapzeromeminit(vm, caddr, csize, nsize, disc)))
	{	GETMEMUSE(mmapzeromem, disc);
		return (Void_t*)addr;
	}
#endif
#if _mem_mmap_anon
	if((_Vmassert & VM_safe) && (addr = safebrkmem(vm, caddr, csize, nsize, disc)))
	{	GETMEMUSE(safebrkmem, disc);
		return (Void_t*)addr;
	}
#endif
#if _mem_sbrk
	if((_Vmassert & VM_break) && (addr = sbrkmem(vm, caddr, csize, nsize, disc)))
	{	GETMEMUSE(sbrkmem, disc);
		return (Void_t*)addr;
	}
#endif
#if _mem_win32
	if((addr = win32mem(vm, caddr, csize, nsize, disc)))
	{	GETMEMUSE(win32mem, disc);
		return (Void_t*)addr;
	}
#endif
#if _std_malloc
	if((_Vmassert & VM_native) && (addr = mallocmem(vm, caddr, csize, nsize, disc)))
	{	GETMEMUSE(mallocmem, disc);
		return (Void_t*)addr;
#endif 
	write(2, "vmalloc: panic: all memory allocation disciplines failed\n", 57);
	abort();
	return NIL(Void_t*);
}

static Memdisc_t _Vmdcsystem = { { getmemory, NIL(Vmexcept_f), VM_INCREMENT, sizeof(Memdisc_t) }, FD_INIT, 0 };

__DEFINE__(Vmdisc_t*,  Vmdcsystem, (Vmdisc_t*)(&_Vmdcsystem) );
__DEFINE__(Vmdisc_t*,  Vmdcsbrk, (Vmdisc_t*)(&_Vmdcsystem) );

/* Note that the below function may be invoked from multiple threads.
** It initializes the Vmheap region to use the VMHEAPMETH method.
*/
#define	HEAPINIT	(1)
#define	HEAPDONE	(2)
#define VMHEAPMETH	Vmbest

static unsigned int	Init = 0;

int _vmheapbusy(void)
{
	return asogetint(&Init) == HEAPINIT;
}

Vmalloc_t* _vmheapinit(Vmalloc_t* vm)
{	
	Vmalloc_t		*heap;
	unsigned int		status;

	/**/DEBUG_ASSERT(!vm /* called from _vmstart() in malloc.c */ || vm == Vmheap);

	if((status = asocasint(&Init, 0, HEAPINIT)) == HEAPDONE)
		return Vmheap;
	else if(status == HEAPINIT) /* someone else is doing initialization */
	{	asospindecl(); /* so we wait until that is done */
		for(asospininit();; asospinnext() )
			if((status = asogetint(&Init)) != HEAPINIT)
				break;
		/**/DEBUG_ASSERT(status == HEAPDONE);
		return Vmheap;
	}
	else /* we won the right to initialize Vmheap below */
	{	/**/DEBUG_ASSERT(status == 0);
		/**/DEBUG_ASSERT(Init == HEAPINIT);
		_vmoptions(3);
	}

	heap = vmopen(Vmheap->disc, VMHEAPMETH, VM_HEAPINIT);
	if(!vm && heap != Vmheap)
	{	if(heap)
			write(9, "\n\nFATAL: _vmheapinit() != Vmheap\n\n", 34);
		else
			write(9, "\n\nFATAL: _vmheapinit() == 0\n\n", 29);
	}

	/**/DEBUG_ASSERT(Init == HEAPINIT);
	asocasint(&Init, HEAPINIT, heap == Vmheap ? HEAPDONE : 0);

	if(vm == Vmheap && Init == HEAPDONE) /* initialize malloc/free/realloc interface */
		free(NIL(Void_t*));

	return heap;
}

static Void_t* heapalloc(Vmalloc_t* vm, size_t size, int local)
{	return (vm = _vmheapinit(vm)) ? (*vm->meth.allocf)(vm, size, local) : NIL(Void_t*);
}
static Void_t* heapresize(Vmalloc_t* vm, Void_t* obj, size_t size, int type, int local)
{	return (vm = _vmheapinit(vm)) ? (*vm->meth.resizef)(vm, obj, size, type, local) : NIL(Void_t*);
}
static int heapfree(Vmalloc_t* vm, Void_t* obj, int local)
{	return (vm = _vmheapinit(vm)) ? (*vm->meth.freef)(vm, obj, local) : -1;
}
static Void_t* heapalign(Vmalloc_t* vm, size_t size, size_t align, int local)
{	return (vm = _vmheapinit(vm)) ? (*vm->meth.alignf)(vm, size, align, local) : NIL(Void_t*);
}
static int heapstat(Vmalloc_t* vm, Vmstat_t* st, int local)
{	return (vm = _vmheapinit(vm)) ? (*vm->meth.statf)(vm, st, local) : -1;
}

Vmalloc_t _Vmheap =
{	{	heapalloc,
		heapresize,
		heapfree,
		heapalign,
		heapstat,
		0,	/* eventf	*/
		0  	/* method ID	*/
	},
	NIL(char*),	/* file	name	*/
	0,		/* line number	*/
	0,		/* function	*/
	(Vmdisc_t*)(&_Vmdcsystem),
	NIL(Vmdata_t*), /* see heapinit	*/
};

__DEFINE__(Vmalloc_t*, Vmheap, &_Vmheap);
__DEFINE__(Vmalloc_t*, Vmregion, &_Vmheap);
