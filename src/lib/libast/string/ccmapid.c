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
 * Glenn Fowler
 * AT&T Research
 *
 * return ccode map id given name
 */

#include <ast.h>
#include <ccode.h>
#include <iconv.h>

int
ccmapid(const char* name)
{
	return iconv_name(name, NiL, 0);
}

/*
 * return ccode map name given id
 */

char*
ccmapname(register int id)
{
	register iconv_list_t*	ic;

	for (ic = iconv_list(NiL); ic; ic = iconv_list(ic))
		if (id == ic->ccode)
			return (char*)ic->name;
	return 0;
}
