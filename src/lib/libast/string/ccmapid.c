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
 * AT&T Research
 *
 * return ccode map id given name
 */

#include <ast.h>
#include <ctype.h>
#include <ccode.h>

static const struct
{
	int		id;
	const char*	name;
	const char*	pattern;
}
maps[] =
{
CC_ASCII,	"ascii",	"a|ascii|iso646|iso8859*",
CC_EBCDIC1,	"ebcdic",	"e|ebcdic?(1)",
CC_EBCDIC2,	"ebcdic2",	"i|ebcdic2|ibm",
CC_EBCDIC3,	"ebcdic3",	"o|ebcdic3|cp1047|ibm1047|mvs|openedition",
CC_NATIVE,	"native",	"n|local",
};

int
ccmapid(const char* name)
{
	register int	c;
	register char*	s;
	register char*	t;
	char		buf[64];

	s = (char*)name;
	t = buf;
	while (t < &buf[sizeof(buf)-1] && (c = *s++))
		if (isalnum(c))
		{
			if (isupper(c))
				c = tolower(c);
			*t++ = c;
		}
	*t = 0;
	t = buf;
	for (c = 0; c < elementsof(maps); c++)
		if (strmatch(t, maps[c].pattern))
			return maps[c].id;
	return -1;
}

/*
 * return ccode map name given id
 */

char*
ccmapname(register int id)
{
	register int	c;

	for (c = 0; c < elementsof(maps); c++)
		if (id == maps[c].id)
			return (char*)maps[c].name;
	return 0;
}
