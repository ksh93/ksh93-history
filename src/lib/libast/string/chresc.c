/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1985-2000 AT&T Corp.                *
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
*                David Korn <dgk@research.att.com>                 *
*                 Phong Vo <kpv@research.att.com>                  *
*                                                                  *
*******************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Research
 *
 * return the next character in the string s
 * \ character constants are converted
 * p is updated to point to the next character in s
 */

#include <ast.h>
#include <ccode.h>

int
chresc(register const char* s, char** p)
{
	register const char*	q;
	register int		c;

	switch (c = *s++)
	{
	case 0:
		s--;
		break;
	case '\\':
		switch (c = *s++)
		{
		case '0': case '1': case '2': case '3':
		case '4': case '5': case '6': case '7':
			c -= '0';
			q = s + 2;
			while (s < q) switch (*s)
			{
			case '0': case '1': case '2': case '3':
			case '4': case '5': case '6': case '7':
				c = (c << 3) + *s++ - '0';
				break;
			default:
				q = s;
				break;
			}
			break;
		case 'a':
			c = CC_bel;
			break;
		case 'b':
			c = '\b';
			break;
		case 'f':
			c = '\f';
			break;
		case 'n':
			c = '\n';
			break;
		case 'r':
			c = '\r';
			break;
		case 's':
			c = ' ';
			break;
		case 't':
			c = '\t';
			break;
		case 'v':
			c = CC_vt;
			break;
		case 'x':
			c = 0;
			q = s;
			while (q) switch (*s)
			{
			case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
				c = (c << 4) + *s++ - 'a' + 10;
				break;
			case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
				c = (c << 4) + *s++ - 'A' + 10;
				break;
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				c = (c << 4) + *s++ - '0';
				break;
			default:
				q = 0;
				break;
			}
			break;
		case 'E':
			c = CC_esc;
			break;
		case 0:
			s--;
			break;
		}
		break;
	}
	if (p) *p = (char*)s;
	return c;
}
