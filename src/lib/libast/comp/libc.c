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
#pragma prototyped

#include <ast.h>

#if _std_malloc || _BLD_INSTRUMENT || cray

int	_STUB_libc;

#else

extern void*	memalign(size_t, size_t);
extern void*	valloc(size_t);

void*	__libc_calloc(size_t n, size_t m) { return calloc(n, m); }
void	__libc_cfree(void* p) { cfree(p); }
void	__libc_free(void* p) { free(p); }
void*	__libc_malloc(size_t n) { return malloc(n); }
void*	__libc_memalign(size_t a, size_t n) { return memalign(a, n); }
void*	__libc_realloc(void* p, size_t n) { return realloc(p, n); }
void*	__libc_valloc(size_t n) { return valloc(n); }

#endif
