/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1985-2004 AT&T Corp.                *
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
*                          AT&T Research                           *
*                         Florham Park NJ                          *
*                                                                  *
*               Glenn Fowler <gsf@research.att.com>                *
*                David Korn <dgk@research.att.com>                 *
*                 Phong Vo <kpv@research.att.com>                  *
*                                                                  *
*******************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Research
 *
 * record format interface
 */

#ifndef _RECFMT_H
#define _RECFMT_H	1

#include <ast.h>

#define REC_fixed	0
#define REC_variable	1
#define REC_delimited	2

#define RECMAKE(t,s)	(((t)<<24)|(s))
#define RECTYPE(n)	(((n)>>24)&((1<<7)-1))
#define RECSIZE(n)	((n)&((1<<24)-1))

#if _BLD_ast && defined(__EXPORT__)
#define extern		__EXPORT__
#endif

extern size_t		recfmt(const void*, size_t, off_t);

#undef	extern

#endif
