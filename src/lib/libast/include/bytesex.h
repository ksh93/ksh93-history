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
/*
 * linux/gnu compatibility
 */

#ifndef _BYTESEX_H
#define _BYTESEX_H

#include <ast_common.h>

#undef __BYTE_ORDER

#if ( _ast_intswap & 3 ) == 3
#define __BYTE_ORDER	__LITTLE_ENDIAN
#else
#if ( _ast_intswap & 3 ) == 1
#define __BYTE_ORDER	__PDP_ENDIAN
#else
#define __BYTE_ORDER	__BIG_ENDIAN
#endif
#endif

#endif
