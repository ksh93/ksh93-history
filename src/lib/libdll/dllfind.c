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
 * if path!=0 then library path up to size chars copied to path with trailing 0
 */

extern void*
dllfind(const char* lib, const char* ver, int flags, char* path, size_t size)
{
	void*		dll;
	int		hit;
	Dllscan_t*	dls;
	Dllent_t*	dle;

	if (lib && (ver || !strchr(lib, '/')))
	{
		hit = 0;
		if (dls = dllsopen(NiL, lib, ver))
		{
			while (dle = dllsread(dls))
			{
				hit = 1;
				if (dll = dlopen(dle->path, flags))
				{
					if (path && size)
#if __OBSOLETE__ < 20041118 /* path,size added 2002-11-18 */
					if (size == PATH_MAX)
#endif
						strncopy(path, dle->path, size);
					break;
				}
			}
			dllsclose(dls);
		}
		if (hit)
			return dll;
	}
	if (!(dll = dlopen(lib, flags)) && !strchr(lib, '/') && strchr(lib, '.'))
		dll = dlopen(sfprints("./%s", lib), flags);
	if (dll && path && size)
#if __OBSOLETE__ < 20041118 /* path,size added 2002-11-18 */
	if (size == PATH_MAX)
#endif
		strncopy(path, lib, size);
	return dll;
}
