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
** There are 5 alternative ways to get raw memory:
**	win32, sbrk, mmap_anon, mmap_zero and reusing the native malloc
**
** Written by Kiem-Phong Vo, phongvo@gmail.com, 03/31/2012
*/

#define FD_INIT		(-1)		/* uninitialized file desc	*/

typedef struct _memdisc_s
{	Vmdisc_t	disc;
	int		fd;
	off_t		offset;
} Memdisc_t;

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
#ifdef MAP_ANONYMOUS
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

static Void_t* win32mem(Void_t* caddr, size_t csize, size_t nsize)
{	/**/DEBUG_ASSERT(csize > 0 || nsize > 0);
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

#if _mem_mmap_anon || _mem_sbrk /* getting space via mmap(MAP_ANON) or brk/sbrk */

/* The below provides concurrency-safe memory allocation via either sbrk()
** or mmap-anonymous. The former may be unsafe due to code external to
** Vmalloc that may also use sbrk().
*/
static Void_t* mmapanonmem(Void_t* caddr, size_t csize, size_t nsize, int usesbrk )
{
	Vmuchar_t	*newm;
	unsigned int	key;

	if(usesbrk)
		_Vmmemsbrk = NIL(Vmuchar_t*);

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

#if _mem_mmap_anon
	if(!usesbrk && !newm && _Vmmemsbrk && _Vmmemsbrk < _Vmmemmax)
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
			newm = (Vmuchar_t*)(-1); /* error state -- so that sbrk() will be avoided */
		}
		else	_Vmmemsbrk = newm+nsize;
	}
#endif /*_mem_mmap_anon*/

#if _mem_sbrk
	if(usesbrk)
	{	if(!newm)
		{	if(csize > 0 && (newm = sbrk(0)) != ((Vmuchar_t*)caddr + csize) )
				newm = (Vmuchar_t*)(-1); /* non-contiguous memory */
			else if(!(newm = sbrk(nsize)) || newm == (Vmuchar_t*)(-1) )
				newm = (Vmuchar_t*)(-1); /* sbrk() failed */
			else if(csize > 0 && newm != ((Vmuchar_t*)caddr + csize) )
				newm = (Vmuchar_t*)(-1); /* non-contiguous memory again */
		}
	}
#endif /*_mem_sbrk*/

	if(newm == (Vmuchar_t*)(-1) )
		newm = NIL(Vmuchar_t*);

	asolock(&_Vmsbrklock, key, ASO_UNLOCK);

	return (newm && caddr) ? caddr : newm;
}
#endif /* _mem_mmap_anon || _mem_sbrk */

#if _mem_mmap_zero /* get space by mmapping from /dev/zero */
#include		<fcntl.h>
#ifndef OPEN_MAX
#define	OPEN_MAX	64
#endif
#define FD_PRIVATE	(3*OPEN_MAX/4)	/* private file descriptor	*/
#define FD_NONE		(-2)		/* no mapping with file desc	*/

static Void_t* mmapzeromem(Void_t* caddr, size_t csize, size_t nsize, Memdisc_t* mmdc)
{
	if(mmdc->fd == FD_INIT ) /* open /dev/zero for mapping */
	{	int	fd;
		if((fd = open("/dev/zero", O_RDONLY)) < 0 )
		{	mmdc->fd = FD_NONE;
			return NIL(Void_t*);
		}
		if(fd >= FD_PRIVATE || (mmdc->fd = dup2(fd, FD_PRIVATE)) < 0 )
			mmdc->fd = fd;
		else	close(fd);
#ifdef FD_CLOEXEC
		fcntl(mmdc->fd,  F_SETFD, FD_CLOEXEC);
#endif
	}

	if(mmdc->fd == FD_NONE)
		return NIL(Void_t*);

	/**/DEBUG_ASSERT(csize > 0 || nsize > 0);
	if(csize == 0)
	{	nsize = ROUND(nsize, _Vmpagesize);
		caddr = NIL(Void_t*);
		if(mmdc->fd >= 0 )
			caddr = mmap(0, nsize, PROT_READ|PROT_WRITE, MAP_PRIVATE, mmdc->fd, mmdc->offset);
		if(!caddr || caddr == (Void_t*)(-1))
			return NIL(Void_t*);
		else
		{	mmdc->offset += nsize;
			return caddr;
		}
	}
	else if(nsize == 0)
	{	Vmuchar_t	*addr = (Vmuchar_t*)sbrk(0);
		if(addr < (Vmuchar_t*)caddr ) /* in sbrk space */
			return NIL(Void_t*);
		else
		{	(void)munmap(caddr, csize);
			return caddr;
		}
	}
	else	return NIL(Void_t*);
}
#endif /* _mem_mmap_zero */

#if _std_malloc /* using native malloc as a last resource */
static Void_t* mallocmem(Void_t* caddr, size_t csize, size_t nsize)
{
	/**/DEBUG_ASSERT(csize > 0 || nsize > 0);
	if(csize == 0)
		return (Void_t*)malloc(nsize);
	else if(nsize == 0)
	{	free(caddr);
		return caddr;
	}
	else	return NIL(Void_t*);
}
#endif

/* A discipline to get raw memory using VirtualAlloc/mmap/sbrk */
static Void_t* getmemory(Vmalloc_t* vm, Void_t* caddr, size_t csize, size_t nsize, Vmdisc_t* disc)
{
	Vmuchar_t	*addr;

	if((csize > 0 && !caddr) || (csize == 0 && nsize == 0) )
		return NIL(Void_t*);

#if _mem_mmap_anon
	if(!(_Vmassert & VM_break) && (addr = mmapanonmem(caddr, csize, nsize, 0)) )
		return (Void_t*)addr;
#endif
#if _mem_sbrk
	if((_Vmassert & VM_break) && (addr = mmapanonmem(caddr, csize, nsize, 1)) )
		return (Void_t*)addr;
#endif
#if _mem_mmap_zero
	if((addr = mmapzeromem(caddr, csize, nsize, (Memdisc_t*)disc)) )
		return (Void_t*)addr;
#endif
#if _mem_win32
	if((addr = win32mem(caddr, csize, nsize)) )
		return (Void_t*)addr;
#endif
#if _std_malloc
	if((addr = mallocmem(caddr, csize, nsize)) )
		return (Void_t*)addr;
#endif 
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
	}

#if DEBUG /* trace all allocation calls through the heap */
	if(!_Vmtrace)
	{	char	*env;
		int	fd;
		if((fd = vmtrace(-1)) >= 0 ||
		   ((env = getenv("VMTRACE")) && (fd = creat(env, 0666)) >= 0 ) )
			vmtrace(fd);
	}
#endif

	heap = vmopen(Vmheap->disc, VMHEAPMETH, VM_HEAPINIT);

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
