/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1990-2001 AT&T Corp.                *
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
*******************************************************************/
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
