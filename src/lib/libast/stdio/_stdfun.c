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

#if !_UWIN

void _STUB_stdfun(){}

#else

#include <ast_windows.h>
#include <uwin.h>

#if _ALPHA_
#define IOB		((char*)_iob)
#else
#define IOB		((char*)__p__iob())
#endif

#define IOBMAX		(512*32)

#include "stdhdr.h"

int
_stdfun(Sfio_t* f, Funvec_t* vp)
{
	static char*	iob;
	static int	init;
	static HANDLE	bp;
	static HANDLE	np;

	if (!iob && !(iob = IOB))
		return 0;
	if (f && ((char*)f < iob || (char*)f > iob+IOBMAX))
		return 0;
	if (!vp->vec[1])
	{
		if (!init)
		{
			init = 1;
			if (!(bp = GetModuleHandle("stdio.dll")))
			{
				char	path[PATH_MAX];

				if (uwin_path("/usr/lib/stdio.dll", path, sizeof(path)) >= 0)
					bp = LoadLibraryEx(path, 0, 0);
			}
		}
		if (bp && (vp->vec[1] = (Fun_f)GetProcAddress(bp, vp->name)))
			return 1;
		if (!np && (!(np = GetModuleHandle("msvcrtd.dll")) || !(np = GetModuleHandle("msvcrt.dll"))))
			return -1;
		if (!(vp->vec[1] = (Fun_f)GetProcAddress(np, vp->name)))
			return -1;
	}
	return 1;
}

#endif
