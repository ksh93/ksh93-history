/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1996-2003 AT&T Corp.                *
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
*                                                                  *
*******************************************************************/
#pragma prototyped

/*
 * Glenn Fowler
 * AT&T Labs Research
 *
 * checksum library interface
 */

#ifndef _SUM_H
#define _SUM_H

#include <ast.h>

#define SUM_SIZE	(1<<0)		/* print size too		*/
#define SUM_SCALE	(1<<1)		/* traditional size scale	*/
#define SUM_TOTAL	(1<<2)		/* print totals since sumopen	*/

#define _SUM_PUBLIC_	const char*	name;

typedef struct
{
	_SUM_PUBLIC_
#ifdef	_SUM_PRIVATE_
	_SUM_PRIVATE_
#endif
} Sum_t;

extern Sum_t*	sumopen(const char*);
extern int	sumblock(Sum_t*, const void*, size_t);
extern int	sumprint(Sum_t*, Sfio_t*, int);
extern int	sumusage(Sfio_t*);
extern int	suminit(Sum_t*);
extern int	sumdone(Sum_t*);
extern int	sumclose(Sum_t*);

#endif
