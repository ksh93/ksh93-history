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
 * posix regex error message handler
 */

const char id[] = "\n@(#)regex (AT&T Research) 1999-04-23\0\n";

#include "reglib.h"

static const char*	reg_error[] =
{
	/* REG_ENOSYS	*/	"not supported",
	/* REG_SUCCESS	*/	"success",
	/* REG_NOMATCH	*/	"no match",
	/* REG_BADPAT	*/	"invalid regular expression",
	/* REG_ECOLLATE	*/	"invalid collation element",
	/* REG_ECTYPE	*/	"invalid character class",
	/* REG_EESCAPE	*/	"trailing \\ in pattern",
	/* REG_ESUBREG	*/	"invalid \\digit backreference",
	/* REG_EBRACK	*/	"[...] imbalance",
	/* REG_EPAREN	*/	"\\(...\\) or (...) imbalance",
	/* REG_EBRACE	*/	"\\{...\\} or {...} imbalance",
	/* REG_BADBR	*/	"invalid {...} digits",
	/* REG_ERANGE	*/	"invalid [...] range endpoint",
	/* REG_ESPACE	*/	"out of space",
	/* REG_BADRPT	*/	"unary op not preceeded by re",
	/* REG_ENULL	*/	"empty subexpr in pattern",
	/* REG_ECOUNT	*/	"re component count overflow",
	/* REG_BADESC	*/	"invalid \\char escape",
	/* REG_VERSION	*/	&id[5],
};

size_t
regerror(int code, const regex_t* p, char* buf, size_t size)
{
	const char*	s;

	NoP(p);
	s = ++code >= 0 && code < elementsof(reg_error) ? reg_error[code] : (const char*)"unknown error";
	if (size)
	{
		strncpy(buf, s, size);
		buf[size - 1] = 0;
	}
	else
		buf = (char*)s;
	return strlen(buf) + 1;
}
