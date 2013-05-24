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
#include	<sys/types.h>
#include	<sys/mman.h>
#include	<sys/shm.h>
#include	<sys/ipc.h>
#if _mem_mmap_anon
#ifndef MAP_ANON
#ifdef MAP_ANONYMOUS
#define	MAP_ANON	MAP_ANONYMOUS
#else
#define MAP_ANON	0
#endif /*MAP_ANONYMOUS*/
#endif /*MAP_ANON*/
#endif /*_mem_mmap_anon*/
#include	<signal.h>
#include	<setjmp.h>

/* Heuristic to suggest an address usable for mapping shared memory
**
** Written by Kiem-Phong Vo, phongvo@gmail.com, 07/07/2012
*/

/* see if a given range of address is available for mapping */
#define VMCHKMEM	1 /* set this to zero if signal&sigsetjmp don't work */
#if !VMCHKMEM
#define	_vmchkmem(a,z)	(1) /* beware of unbounded optimism! */
#else
typedef void(		*Sighandler_t)_ARG_((int));
static sigjmp_buf	Jmpbuf;
static int		pokec;	/* define here instead of local in _vmchkmem so that
				** the assignment statements won't be optimized away
				** by the compiler. this costs a few bytes but some
				** compiler does not support 'volatile'.
				*/
static void sigsegv(int sig)
{	
	if(sig == SIGSEGV)
	{	signal(sig, sigsegv);
		/**/DEBUG_MESSAGE("In sigsegv handler");
		siglongjmp(Jmpbuf, SIGSEGV);
	}
}
static int _vmchkmem(Vmuchar_t* area, size_t size)
{
	int		jmp;
	Sighandler_t	segvf;
	int		avail = 0;

#if !VMCHKMEM
	static int	Chkmem =  0; /* no memory check -- good for debugging	*/
#else
	static int	Chkmem = -1; /* consult environment variable VMCHKMEM	*/
	if(Chkmem < 0) /* get the environment variable to check memory */
	{	char	*chkmem;
		if((chkmem = getenv("VMCHKMEM")) && chkmem[0] == '0')
			Chkmem = 0;
		else	Chkmem = 1;
	}
#endif
	if(Chkmem == 0) /* optimistically assume memory is good */
		return 1;

	segvf = signal(SIGSEGV, sigsegv); /* set sighandler and save old one */

	if((jmp = sigsetjmp(Jmpbuf, 1)) == 0) /* poke left end of area */
		pokec = area[0]; /* if not yet mapped, SIGSEGV will be generated */
	if(jmp == 0) /* then jmp must == SIGSEGV because a longjmp happened */
		goto done; /* else jmp == 0 meaning that address is already in use */

	if((jmp = sigsetjmp(Jmpbuf, 1)) == 0) /* poke right end of area */
		pokec = area[size-1]; /* if not yet mapped, SIGSEGV will be generated */
	if(jmp == 0) /* then jmp must == SIGSEGV because a longjmp happened */
		goto done; /* else jmp == 0 meaning that address is already in use */

	if((jmp = sigsetjmp(Jmpbuf, 1)) == 0) /* poke middle of area */
		pokec = area[size/2]; /* if not yet mapped, SIGSEGV will be generated */
	if(jmp == 0) /* then jmp must == SIGSEGV because a longjmp happened */
		goto done; /* else jmp == 0 meaning that address is already in use */

	avail = 1; /* if get here, area is (optimistically) available for mapping */
		
done:	signal(SIGSEGV, segvf); /* restore old handler */
	return avail;
}
#endif /*VMCHKMEM*/

