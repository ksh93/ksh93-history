/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1985-2004 AT&T Corp.                  *
*                      and is licensed under the                       *
*                  Common Public License, Version 1.0                  *
*                            by AT&T Corp.                             *
*                                                                      *
*                A copy of the License is available at                 *
*            http://www.opensource.org/licenses/cpl1.0.txt             *
*         (with md5 checksum 059e8cd6165cb4c31e351f2b69388fd9)         *
*                                                                      *
*              Information and Software Systems Research               *
*                            AT&T Research                             *
*                           Florham Park NJ                            *
*                                                                      *
*                 Glenn Fowler <gsf@research.att.com>                  *
*                  David Korn <dgk@research.att.com>                   *
*                   Phong Vo <kpv@research.att.com>                    *
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Bell Laboratories
 *
 * generate unistd.h definitions for posix conf function
 */

#if defined(__STDPP__directive) && defined(__STDPP__hide)
__STDPP__directive pragma pp:hide getpagesize getdtablesize printf
#else
#define getpagesize	______getpagesize
#define getdtablesize	______getdtablesize
#define printf		______printf
#endif

#include "FEATURE/standards"

#include <sys/types.h>

#include "FEATURE/lib"
#include "FEATURE/limits"
#include "FEATURE/unistd.lcl"

#if defined(__STDPP__directive) && defined(__STDPP__hide)
__STDPP__directive pragma pp:nohide getpagesize getdtablesize printf
#else
#undef  getpagesize
#undef  getdtablesize
#endif

#if defined(__STDPP__hide) || defined(printf)
#undef	printf
extern int		printf(const char*, ...);
#endif

#include "conflib.h"

int main()
{
#include "confuni.h"
#if _dll_data_intercept
	printf("\n#if _dll_data_intercept && ( _DLL_BLD || _BLD_DLL )\n");
	printf("#undef	environ\n");
	printf("#define environ (*_ast_dll._dll_environ)\n");
	printf("struct _astdll { char*** _dll_environ; };\n");
	printf("extern struct _astdll _ast_dll;\n");
	printf("extern struct _astdll* _ast_getdll(void);\n");
	printf("#endif\n");
#endif
	return 0;
}
