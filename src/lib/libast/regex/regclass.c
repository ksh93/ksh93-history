/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1985-2001 AT&T Corp.                *
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
*******************************************************************/
#pragma prototyped
/*
 * RE character class support
 */

#include "reglib.h"

#include <ccode.h>

/*
 * this stuff gets around posix failure to define isblank,
 * and the fact that ctype functions are macros
 */

static int Isalnum(int c) { return isalnum(c); }
static int Isalpha(int c) { return isalpha(c); }
static int Isblank(int c) { return c == ' ' || c == '\t'; }
static int Iscntrl(int c) { return iscntrl(c); }
static int Isdigit(int c) { return isdigit(c); }
static int Isgraph(int c) { return isgraph(c); }
static int Islower(int c) { return islower(c); }
static int Isprint(int c) { return isprint(c); }
static int Ispunct(int c) { return ispunct(c); }
static int Isspace(int c) { return isspace(c); }
static int Isupper(int c) { return isupper(c); }
static int Isxdigit(int c){ return isxdigit(c);}

static struct
{
	const char*	name;
	regclass_t	ctype;
} ctype[] =
{
	 { "alnum", Isalnum },
	 { "alpha", Isalpha },
	 { "blank", Isblank },
	 { "cntrl", Iscntrl },
	 { "digit", Isdigit },
	 { "graph", Isgraph },
	 { "lower", Islower },
	 { "print", Isprint },
	 { "punct", Ispunct },
	 { "space", Isspace },
	 { "upper", Isupper },
	 { "xdigit",Isxdigit}
};

/*
 * return pointer to ctype function for :class:] in s
 * s points to the first char after the initial [
 * if e!=0 it points to next char in s
 * 0 returned on error
 */

regclass_t
regclass(const char* s, char** e)
{
	register int		c;
	register int		i;
	register const char*	t;
	register const char*	u;

	if (c = *s++)
		for (i = 0; i < elementsof(ctype); i++)
		{
			t = s;
			u = ctype[i].name;
			do if (!*u)
			{
				if (*t++ == c && *t++ == ']')
				{
					if (e)
						*e = (char*)t;
					return ctype[i].ctype;
				}
				break;
			} while (*t++ == *u++);
		}
	return 0;
}

/*
 * return the collating symbol delimited by [c c], where c is
 * either '=' or '.'
 *
 * s points to the first char after the initial [
 * if e!=0 it points to next char in s
 * -1 returned on error
 */

int
regcollate(register const char* s, char** e)
{
	register int		c;
	register int		i;

	if (((c = *s++) == '.' || c == '=') && ((i = *s++) && *s++ == c && *s++ == ']'))
	{
		if (e)
			*e = (char*)s;
		return i;
	}
	return -1;
}
