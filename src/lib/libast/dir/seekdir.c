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
 * seekdir
 *
 * seek on directory stream
 * this is not optimal because there aren't portable
 * semantics for directory seeks
 */

#include "dirlib.h"

#if _dir_ok

NoN(seekdir)

#else

void
seekdir(register DIR* dirp, long loc)
{
	off_t	base;		/* file location of block */
	off_t	offset; 	/* offset within block */

	if (telldir(dirp) != loc)
	{
		lseek(dirp->dd_fd, 0L, SEEK_SET);
		dirp->dd_loc = dirp->dd_size = 0;
		while (telldir(dirp) != loc)
			if (!readdir(dirp))
				break; 	/* "can't happen" */
	}
}

#endif
