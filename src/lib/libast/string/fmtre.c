/***************************************************************
*                                                              *
*           This software is part of the ast package           *
*              Copyright (c) 1985-2000 AT&T Corp.              *
*      and it may only be used by you under license from       *
*                     AT&T Corp. ("AT&T")                      *
*       A copy of the Source Code Agreement is available       *
*              at the AT&T Internet web site URL               *
*                                                              *
*     http://www.research.att.com/sw/license/ast-open.html     *
*                                                              *
*     If you received this software without first entering     *
*       into a license with AT&T, you have an infringing       *
*           copy and cannot use it without violating           *
*             AT&T's intellectual property rights.             *
*                                                              *
*               This software was created by the               *
*               Network Services Research Center               *
*                      AT&T Labs Research                      *
*                       Florham Park NJ                        *
*                                                              *
*             Glenn Fowler <gsf@research.att.com>              *
*              David Korn <dgk@research.att.com>               *
*               Phong Vo <kpv@research.att.com>                *
*                                                              *
***************************************************************/
#pragma prototyped

/*
 * Glenn Fowler
 * AT&T Bell Laboratories
 *
 * return RE expression given strmatch() pattern
 * 0 returned for invalid RE
 */

#include <ast.h>

#define CHUNK		32

char*
fmtre(const char* as)
{
	register char*	s = (char*)as;
	register int	c;
	register char*	t;
	register char*	p;
	int		n;
	char		stack[32];

	static char*	buf;
	static int	bufsiz;

	c = 2 * strlen(s) + 1;
	if (bufsiz < c)
	{
		bufsiz = ((c + CHUNK - 1) / CHUNK) * CHUNK;
		if (!(buf = newof(buf, char, bufsiz, 0))) return(0);
	}
	t = buf;
	p = stack;
	for (;;)
	{
		switch (c = *s++)
		{
		case 0:
			break;
		case '\\':
			if (!(c = *s++) || c == '{' || c == '}') return(0);
			*t++ = '\\';
			if ((*t++ = c) == '(' && *s == '|')
			{
				*t++ = *s++;
				goto alternate;
			}
			continue;
		case '[':
			*t++ = c;
			n = 0;
			if ((c = *s++) == '!')
			{
				*t++ = '^';
				c = *s++;
			}
			else if (c == '^')
			{
				if ((c = *s++) == ']')
				{
					*(t - 1) = '\\';
					*t++ = '^';
					continue;
				}
				n = '^';
			}
			for (;;)
			{
				if (!(*t++ = c)) return(0);
				if ((c = *s++) == ']')
				{
					if (n) *t++ = n;
					*t++ = c;
					break;
				}
			}
			continue;
		case '*':
		case '?':
		case '+':
		case '@':
		case '!':
			if (*s == '(')
			{
				if (p >= &stack[elementsof(stack)]) return(0);
				*p++ = c == '@' ? 0 : c;
				c = *s++;
			}
			switch (c)
			{
			case '*':
				*t++ = '.';
				break;
			case '?':
				c = '.';
				break;
			case '+':
			case '!':
				*t++ = '\\';
				break;
			}
			*t++ = c;
			continue;
		case '(':
			if (p >= &stack[elementsof(stack)]) return(0);
			*p++ = 0;
			*t++ = c;
			continue;
		case ')':
			if (p == stack) return(0);
			*t++ = c;
			if (c = *--p) *t++ = c;
			continue;
		case '|':
			if (t == buf || *(t - 1) == '(') return(0);
		alternate:
			if (!*s || *s == ')') return(0);
			/*FALLTHROUGH*/
		default:
			*t++ = c;
			continue;
		}
		break;
	}
	if (p != stack) return(0);
	*t = 0;
	return(buf);
}
