/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1985-2003 AT&T Corp.                *
*        and it may only be used by you under license from         *
*                       AT&T Corp. ("AT&T")                        *
*         A copy of the Source Code Agreement is available         *
*                at the AT&T Internet web site URL                 *
*                                                                  *
*       http://www.research.att.com/sw/license/ast-open.html       *
*                                                                  *
*    If you have copied or used this software without agreeing     *
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
*                                                                  *
*******************************************************************/
#pragma prototyped

/*
 * determine the preferred raw memory allocation method
 */

#include "FEATURE/common"
#include "FEATURE/mmap"
#include "FEATURE/vmalloc"

extern int	printf(const char*, ...);

/* different methods to get raw memory for MEM_METHOD */

#define RAW_MALLOC	0x01	/* native malloc		*/
#define RAW_MMAP_ANON	0x02	/* mmap MAP_ANON is ok		*/
#define RAW_MMAP_DEV	0x04	/* mmap on /dev/zero is ok	*/
#define RAW_SBRK	0x08	/* sbrk				*/
#define RAW_WIN32	0x10	/* WIN32 VirtualAlloc		*/

#undef	MEM_METHOD		/* inclusive | of MEM_*		*/

/* first some availability tests for MEM_METHOD */

#if _map_malloc || _std_malloc
#define MEM_MALLOC	RAW_MALLOC
#else
#define MEM_MALLOC	0
#endif

#if _mmap_anon
#define MEM_MMAP_ANON	RAW_MMAP_ANON
#else
#define MEM_MMAP_ANON	0
#endif

#if _mmap_devzero
#define MEM_MMAP_DEV	RAW_MMAP_DEV
#else
#define MEM_MMAP_DEV	0
#endif

#if _lib_sbrk
#define MEM_SBRK	RAW_SBRK
#else
#define MEM_SBRK	0
#endif

#if _WIN32
#define MEM_WIN32	RAW_WIN32
#else
#define MEM_WIN32	0
#endif

/* finally the selection for MEM_METHOD */

#if !MEM_METHOD && MEM_MALLOC && ( _BLD_INSTRUMENT || _WINIX && !_UWIN )
#define MEM_METHOD	MEM_MALLOC
#endif

#if !MEM_METHOD && MEM_WIN32
#define MEM_METHOD	MEM_WIN32
#endif

#if !MEM_METHOD && MEM_MMAP_ANON
#define MEM_METHOD	MEM_MMAP_ANON
#endif

#if !MEM_METHOD && MEM_MMAP_DEV
#define MEM_METHOD	MEM_MMAP_DEV
#endif

#if !MEM_METHOD && MEM_SBRK
#define MEM_METHOD	MEM_SBRK
#endif

#if !MEM_METHOD && MEM_MALLOC
#define MEM_METHOD	MEM_MALLOC
#endif

main()
{
	char*	s;
	int	x;

	printf("\n");
	printf("/* fundamental memory allocation methods */\n");
	printf("\n");
	x = 1;
#if MEM_MALLOC
	printf("#define _mem_malloc	0x%x	/* native malloc	*/\n", x);
	x <<= 1;
#endif
#if MEM_MMAP_ANON
	printf("#define _mem_mmap_anon	0x%x	/* mmap MAP_ANON	*/\n", x);
	x <<= 1;
#endif
#if MEM_MMAP_DEV
	printf("#define _mem_mmap_dev	0x%x	/* mmap /dev/zero	*/\n", x);
	x <<= 1;
#endif
#if MEM_SBRK
	printf("#define _mem_sbrk	0x%x	/* sbrk			*/\n", x);
	x <<= 1;
#endif
#if MEM_WIN32
	printf("#define _mem_win32	0x%x	/* win32 VirtualAlloc	*/\n", x);
	x <<= 1;
#endif
	printf("\n");
	printf("/* preferred memory allocation method */\n");
	printf("\n");
#if !MEM_METHOD
	s = "0";
#endif
#if MEM_METHOD & MEM_MALLOC
	s = "_mem_malloc";
#endif
#if MEM_METHOD & MEM_MMAP_ANON
	s = "_mem_mmap_anon";
#endif
#if MEM_METHOD & MEM_MMAP_DEV
	s = "_mem_mmap_dev";
#endif
#if MEM_METHOD & MEM_SBRK
	s = "_mem_sbrk";
#endif
#if MEM_METHOD & MEM_WIN32
	s = "_mem_win32";
#endif
	printf("#define _mem_method	%s\n", s);
	printf("\n");
	return 0;
}
