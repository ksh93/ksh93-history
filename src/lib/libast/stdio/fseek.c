/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1985-2004 AT&T Corp.                  *
*                      and is licensed under the                       *
*          Common Public License, Version 1.0 (the "License")          *
*                        by AT&T Corp. ("AT&T")                        *
*      Any use, downloading, reproduction or distribution of this      *
*      software constitutes acceptance of the License.  A copy of      *
*                     the License is available at                      *
*                                                                      *
*         http://www.research.att.com/sw/license/cpl-1.0.html          *
*         (with md5 checksum 8a5e0081c856944e76c69a1cf29c2e8b)         *
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

#include "stdhdr.h"

int
fseek(Sfio_t* f, long off, int op)
{
	STDIO_INT(f, "fseek", int, (Sfio_t*, long, int), (f, off, op))

	return sfseek(f, (Sfoff_t)off, op|SF_SHARE) >= 0 ? 0 : -1;
}

#ifdef _ast_int8_t

int
fseek64(Sfio_t* f, _ast_int8_t off, int op)
{
	STDIO_INT(f, "fseek64", int, (Sfio_t*, _ast_int8_t, int), (f, off, op))

	return sfseek(f, (Sfoff_t)off, op|SF_SHARE) >= 0 ? 0 : -1;
}

#endif
