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
/*
 * David Korn
 * AT&T Research
 *
 * Interface definitions for a stack-like storage library
 *
 */

#ifndef _STAK_H
#define _STAK_H

#include	<stk.h>

#define Stak_t		Sfio_t
#define	staksp		stkstd
#define STAK_SMALL	STK_SMALL

#define	stakptr(n)		stkptr(stkstd,n)
#define	staktell()		stktell(stkstd)
#define stakputc(c)		sfputc(stkstd,(c))
#define stakwrite(b,n)		sfwrite(stkstd,(b),(n))
#define stakputs(s)		(sfputr(stkstd,(s),0),--stkstd->next)
#define stakseek(n)		stkseek(stkstd,n)
#define stakcreate(n)		stkopen(n)
#define stakinstall(s,f)	stkinstall(s,f)
#define stakdelete(s)		stkclose(s)
#define staklink(s)		stklink(s)
#define stakalloc(n)		stkalloc(stkstd,n)
#define stakcopy(s)		stkcopy(stkstd,s)
#define stakset(c,n)		stkset(stkstd,c,n)
#define stakfreeze(n)		stkfreeze(stkstd,n)

#endif
