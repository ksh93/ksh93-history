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
 * posix regex ed-style substitute
 */

#include "reglib.h"

/*
 * do a single substitution
 */

static int
sub(register Sfio_t* dp, const char* op, register const char* sp, size_t nmatch, register regmatch_t* match, register regflags_t flags)
{
	register int	c;
	char*		s;
	char*		e;

	flags &= (REG_SUB_LOWER|REG_SUB_UPPER);
	for (;;)
		switch (c = *sp++)
		{
		case 0:
			return 0;
		case '\\':
			switch (c = *sp++)
			{
			case 0:
				sp--;
				break;
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				c -= '0';
				goto backref;
			default:
				sfputc(dp, chresc(sp - 2, &s));
				sp = (const char*)s;
				break;
			}
			break;
		case '&':
			c = 0;
		backref:
			if (c >= nmatch)
				return REG_ESUBREG;
			s = (char*)op + match[c].rm_so;
			e = (char*)op + match[c].rm_eo;
			while (s < e)
			{
				c = *s++;
				switch (flags)
				{
				case REG_SUB_UPPER:
					if (islower(c))
						c = toupper(c);
					break;
				case REG_SUB_LOWER:
					if (isupper(c))
						c = tolower(c);
					break;
				}
				sfputc(dp, c);
			}
			break;
		default:
			switch (flags)
			{
			case REG_SUB_UPPER:
				if (islower(c))
					c = toupper(c);
				break;
			case REG_SUB_LOWER:
				if (isupper(c))
					c = tolower(c);
				break;
			}
			sfputc(dp, c);
			break;
		}
}

/*
 * ed(1) style substitute using matches from last regexec()
 */

int
regsub(const regex_t* p, Sfio_t* dp, const char* op, const char* sp, size_t nmatch, regmatch_t* match, regflags_t flags)
{
	int	r;

	if ((p->env->flags & REG_NOSUB) || !nmatch)
		return REG_BADPAT;
	do
	{
		sfwrite(dp, op, match->rm_so);
		if (r = sub(dp, op, sp, nmatch, match, flags))
			return fatal(p->env->disc, r, NiL);
		op += match->rm_eo;
	} while ((flags & REG_SUB_ALL) && *op && match->rm_so != match->rm_eo && !(r = regexec(p, op, nmatch, match, p->env->flags)));
	sfputr(dp, op, -1);
	return r == REG_NOMATCH ? 0 : r;
}