/* set page size and also parameters to get mappable addresses */
ssize_t _vmpagesize()
{
	ssize_t		page, memz, z;
	unsigned long	left, rght, size;
	int		shmid; /* shared memory id */
	Vmuchar_t	*tmp, *shm, *min, *max;

	if(_Vmpagesize > 0 )
		return _Vmpagesize;
	else
	{
#if !_lib_getpagesize
		page = VM_PAGESIZE;
#else
		if((page = getpagesize()) <= 0 )
			page = VM_PAGESIZE;
#endif
		page = (*_Vmlcm)(page, ALIGN); /* alignment requirement */
	}

#if !_WINIX
	/* try to get a shared memory segment, memz is the successful size */
	memz = sizeof(void*) < 8 ? 1024*1024 : 64*1024*1024;
	for(; memz >= page; memz /= 2)
	{	z = ROUND(memz, page);
		if((shmid = shmget(IPC_PRIVATE, z, IPC_CREAT|0600)) >= 0 ) 
			break;
	}
	if(memz >= page) /* did get a shared segment */
		memz = ROUND(memz, page);
	else
	{	/**/DEBUG_MESSAGE("shmget() failed");
		return (int)page;
	}

	/* the stack and the heap in Unix programs are conventionally set
	** at opposite ends of the available address space. So, we use them
	** as candidate boundaries for mappable memory.
	*/
	min = (Vmuchar_t*)sbrk(0); min = (Vmuchar_t*)ROUND((unsigned long)min, page); /* heap  */
	max = (Vmuchar_t*)(&max);  max = (Vmuchar_t*)ROUND((unsigned long)max, page); /* stack */
	if(min > max)
		{ tmp = min; min = max; max = tmp; }

	/* now attach a segment to see where it falls in the range */
	if(!(shm = shmat(shmid, NIL(Void_t*), 0600)) || shm == (Vmuchar_t*)(-1) )
	{	/**/DEBUG_MESSAGE("shmat() failed first NULL attachment");
		return (int)page; /* can't attach data */
	}
	else	shmdt((Void_t*)shm);
	if(shm < min || shm > max )
	{	/**/DEBUG_MESSAGE("shmat() got an out-of-range address");
		return (int)page; /* too weird */
	}

	/* Heuristic: allocate address in the larger side */
	left = shm - min;
	rght = max - shm;

	min = max = shm; /* compute bounds of known mappable memory */
	for(size = 7*(left > rght ? left : rght)/8; size > memz; size /= 2 )
	{	size = ROUND(size, page);
		shm = left > rght ? max-size : min+size;
		if((tmp = shmat(shmid, shm, 0600)) == shm )
		{	shmdt((Void_t*)tmp);
			if(left > rght)
				min = shm;
			else	max = shm;
			break;
		}
	}
	(void)shmctl(shmid, IPC_RMID, 0);

	if((min+memz) >= max ) /* no mappable region of memory */
		/**/DEBUG_MESSAGE("vmmaddress: No mappable memory region found");
	else
	{	/* search outward from last computed bound for a better bound */
		for(z = memz; z < size; z *= 2 )
		{	shm = left > rght ? min-z : max+z;
			if((tmp = shmat(shmid, shm, 0600)) == shm )
				shmdt((Void_t*)tmp);
			else /* failing to attach means at limit or close to it */
			{	if(left > rght)
					min -= z/2;
				else	max += z/2;
				break;
			}
		}

		/* amount to offset from boundaries to avoid random collisions */
		z = (max - min)/(sizeof(Void_t*) > 4 ? 4 : 8);
		z = ROUND(z, page);

		/* these are the bounds that we can use */
		_Vmmemmin = min;
		_Vmmemmax = max;

		_Vmmemaddr = max - z; /* address usable by vmmaddress() */
		_Vmmemsbrk = NIL(Vmuchar_t*); /* address usable for sbrk() simulation */

#if _mem_mmap_anon /* see if we can simulate sbrk(): memory grows from low to high */
		/* map two consecutive pages to see if they come out adjacent */
		tmp = (Void_t*)mmap((Void_t*)(min+z), page, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANON, -1, 0);
		shm = (Void_t*)mmap((Void_t*)(tmp+page), page, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANON, -1, 0);
		if(tmp && tmp != (Vmuchar_t*)(-1) )
			munmap((Void_t*)tmp, page);
		if(shm && shm != (Vmuchar_t*)(-1) )
			munmap((Void_t*)shm, page);

		if(tmp && tmp != (Vmuchar_t*)(-1) && shm && shm != (Vmuchar_t*)(-1) )
		{	_Vmmemsbrk = shm+page; /* mmap starts from here */

			if(tmp >= (_Vmmemmin + (_Vmmemmax - _Vmmemmin)/2) ||
			   shm >= (_Vmmemmin + (_Vmmemmax - _Vmmemmin)/2) ||
			   shm < tmp ) /* mmap can be used but needs MAP_FIXED! */
#if VMCHKMEM
				_Vmchkmem = _vmchkmem; /* _vmchkmem() can check memory availability */
#else
				_Vmmemsbrk = NIL(Vmuchar_t*); /* no memory checking, must use sbrk() */
#endif /*VMCHKMEM*/
		}
#endif /*_mem_mmap_anon_*/
	}
#endif /*!_WINIX*/

	return page;
}

/* Function to suggest an address usable for mapping shared memory. */
Void_t* vmmaddress(size_t size)
{
	Vmuchar_t	*addr, *memaddr;

	GETPAGESIZE(_Vmpagesize);
	if(!_Vmmemaddr)
		return NIL(Void_t*);

	for(size = ROUND(size, _Vmpagesize);; )
	{	if((addr = (memaddr = _Vmmemaddr) - size) < _Vmmemmin)
			return NIL(Void_t*); /* exceed allowed range */

		if(asocasptr(&_Vmmemaddr, memaddr, addr) != memaddr)
			continue; /* _Vmmemaddr was changed so addr is now wrong */

		if(_vmchkmem(addr, size)) /* check availability */
			return addr;
	}
}
