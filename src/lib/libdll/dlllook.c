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
 * dlsym() with `_' weak fallback
 */

void*
dlllook(void* dll, const char* name)
{
	register void*	addr;
	char		buf[256];

	if (!(addr = dlsym(dll, name)) && strlen(name) < (sizeof(buf) - 2))
	{
		buf[0] = '_';
		strcpy(buf + 1, name);
		addr = dlsym(dll, buf);
	}
	return addr;
}
