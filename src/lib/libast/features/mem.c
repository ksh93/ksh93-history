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

/* different methods to get raw memory */

#define RAW_MALLOC	0x01	/* native malloc		*/
#define RAW_MMAP_ANON	0x02	/* mmap MAP_ANON is ok		*/
#define RAW_MMAP_DEV	0x04	/* mmap on /dev/zero is ok	*/
#define RAW_SBRK	0x08	/* sbrk				*/
#define RAW_WIN32	0x10	/* WIN32 VirtualAlloc		*/

/* first some availability tests */

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

#if _lib_sbrk && _lib_brk
#define MEM_SBRK	RAW_SBRK
#else
#define MEM_SBRK	0
#endif

#if _WIN32
#define MEM_WIN32	RAW_WIN32
#else
#define MEM_WIN32	0
#endif

main()
{
	int	c;
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
	printf("#define _mem_method	");
	c = '(';
#if MEM_MALLOC && ( _BLD_INSTRUMENT || _WINIX && !_UWIN )
	printf("_mem_malloc");
	c = ')';
#else
#	if MEM_WIN32
		printf("_mem_win32");
#	else
#		if MEM_SBRK
			printf("%c_mem_sbrk", c);
			c = '|';
#		endif
#		if MEM_MMAP_ANON
			printf("%c_mem_mmap_anon", c);
			c = '|';
#		else
#			if MEM_MMAP_DEV
				printf("%c_mem_mmap_dev", c);
				c = '|';
#			endif
#		endif
#		if MEM_MALLOC
			if (c != '|')
			{
				printf("_mem_malloc");
				c = ')';
			}
#		endif
#	endif
#endif
	if (c == '(')
		printf("0");
	else if (c == '|')
		printf(")");
	printf("\n\n");
	return 0;
}
