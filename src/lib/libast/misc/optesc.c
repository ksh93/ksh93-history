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
 * escape optget() special chars in s and write to sp
 */

#include <optlib.h>
#include <ctype.h>

int
optesc(Sfio_t* sp, register const char* s)
{
	register const char*	m;
	register int		c;

	if (*s == '[' && *(s + 1) == '+' && *(s + 2) == '?')
	{
		c = strlen(s);
		if (s[c - 1] == ']')
		{
			sfprintf(sp, "%-.*s", c - 4, s + 3);
			return 0;
		}
	}
	while (c = *s++)
	{
		if (isalnum(c))
		{
			for (m = s - 1; isalnum(*s); s++);
			if (isalpha(c) && *s == '(' && isdigit(*(s + 1)) && *(s + 2) == ')')
			{
				sfputc(sp, '\b');
				sfwrite(sp, m, s - m);
				sfputc(sp, '\b');
				sfwrite(sp, s, 3);
				s += 3;
			}
			else
				sfwrite(sp, m, s - m);
		}
		else if (c == '-' && *s == '-' || c == '<')
		{
			m = s - 1;
			if (c == '-')
				s++;
			else if (*s == '/')
				s++;
			while (isalnum(*s))
				s++;
			if (c == '<' && *s == '>' || isspace(*s) || *s == 0 || *s == '=' || *s == ':' || *s == ';' || *s == '.' || *s == ',')
			{
				sfputc(sp, '\b');
				sfwrite(sp, m, s - m);
				sfputc(sp, '\b');
			}
			else
				sfwrite(sp, m, s - m);
		}
		else
		{
			if (c == ']')
				sfputc(sp, c);
			sfputc(sp, c);
		}
	}
	return 0;
}
