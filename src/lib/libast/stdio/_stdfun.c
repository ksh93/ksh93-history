/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1985-2001 AT&T Corp.                *
*        and it may only be used by you under license from         *
*                       AT&T Corp. ("AT&T")                        *
*         A copy of the Source Code Agreement is available         *
*                at the AT&T Internet web site URL                 *
*                                                                  *
*       http://www.research.att.com/sw/license/ast-open.html       *
*                                                                  *
*        If you have copied this software without agreeing         *
*        to the terms of the license you are infringing on         *
*           the license and copyright and are violating            *
*               AT&T's intellectual property rights.               *
*                                                                  *
*                 This software was created by the                 *
*                 Network Services Research Center                 *
*                        AT&T Labs Research                        *
*                         Florham Park NJ                          *
*                                                                  *
*               Glenn Fowler <gsf@research.att.com>                *
*                David Korn <dgk@research.att.com>                 *
*                 Phong Vo <kpv@research.att.com>                  *
*******************************************************************/
#pragma prototyped

#include <ast.h>

#if !_UWIN

void _STUB_stdfun(){}

#else

#include <windows.h>
#include <uwin.h>

#if 0 /* this is in <wchar.h> now */
__declspec(dllimport) extern char* __cdecl __p__iob(void);
#endif

#define IOBMAX	(512*32)

#include "stdhdr.h"

int
_stdfun(Sfio_t* f, Funvec_t* vp)
{
	static char*	iob;
	static int	init;
	static HANDLE	bp;
	static HANDLE	np;

	if (!iob && !(iob = (char*)__p__iob()))
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
