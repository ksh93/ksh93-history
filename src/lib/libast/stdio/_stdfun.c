/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1985-2006 AT&T Corp.                  *
*                      and is licensed under the                       *
*                  Common Public License, Version 1.0                  *
*                            by AT&T Corp.                             *
*                                                                      *
*                A copy of the License is available at                 *
*            http://www.opensource.org/licenses/cpl1.0.txt             *
*         (with md5 checksum 059e8cd6165cb4c31e351f2b69388fd9)         *
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
