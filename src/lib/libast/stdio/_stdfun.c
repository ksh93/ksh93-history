/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1985-2000 AT&T Corp.                *
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
*                                                                  *
*******************************************************************/
#pragma prototyped

#include <ast.h>

#if !_UWIN

void _STUB_stdfun(){}

#else

#include <windows.h>

typedef struct
{
	char*		next;
	int		endw_cnt;
	int		endr_base;
	int		endb_flag;
	int		push_file;
} Sfio_file_t;

#include "stdhdr.h"

int
_stdfun(Sfio_t* f, Funvec_t* vp)
{
#if _MAN_THIS_ALMOST_WORKS
	static HANDLE	hp;
	Sfio_file_t*	mp = (Sfio_file_t*)f;

	if (mp->endr_base && mp->endr_base == mp->endb_flag)
		return 0;
	if (mp->endw_cnt && mp->endw_cnt == mp->endb_flag)
		return 0;
	if (mp->push_file >= 0 && mp->push_file >= 100000)
		return 0;
	if (mp->endb_flag == 0)
		return 0;
	if (!vp->vec[1])
	{
		if (!hp && !(hp = GetModuleHandle("msvcrt.dll")))
			return -1;
		if (!(vp->vec[1] = (Fun_f)GetProcAddress(hp, vp->name)))
			return -1;
	}
	return 1;
#else
	return 0;
#endif
}

#endif
