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

#include <ast.h>

#if _lib_readlink

NoN(readlink)

#else

#include "fakelink.h"

#include <error.h>

#ifndef ENOSYS
#define ENOSYS	EINVAL
#endif

int
readlink(const char* path, char* buf, int siz)
{
	int	fd;
	int	n;

	if (siz > sizeof(FAKELINK_MAGIC))
	{
		if ((fd = open(path, O_RDONLY)) < 0)
			return -1;
		if (read(fd, buf, sizeof(FAKELINK_MAGIC)) == sizeof(FAKELINK_MAGIC) && !strcmp(buf, FAKELINK_MAGIC) && (n = read(fd, buf, siz)) > 0 && !buf[n - 1])
		{
			close(fd);
			return n;
		}
		close(fd);
	}
	errno = ENOSYS;
	return -1;
}

#endif
