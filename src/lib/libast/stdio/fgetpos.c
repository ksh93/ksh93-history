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

#include "stdhdr.h"

int
fgetpos(Sfio_t* f, fpos_t* pos)
{
	STDIO_INT(f, "fgetpos", int, (Sfio_t*, fpos_t*), (f, pos))

	return (*pos = (fpos_t)sfseek(f, (Sfoff_t)0, SEEK_CUR)) >= 0 ? 0 : -1;
}

#ifdef _ast_int8_t

int
fgetpos64(Sfio_t* f, fpos64_t* pos)
{
	STDIO_INT(f, "fgetpos64", int, (Sfio_t*, fpos64_t*), (f, pos))

	return (*pos = (fpos64_t)sfseek(f, (Sfoff_t)0, SEEK_CUR)) >= 0 ? 0 : -1;
}

#endif
