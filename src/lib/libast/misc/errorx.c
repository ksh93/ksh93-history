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

#include "lclib.h"

/*
 * low level for ERROR_translate()
 * this fills in NiL arg defaults and calls error_info.translate
 */

char*
errorx(const char* loc, const char* cmd, const char* cat, const char* msg)
{
	char*	s;

	if (ERROR_translating())
	{
		if (!loc)
			loc = (const char*)locales[AST_LC_MESSAGES]->code;
		if (!cat)
		{
			cat = (const char*)error_info.catalog;
			if (!cmd)
				cmd = (const char*)error_info.id;
		}
		if (s = (*error_info.translate)(loc, cmd, cat, msg))
			return s;
	}
	return (char*)msg;
}
