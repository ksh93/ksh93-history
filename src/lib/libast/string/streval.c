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
/*
 * obsolete streval() interface to strexpr()
 */

#include <ast.h>

typedef long (*Old_convert_t)(const char*, char**);

typedef long (*Convert_t)(const char*, char**, void*);

typedef struct
{
	Old_convert_t	convert;
} Handle_t;

static long
userconv(const char* s, char** end, void* handle)
{
	return((*((Handle_t*)handle)->convert)(s, end));
}

long
streval(const char* s, char** end, Old_convert_t convert)
{
	Handle_t	handle;

	return((handle.convert = convert) ? strexpr(s, end, userconv, &handle) : strexpr(s, end, (Convert_t)0, NiL));
}
