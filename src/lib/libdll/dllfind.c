/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1997-2002 AT&T Corp.                *
*        and it may only be used by you under license from         *
*                       AT&T Corp. ("AT&T")                        *
*         A copy of the Source Code Agreement is available         *
*                at the AT&T Internet web site URL                 *
*                                                                  *
*       http://www.research.att.com/sw/license/ast-open.html       *
*                                                                  *
*    If you have copied or used this software without agreeing     *
*        to the terms of the license you are infringing on         *
*           the license and copyright and are violating            *
*               AT&T's intellectual property rights.               *
*                                                                  *
*            Information and Software Systems Research             *
*                        AT&T Labs Research                        *
*                         Florham Park NJ                          *
*                                                                  *
*               Glenn Fowler <gsf@research.att.com>                *
*                                                                  *
*******************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Labs Research
 */

#include <ast.h>
#include <dlldefs.h>

/*
 * find and load library lib with optional version ver and dlopen() flags
 * at least one dlopen() is called to initialize dlerror()
 */

extern void*
dllfind(const char* lib, const char* ver, int flags)
{
	void*		dll;
	int		hit;
	Dllscan_t*	dls;
	Dllent_t*	dle;

	if (!lib || !ver && strchr(lib, '/'))
		dll = dlopen(lib, flags);
	else
	{
		hit = 0;
		if (dls = dllsopen(NiL, lib, ver))
		{
			while (dle = dllsread(dls))
			{
				hit = 1;
				if (dll = dlopen(dle->path, flags))
					break;
			}
			dllsclose(dls);
		}
		if (!hit)
			dll = dlopen(lib, flags);
	}
	return dll;
}
