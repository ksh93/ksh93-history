/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1997-2004 AT&T Corp.                  *
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
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Labs Research
 */

#include <ast.h>
#include <dlldefs.h>

/*
 * find and load lib plugin/module library name with optional version ver and dlopen() flags
 * at least one dlopen() is called to initialize dlerror()
 * if path!=0 then library path up to size chars copied to path with trailing 0
 * if name contains a directory prefix then library search is limited to the dir and siblings
 */

extern void*
dllplug(const char* lib, const char* name, const char* ver, int flags, char* path, size_t size)
{
	void*		dll;
	int		hit;
	Dllscan_t*	dls;
	Dllent_t*	dle;

	hit = 0;
	for (;;)
	{
		if (dls = dllsopen(lib, name, ver))
		{
			while (dle = dllsread(dls))
			{
				hit = 1;
				if (dll = dlopen(dle->path, flags|RTLD_GLOBAL|RTLD_PARENT))
				{
					if (path && size)
						strncopy(path, dle->path, size);
					break;
				}
			}
			dllsclose(dls);
		}
		if (hit)
			return dll;
		if (!lib)
			break;
		lib = 0;
	}
	if (!(dll = dlopen(name, flags)) && !strchr(name, '/') && strchr(name, '.'))
		dll = dlopen(sfprints("./%s", name), flags);
	if (dll && path && size)
		strncopy(path, name, size);
	return dll;
}
