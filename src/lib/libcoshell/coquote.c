/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1990-2004 AT&T Corp.                  *
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
 * AT&T Research
 *
 * single quote s into sp
 * if type!=0 then /<getenv(<CO_ENV_TYPE>)/ translated to /$<CO_ENV_TYPE>/
 */

#include "colib.h"

void
coquote(register Sfio_t* sp, register const char* s, int type)
{
	register int	c;

	if (type && (!state.type || !*state.type))
		type = 0;
	while (c = *s++)
	{
		sfputc(sp, c);
		if (c == '\'')
		{
			sfputc(sp, '\\');
			sfputc(sp, '\'');
			sfputc(sp, '\'');
		}
		else if (type && c == '/' && *s == *state.type)
		{
			register const char*	x = s;
			register char*		t = state.type;

			while (*t && *t++ == *x) x++;
			if (!*t && *x == '/')
			{
				s = x;
				sfprintf(sp, "'$%s'", CO_ENV_TYPE);
			}
		}
	}
}
