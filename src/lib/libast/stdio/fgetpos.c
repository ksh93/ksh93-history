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
fgetpos(Sfio_t* f, fpos_t* pos)
{
	STDIO_INT(f, "fgetpos", int, (Sfio_t*, fpos_t*), (f, pos))

	return (pos->_sf_offset = sfseek(f, (Sfoff_t)0, SEEK_CUR)) >= 0 ? 0 : -1;
}

#ifdef _ast_int8_t

int
fgetpos64(Sfio_t* f, fpos64_t* pos)
{
	STDIO_INT(f, "fgetpos64", int, (Sfio_t*, fpos64_t*), (f, pos))

	return (pos->_sf_offset = sfseek(f, (Sfoff_t)0, SEEK_CUR)) >= 0 ? 0 : -1;
}

#endif
