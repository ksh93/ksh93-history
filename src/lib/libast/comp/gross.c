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
/*
 * porting hacks here
 */

#include <ast.h>
#include <ls.h>

#include "FEATURE/hack"

void _STUB_gross(){}

#if _lcl_xstat

extern int fstat(int fd, struct stat* st)
{
#if _lib___fxstat
	return __fxstat(_STAT_VER, fd, st);
#else
	return _fxstat(_STAT_VER, fd, st);
#endif
}

extern int lstat(const char* path, struct stat* st)
{
#if _lib___lxstat
	return __lxstat(_STAT_VER, path, st);
#else
	return _lxstat(_STAT_VER, path, st);
#endif
}

extern int stat(const char* path, struct stat* st)
{
#if _lib___xstat
	return __xstat(_STAT_VER, path, st);
#else
	return _xstat(_STAT_VER, path, st);
#endif
}

#endif

#if _lcl_xstat64

extern int fstat64(int fd, struct stat64* st)
{
#if _lib___fxstat64
	return __fxstat64(_STAT_VER, fd, st);
#else
	return _fxstat64(_STAT_VER, fd, st);
#endif
}

extern int lstat64(const char* path, struct stat64* st)
{
#if _lib___lxstat64
	return __lxstat64(_STAT_VER, path, st);
#else
	return _lxstat64(_STAT_VER, path, st);
#endif
}

extern int stat64(const char* path, struct stat64* st)
{
#if _lib___xstat64
	return __xstat64(_STAT_VER, path, st);
#else
	return _xstat64(_STAT_VER, path, st);
#endif
}

#endif

#if __sgi && _hdr_locale_attr

#include "gross_sgi.h"

#endif
