/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1985-2010 AT&T Intellectual Property          *
*                      and is licensed under the                       *
*                  Common Public License, Version 1.0                  *
*                    by AT&T Intellectual Property                     *
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
__STDPP__directive pragma pp:hide getpagesize getdtablesize
#else
#define getpagesize	______getpagesize
#define getdtablesize	______getdtablesize
#endif

/*
 * we'd like as many symbols as possible defined
 * the standards push the vendors the other way
 * but don't provide guard that lets everything through
 * so each vendor adds their own guard
 * many now include something like <standards.h> to
 * get it straight in one place -- <sys/types.h> should
 * kick that in
 */

#include "FEATURE/standards"
#include "FEATURE/lib"

#ifdef __sun
#define _timespec	timespec
#endif

#include <sys/types.h>

#undef	_SGIAPI
#define _SGIAPI		1

#if _hdr_limits
#include <limits.h>
#endif

#undef	_SGIAPI
#define _SGIAPI		0

#include "FEATURE/lib"
#include "FEATURE/common"

#if _hdr_unistd
#include <unistd.h>
#endif

#include "FEATURE/param"

#if defined(__STDPP__directive) && defined(__STDPP__hide)
__STDPP__directive pragma pp:nohide getpagesize getdtablesize
#else
#undef	getpagesize
#undef	getdtablesize   
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
