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
 * token stream routines
 */

#include <ast.h>
#include <tok.h>

#define FLG_RESTORE	01		/* restore string on close	*/
#define FLG_NEWLINE	02		/* return newline token next	*/

struct tok				/* token stream state		*/
{
	union
	{
	char*		end;		/* end ('\0') of last token	*/
	struct tok*	nxt;		/* next in free list		*/
	}		ptr;
	char		chr;		/* replace *end with this	*/
	char		flg;		/* FLG_*			*/
};

typedef struct tok TOK;

static struct tok*	freelist;

/*
 * open a new token stream on s
 * if f==0 then string is not restored
 */

char*
tokopen(register char* s, int f)
{
	register TOK*	p;

	if (p = freelist) freelist = freelist->ptr.nxt;
	else if (!(p = newof(0, TOK, 1, 0))) return(0);
	p->chr = *(p->ptr.end = s);
	p->flg = f ? FLG_RESTORE : 0;
	return((char*)p);
}

/*
 * close a token stream
 * restore the string to its original state
 */

void
tokclose(char* u)
{
	register TOK*	p = (TOK*)u;

	if (p->flg == FLG_RESTORE) *p->ptr.end = p->chr;
	p->ptr.nxt = freelist;
	freelist = p;
}

/*
 * return next space separated token
 * "\n" is returned as a token
 * 0 returned when no tokens remain
 * "..." and '...' quotes are honored with \ escapes
 */

char*
tokread(char* u)
{
	register TOK*	p = (TOK*)u;
	register char*	s;
	register char*	r;
	register int	q;
	register int	c;

	/*
	 * restore string on each call
	 */

	if (!p->chr) return(0);
	s = p->ptr.end;
	switch (p->flg)
	{
	case FLG_NEWLINE:
		p->flg = 0;
		return("\n");
	case FLG_RESTORE:
		*s = p->chr;
		break;
	default:
		if (!*s) s++;
		break;
	}

	/*
	 * skip leading space
	 */

	while (*s == ' ' || *s == '\t') s++;
	if (!*s)
	{
		p->ptr.end = s;
		p->chr = 0;
		return(0);
	}

	/*
	 * find the end of this token
	 */

	r = s;
	q = 0;
	for (;;) switch (c = *r++)
	{
	case '\n':
		if (!q)
		{
			if (s == (r - 1))
			{
				if (!p->flg)
				{
					p->ptr.end = r;
					return("\n");
				}
				r++;
			}
			else if (!p->flg) p->flg = FLG_NEWLINE;
		}
		/*FALLTHROUGH*/
	case ' ':
	case '\t':
		if (q) break;
		/*FALLTHROUGH*/
	case 0:
		if (s == --r)
		{
			p->ptr.end = r;
			p->chr = 0;
		}
		else
		{
			p->chr = *(p->ptr.end = r);
			*r = 0;
		}
		return(s);
	case '\\':
		if (*r) r++;
		break;
	case '"':
	case '\'':
		if (c == q) q = 0;
		else if (!q) q = c;
		break;
	}
}
