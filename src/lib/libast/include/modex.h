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
 * AT&T Research
 *
 * external mode_t representation support
 */

#ifndef _MODEX_H
#define _MODEX_H

#include <ast_fs.h>
#include <modecanon.h>

#if _BLD_ast && defined(__EXPORT__)
#define extern		__EXPORT__
#endif

extern int		modei(int);
extern int		modex(int);

#undef	extern

#if _S_IDPERM
#define modei(m)	((m)&X_IPERM)
#if _S_IDTYPE
#define modex(m)	(m)
#endif
#endif

#endif
