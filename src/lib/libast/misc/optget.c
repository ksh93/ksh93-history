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
 * command line option parser and usage formatter
 */

#include <optlib.h>
#include <debug.h>
#include <ccode.h>
#include <ctype.h>

#define GO		'{'		/* group nest open		*/
#define OG		'}'		/* group nest close		*/

#define OPT_WIDTH	80		/* default help text width	*/
#define OPT_MARGIN	10		/* default help text margin	*/
#define OPT_USAGE	7		/* usage continuation indent	*/

#define OPT_flag	0001		/* flag ( 0 or 1 )		*/
#define OPT_hidden	0002		/* remaining are hidden		*/
#define OPT_ignorecase	0004		/* arg match ignores case	*/
#define OPT_invert	0010		/* flag inverts long sense	*/
#define OPT_listof	0020		/* arg is ' ' or ',' list	*/
#define OPT_number	0040		/* arg is strton() number	*/
#define OPT_oneof	0100		/* arg may be set once		*/
#define OPT_optional	0200		/* arg is optional		*/
#define OPT_string	0400		/* arg is string		*/

#define OPT_preformat	0001		/* output preformat string	*/

#define OPT_TYPE	(OPT_flag|OPT_number|OPT_string)

#define STYLE_short	0		/* [default] short usage	*/
#define STYLE_long	1		/* long usage			*/
#define STYLE_match	2		/* long description of matches	*/
#define STYLE_options	3		/* short and long descriptions	*/
#define STYLE_man	4		/* pretty details		*/
#define STYLE_html	5		/* html details			*/
#define STYLE_nroff	6		/* nroff details		*/
#define STYLE_api	7		/* program details		*/
#define STYLE_usage	8		/* escaped usage string		*/
#define STYLE_keys	9		/* translation key strings	*/

typedef struct
{
	const char*	name;
	int		flag;
} Attr_t;

typedef struct
{
	const char*	match;		/* builtin help match name	*/
	const char*	name;		/* builtin help name		*/
	int		style;		/* STYLE_*			*/
	const char*	text;		/* --? text			*/
	unsigned int	size;		/* strlen text			*/
} Help_t;

typedef struct Push_s
{
	struct Push_s*	next;		/* next stream			*/
	char*		cp;		/* next char in stream		*/
	Sfio_t*		sp;		/* pushed stream		*/
} Push_t;

typedef struct
{
	int		stop;		/* tab column position		*/
} Indent_t;

static Indent_t		indent[] =
{
	0,2,	4,10,	12,18,	20,26,	28,34,	36,42,	44,50,	0,0
};

static const char	help_head[] = "\
[-?] [--?[item]] [--help[=item]]\n\
OPTIONS\
\t-? and --?* options are the same for all ast commands.\
 For any item below, if --item is not supported by a\
 given command then it is equivalent to --??item.\
 All output is written to the standard error.\
\n";

static const char	help_tail[] = "\
\t  --?-label\t\t\t\tList implementation info matching label*.\n\
\t  --?name\t\t\t\tList descriptions for long options matching name*.\n\
\t  --?\t\t\t\tEquivalent to --??options.\n\
\t  --??\t\t\t\tEquivalent to --??man.\n\
\t  --???\t\t\t\tEquivalent to --??help.\n\
\t  -?\t\t\t\t--??long if any long options otherwise --??short.\n\
\t  --???item\t\t\t\tPrint version=n: n>0 if --??item is supported, 0 otherwise.\n\
\t  --???ESC\t\t\t\tEmit escape codes even if output is not a terminal.\n\
\t  --???TEST\t\t\t\tMassage output for regression testing.";

static const char help_text[] = "List detailed help option info.";

static const char about_text[] = "List all implementation info.";

static const char api_text[] = "List detailed info in program readable form.";

static const char html_text[] = "List detailed info in html.";

static const char keys_text[] = "List the usage translation key strings with C style escapes.";

static const char long_text[] = "List long option usage.";

static const char man_text[] = "List detailed info in displayed man page form.";

static const char nroff_text[] = "List detailed info in nroff.";

static const char options_text[] = "List short and long option details.";

static const char short_text[] = "List short option usage.";

static const char usage_text[] = "List the usage string with C style escapes.";

#define Z(x)	x,sizeof(x)-1

static const Help_t	styles[] =
{
	"about",	"-",		STYLE_match,	Z(about_text),
	"api",		"?api",		STYLE_api,	Z(api_text),
	"help",		"",		-1,		Z(help_text),
	"html",		"?html",	STYLE_html,	Z(html_text),
	"keys",		"?keys",	STYLE_keys,	Z(keys_text),
	"long",		"?long",	STYLE_long,	Z(long_text),
	"man",		"?man",		STYLE_man,	Z(man_text),
	"nroff",	"?nroff",	STYLE_nroff,	Z(nroff_text),
	"options",	"?options",	STYLE_options,	Z(options_text),
	"short",	"?short",	STYLE_short,	Z(short_text),
	"usage",	"?usage",	STYLE_usage,	Z(usage_text),
};

static const Attr_t	attrs[] =
{
	"flag",		OPT_flag,
	"hidden",	OPT_hidden,
	"ignorecase",	OPT_ignorecase,
	"invert",	OPT_invert,
	"listof",	OPT_listof,
	"number",	OPT_number,
	"oneof",	OPT_oneof,
	"optional",	OPT_optional,
	"string",	OPT_string,
};

static const char	unknown[] = "unknown option or attribute";

static const char*	heading[] =
{
	"INDEX",
	"USER COMMANDS",
	"SYSTEM LIBRARY",
	"USER LIBRARY",
	"FILE FORMATS",
	"MISCELLANEOUS",
	"GAMES and DEMOS",
	"SPECIAL FILES",
	"ADMINISTRATIVE COMMANDS",
	"GUIs",
};

Opt_t			opt_info;

__EXTERN__(Opt_t, opt_info);

/*
 * pop a the push stack
 */

static Push_t*
pop(register Push_t* psp)
{
	register Push_t*	tsp;

	while (tsp = psp)
	{
		psp = psp->next;
		sfclose(tsp->sp);
		free(tsp);
	}
	return 0;
}

/*
 * skip over line space to the next token
 */

static char*
next(register char* s, int version)
{
	register char*	b;

	while (*s == '\t' || *s == '\r' || version >= 1 && *s == ' ')
		s++;
	if (*s == '\n')
	{
		b = s;
		while (*++s == ' ' || *s == '\t' || *s == '\r');
		if (*s == '\n')
			return b;
	}
	return s;
}

/*
 * skip to t1 or t2 or t3, whichever first, in s
 *	n==0	outside [...]
 *	n==1	inside [...] before ?
 *	n==2	inside [...] after ?
 *	b==0	outside {...}
 *	b==1	inside {...}
 * past skips past the terminator to the next token
 * otherwise a pointer to the terminator is returned
 *
 * ]] for ] inside [...]
 * ?? for ? inside [...] before ?
 */

static char*
skip(register char* s, register int t1, register int t2, register int t3, register int n, register int b, int past, int version)
{
	register int	c;
	register int	on = n;
	register int	ob = b;

	if (version < 1)
	{
		n = n >= 1;
		for (;;)
		{
			switch (*s++)
			{
			case 0:
				break;
			case '[':
				n++;
				continue;
			case ']':
				if (--n <= 0)
					break;
				continue;
			default:
				continue;
			}
			break;
		}
	}
	else while (c = *s++)
	{
		message((-22, "optget: skip t1=%c t2=%c t3=%c n=%d b=%d `%-.16s'", t1 ? t1 : '@', t2 ? t2 : '@', t3 ? t3 : '@', n, b, s - 1));
		if (c == '[')
		{
			if (!n)
				n = 1;
		}
		else if (c == ']')
		{
			if (n)
			{
				if (*s == ']')
					s++;
				else if (on == 1)
					break;
				else
					n = 0;
			}
		}
		else if (c == GO)
		{
			if (n == 0)
				b++;
		}
		else if (c == OG)
		{
			if (n == 0 && b-- == ob)
				break;
		}
		else if (c == '?')
		{
			if (n == 1)
			{
				if (*s == '?')
					s++;
				else
				{
					if (n == on && (c == t1 || c == t2 || c == t3))
						break;
					n = 2;
				}
			}
		}
		else if (n == on && (c == t1 || c == t2 || c == t3))
			break;
	}
	return past && *(s - 1) ? next(s, version) : s - 1;
}

/*
 * initialize the attributes for pass p from opt string s
 */

static void
init(register char* s, Optpass_t* p)
{
	register char*	t;
	register int	c;
	register int	n;
	char*		dictionary;
	char		buf[PATH_MAX];

#if _BLD_DEBUG
	error(-1, "optget debug");
#endif
	p->oopts = s;
	p->disc = opt_info.disc;
	p->version = 0;
	p->prefix = 2;
	p->section = 1;
	dictionary = (p->disc && p->disc->dictionary) ? p->disc->dictionary : error_info.id;
	s = next(s, 0);
	if (*s == ':')
		s++;
	if (*s == '+')
		s++;
	s = next(s, 0);
	if (*s++ == '[')
	{
		if (*s == '+')
			p->version = 1;
		else if (*s++ == '-')
		{
			if (*s == '?' || *s == ']')
				p->version = 1;
			else
			{
				if (*s < '0' || *s > '9')
					p->version = 1;
				else
					while (*s >= '0' && *s <= '9')
						p->version = p->version * 10 + (*s++ - '0');
				while (*s && *s != '?' && *s != ']')
				{
					c = *s++;
					if (*s < '0' || *s > '9')
						n = 1;
					else
					{
						n = 0;
						while (*s >= '0' && *s <= '9')
							n = n * 10 + (*s++ - '0');
					}
					switch (c)
					{
					case 'i':
						p->flags |= OPT_ignore;
						break;
					case 'l':
						p->flags |= OPT_long;
						break;
					case 'p':
						p->prefix = n;
						break;
					case 's':
						p->section = n;
						if (n > 1 && n < 6)
						{
							p->flags |= OPT_functions;
							p->prefix = 0;
						}
						break;
					}
				}
			}
		}
		while (*s)
			if (*s++ == ']' && *s++ == '[')
			{
				if (*s++ != '-')
					break;
				if (*s == '-')
					s++;
				if (strneq(s, "dictionary?", 11))
				{
					s += 11;
					if (t = strchr(s, ']'))
					{
						if ((n = (t - s)) >= sizeof(buf))
							n = sizeof(buf) - 1;
						memcpy(dictionary = buf, s, n);
						dictionary[n] = 0;
					}
					break;
				}
			}
	}
	s = ERROR_translate(dictionary, p->oopts);
	if (*s == ':')
		s++;
	if (*s == '+')
	{
		s++;
		opt_info.plus = 1;
	}
	p->opts = s;
}

/*
 * return the bold set/unset sequence for style
 */

static char*
bold(int style, int set)
{
	static const char	on[] = { CC_esc, '[', '1', 'm', 0 };
	static const char	no[] = { CC_esc, '[', '0', 'm', 0 };

	switch (style)
	{
	case STYLE_html:
		return set ? "<B>" : "</B>";
	case STYLE_nroff:
		return set ? "\\fB" : "\\fP";
	case STYLE_short:
	case STYLE_long:
	case STYLE_api:
		break;
	default:
		if (opt_info.emphasis > 0)
			return set ? (char*)on : (char*)no;
		break;
	}
	return "";
}

/*
 * return the italic set/unset sequence for style
 */

static char*
italic(int style, int set)
{
	static const char	on[] = { CC_esc, '[', '1', ';', '4', 'm', 0 };
	static const char	no[] = { CC_esc, '[', '0', 'm', 0 };

	switch (style)
	{
	case STYLE_html:
		return set ? "<I>" : "</I>";
	case STYLE_nroff:
		return set ? "\\fI" : "\\fP";
	case STYLE_short:
	case STYLE_long:
	case STYLE_api:
		break;
	default:
		if (opt_info.emphasis > 0)
			return set ? (char*)on : (char*)no;
		break;
	}
	return "";
}

/*
 * return \f...\f info
 * *stk is set to the next input char pointer
 * discipline infof puts info in ip
 */

static char*
info(register char* p, char** stk, Sfio_t* ip)
{
	register int	c;
	register char*	b = p;
	int		x;

	while ((c = *p++) && c != '\f');
	sfwrite(ip, b, p - b - 1);
	sfputc(ip, 0);
	b = sfstrbase(ip);
	x = sfstrtell(ip);
	if (!c)
		p--;
	*stk = p;
	if (*b == '?')
	{
		if (!*++b || streq(b, "NAME"))
		{
			if (!(b = error_info.id))
				b = "command";
		}
		sfstrset(ip, 0);
	}
	else if (opt_info.disc && opt_info.disc->infof && (*opt_info.disc->infof)(&opt_info, ip, b, opt_info.disc) >= 0)
		b = sfstruse(ip) + x;
	else
		sfstrset(ip, 0);
	return b;
}

/*
 * output label s from [ ...label...[?...] ] to sp
 */

static void
label(register Sfio_t* sp, int sep, register char* s, int z, int style, int a, int b, int version)
{
	register int	c;
	register char*	t;
	register char*	e;
	int		ostyle;
	int		n = 1;
	char*		h;
	char*		u;
	char*		x = 0;

	if ((ostyle = style) > (STYLE_nroff - (sep <= 0)))
		style = 0;
	if (z < 0)
		e = s + strlen(s);
	else
		e = s + z;
	if (sep > 0)
	{
		if (sep == ' ' && style == STYLE_nroff)
			sfputc(sp, '\\');
		sfputc(sp, sep);
	}
	sep = !sep || z < 0;
	if (version < 1)
	{
		b = 0;
		for (;;)
		{
			if (s >= e)
				return;
			switch (c = *s++)
			{
			case '[':
				b++;
				break;
			case ']':
				if (--b < 0)
					return;
				break;
			}
			sfputc(sp, c);
		}
	}
	switch (*s)
	{
	case '\a':
		if (a)
		{
			a = 0;
			s++;
		}
		else
			b = 0;
		break;
	case '\b':
		if (b)
		{
			b = 0;
			s++;
		}
		else
			a = 0;
		break;
	default:
		if (a)
			sfputr(sp, italic(style, 1), -1);
		else if (b)
			sfputr(sp, bold(style, 1), -1);
		break;
	}
	for (;;)
	{
		if (s >= e)
		{
			if (!(s = x))
				goto restore;
			x = 0;
			e = h;
			continue;
		}
		switch (c = *s++)
		{
		case '(':
			if (n)
			{
				n = 0;
				if (a)
					sfputr(sp, bold(style, a = !a), -1);
				else if (b)
					sfputr(sp, bold(style, b = !b), -1);
			}
			break;
		case '?':
			if (sep && *s++ != '?')
				goto restore;
			break;
		case ']':
			if (sep && *s++ != ']')
				goto restore;
			break;
		case ':':
			if (sep && *s++ != ':')
				goto restore;
			break;
		case '\a':
			if (b)
				sfputr(sp, bold(style, b = !b), -1);
			if (!a && style == STYLE_html)
			{
				for (t = s; t < e && !isspace(*t) && !iscntrl(*t); t++);
				if (*t == '\a' && *++t == '(')
				{
					u = t;
					while (++t < e && isdigit(*t));
					if (t < e && *t == ')' && t > u + 1)
					{
						sfprintf(sp, "<NOBR><A HREF=\"../man%-.*s/%-.*s.html\"><I>%-.*s</I></A>%-.*s</NOBR>"
							, t - u - 1, u + 1
							, u - s - 1, s
							, u - s - 1, s
							, t - u + 1, u
							);
						s = t + 1;
						continue;
					}
				}
			}
			sfputr(sp, italic(style, a = !a), -1);
			continue;
		case '\b':
			if (a)
				sfputr(sp, italic(style, a = !a), -1);
			if (!b && style == STYLE_html)
			{
				for (t = s; t < e && !isspace(*t) && !iscntrl(*t); t++);
				if (*t == '\b' && *++t == '(')
				{
					u = t;
					while (++t < e && isdigit(*t));
					if (*t == ')' && t > u + 1)
					{
						sfprintf(sp, "<NOBR><A HREF=\"../man%-.*s/%-.*s.html\"><B>%-.*s</B></A>%-.*s</NOBR>"
							, t - u - 1, u + 1
							, u - s - 1, s
							, u - s - 1, s
							, t - u + 1, u
							);
						s = t + 1;
						continue;
					}
				}
			}
			sfputr(sp, bold(style, b = !b), -1);
			continue;
		case '\f':
			if (!x)
			{
				s = info(s, &x, opt_info.mp);
				h = e;
				e = s + strlen(s);
			}
			continue;
		case '<':
			if (style == STYLE_html)
			{
				sfputr(sp, "&lt;", -1);
				continue;
			}
			break;
		case '>':
			if (style == STYLE_html)
			{
				sfputr(sp, "&gt;", -1);
				continue;
			}
			break;
		case '&':
			if (style == STYLE_html)
			{
				sfputr(sp, "&amp;", -1);
				continue;
			}
			break;
		case ' ':
		case '\\':
			if (ostyle == STYLE_nroff)
				sfputc(sp, '\\');
			break;
		}
		sfputc(sp, c);
	}
 restore:
	if (a)
		sfputr(sp, italic(style, 0), -1);
	else if (b)
		sfputr(sp, bold(style, 0), -1);
}

/*
 * output args description to sp from p of length n
 */

static void
args(register Sfio_t* sp, register char* p, register int n, int flags, int style, int version)
{
	register int	i;
	register char*	t;
	register char*	o;
	register char*	a = 0;
	char*		b;
	int		sep;

	if (flags & OPT_functions)
		sep = '\t';
	else
	{
		sep = ' ';
		o = ERROR_translate(ast.id, "options");
		b = style == STYLE_nroff ? "\\ " : " ";
		for (;;)
		{
			t = (char*)memchr(p, '\n', n);
			if (style >= STYLE_man)
			{
				if (!(a = error_info.id))
					a = "...";
				sfprintf(sp, "\t%s%s%s%s[%s%s%s%s%s]", bold(style, 1), a, bold(style, 0), b, b, italic(style, 1), o, italic(style, 0), b);
			}
			else if (a)
				sfprintf(sp, "%*.*s%s%s%s[%s%s%s]", OPT_USAGE - 1, OPT_USAGE - 1, ERROR_translate(ast.id, "Or:"), b, a, b, b, o, b);
			else
			{
				if (!(a = error_info.id))
					a = "...";
				if (!sfstrtell(sp))
					sfprintf(sp, "[%s%s%s]", b, o, b);
			}
			if (!t)
				break;
			i = ++t - p;
			if (i)
			{
				sfputr(sp, b, -1);
				sfwrite(sp, p, i);
			}
			if (style == STYLE_html)
				sfputr(sp, "<BR>", '\n');
			else if (style == STYLE_nroff)
				sfputr(sp, ".br", '\n');
			else if (style == STYLE_api)
				sfputr(sp, ".BR", '\n');
			p = t;
			n -= i;
			while (n > 0 && (*p == ' ' || *p == '\t'))
			{
				p++;
				n--;
			}
		}
	}
	if (n)
		label(sp, sep, p, n, style, 0, 0, version);
}

/*
 * output [+-...label...?...] label s to sp
 * according to {...} level and style
 * return 0:header 1:paragraph
 */

static int
item(Sfio_t* sp, char* s, int level, int style, int version)
{
	int	n;
	int	par;

	sfputc(sp, '\n');
	if (*s != ']' && (*s != '?' || *(s + 1) == '?'))
	{
		par = 0;
		if (level)
		{
			if (style >= STYLE_nroff)
				sfprintf(sp, ".H%d ", (level + 1) / 2);
			else
				for (n = 0; n < level; n++)
					sfputc(sp, '\t');
		}
		if (style == STYLE_html)
		{
			if (!level)
				sfputr(sp, "<H4>", -1);
			sfputr(sp, "<A NAME=\"", -1);
			label(sp, 0, s, -1, 0, 0, 0, version);
			sfputr(sp, "\">", -1);
			label(sp, 0, s, -1, style, 0, !!level, version);
			sfputr(sp, "</A>", -1);
			if (!level)
				sfputr(sp, "</H4>", -1);
		}
		else
		{
			if (!level)
			{
				if (style >= STYLE_nroff)
					sfprintf(sp, ".SH ");
				else if (style == STYLE_man)
					sfputc(sp, '\n');
				else if (style != STYLE_options && style != STYLE_match || *s == '-' || *s == '+')
					sfputc(sp, '\t');
			}
			label(sp, 0, s, -1, style, 0, 1, version);
		}
	}
	else
	{
		par = 1;
		if (style >= STYLE_nroff)
			sfputr(sp, ".PP", -1);
	}
	if (style >= STYLE_nroff || !level)
		sfputc(sp, '\n');
	if (par && style < STYLE_nroff)
		for (n = 0; n < level; n++)
			sfputc(sp, '\t');
	return par;
}

/*
 * output text to sp from p according to style
 */

static char*
text(Sfio_t* sp, register char* p, int style, int level, int bump, Sfio_t* ip, int version)
{
	register char*	t;
	register int	c;
	register int	n;
	char*		e;
	int		a;
	int		b;
	int		par;

	int		lev = level;
	char*		stk = 0;

 again:
	if ((c = *p) == GO)
	{
		for (;;)
		{
			while (*(p = next(p + 1, version)) == '\n');
			if (*p == GO)
			{
				if (level > 1)
					level++;
				level++;
			}
			else if (*p != OG)
				break;
			else if ((level -= 2) <= lev)
				return p + 1;
		}
		if (*p == '\f' && !stk)
			p = info(p + 1, &stk, ip);
		if (*p != '[')
			return p;
		c = *++p;
		if (level > 1)
			level++;
		level++;
	}
	if (c == '-' && level > 1)
		return skip(p, 0, 0, 0, 1, level, 1, version);
	if (c == '+' || c == '-' && (bump = 3))
	{
		p = skip(t = p + 1, '?', 0, 0, 1, level, 0, version);
		if (c == '-' && (*t == '?' || *t >= '0' && *t <= '9'))
		{
			if (*p != '?')
				return skip(p, 0, 0, 0, 1, level, 1, version);
			par = item(sp, ERROR_translate(ast.id, "version"), level, style, version);
			if ((c = *p) == '?')
			{
				e = p; 
				while (isspace(*++e));
				if (*e++ == '@' && *e++ == '(' && *e++ == '#' && *e == ')')
					p = e;
			}
		}
		else
		{
			par = item(sp, t, level, style, version);
			c = *p;
		}
		if (level)
			par = 0;
	}
	else
	{
		if (style >= STYLE_nroff)
			sfputc(sp, '\n');
		else if (c == '?')
			for (n = 0; n < level; n++)
				sfputc(sp, '\t');
		par = 0;
	}
	if (c == ':')
		c = *(p = skip(p, '?', 0, 0, 1, 0, 0, version));
	if (c == ']' && (c = *(p = next(p + 1, version))) == GO)
		p = text(sp, p, style, level + bump + par + 1, 0, ip, version);
	else if (c == '?' || c == ' ')
	{
		p++;
		if (c != '?')
			sfputc(sp, c);
		else if (style < STYLE_nroff)
			for (n = 0; n < bump + 1; n++)
				sfputc(sp, '\t');
		a = b = 0;
		for (;;)
		{
			switch (c = *p++)
			{
			case 0:
				if (!stk)
				{
					if (a)
						sfputr(sp, italic(style, 0), -1);
					else if (b)
						sfputr(sp, bold(style, 0), -1);
					return p - 1;
				}
				p = stk;
				stk = 0;
				continue;
			case ']':
				if (*p != ']')
				{
					if (a)
					{
						a = 0;
						sfputr(sp, italic(style, 0), -1);
					}
					else if (b)
					{
						b = 0;
						sfputr(sp, bold(style, 0), -1);
					}
					for (;;)
					{
						if (*(p = next(p, version)) == GO)
							p = text(sp, p, style, level + bump + !level, 0, ip, version);
						else if (*p == '[' && level > lev)
						{
							p++;
							goto again;
						}
						else if (!*p)
						{
							if (!stk)
								break;
							p = stk;
							stk = 0;
						}
						else if (*p != OG)
							break;
						else
						{
							p++;
							if ((level -= 2) <= lev)
								break;
						}
					}
					return p;
				}
				p++;
				break;
			case '\a':
				if (b)
					sfputr(sp, bold(style, b = !b), -1);
				if (!a && style == STYLE_html)
				{
					for (t = p; *t && !isspace(*t) && !iscntrl(*t); t++);
					if (*t == '\a' && *++t == '(')
					{
						e = t;
						while (isdigit(*++t));
						if (*t == ')' && t > e + 1)
						{
							sfprintf(sp, "<NOBR><A HREF=\"../man%-.*s/%-.*s.html\"><I>%-.*s</I></A>%-.*s</NOBR>"
								, t - e - 1, e + 1
								, e - p - 1, p
								, e - p - 1, p
								, t - e + 1, e
								);
							p = t + 1;
							continue;
						}
					}
				}
				sfputr(sp, italic(style, a = !a), -1);
				continue;
			case '\b':
				if (a)
					sfputr(sp, italic(style, a = !a), -1);
				if (!b && style == STYLE_html)
				{
					for (t = p; *t && !isspace(*t) && !iscntrl(*t); t++);
					if (*t == '\b' && *++t == '(')
					{
						e = t;
						while (isdigit(*++t));
						if (*t == ')' && t > e + 1)
						{
							sfprintf(sp, "<NOBR><A HREF=\"../man%-.*s/%-.*s.html\"><B>%-.*s</B></A>%-.*s</NOBR>"
								, t - e - 1, e + 1
								, e - p - 1, p
								, e - p - 1, p
								, t - e + 1, e
								);
							p = t + 1;
							continue;
						}
					}
				}
				sfputr(sp, bold(style, b = !b), -1);
				continue;
			case '\f':
				if (stk)
					continue;
				p = info(p, &stk, ip);
				continue;
			case ' ':
			case '\n':
			case '\r':
			case '\t':
				while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')
					p++;
				if (*p == ']' && *(p + 1) != ']')
					continue;
				c = ' ';
				break;
			case '<':
				if (style == STYLE_html)
				{
					sfputr(sp, "&lt;", -1);
					continue;
				}
				break;
			case '>':
				if (style == STYLE_html)
				{
					sfputr(sp, "&gt;", -1);
					continue;
				}
				break;
			case '&':
				if (style == STYLE_html)
				{
					sfputr(sp, "&amp;", -1);
					continue;
				}
				break;
			case '\\':
				if (style == STYLE_nroff)
					sfputc(sp, c);
				break;
			}
			sfputc(sp, c);
		}
	}
	return p;
}

/*
 * return pointer to help message sans `Usage: command'
 * if oopts is 0 then opt_info.pass is used
 * what:
 *	0	?short by default, ?long if any long options used
 *	*	otherwise see help_text[] (--???)
 * external formatter:
 *	\a...\a	italic
 *	\b...\b	bold
 *	\f...\f	discipline infof callback on ...
 * internal formatter:
 *	\t	indent
 *	\n	newline
 * margin flush pops to previous indent
 */

char*
opthelp(const char* oopts, const char* what)
{
	register Sfio_t*	sp;
	register Sfio_t*	mp;
	register int		c;
	register char*		p;
	register Indent_t*	ip;
	char*			t;
	char*			x;
	char*			w;
	char*			u;
	char*			y;
	char*			s;
	char*			d;
	char*			v;
	char*			name;
	char*			pp;
	char*			rb;
	char*			re;
	int			f;
	int			i;
	int			j;
	int			m;
	int			n;
	int			a;
	int			sl;
	int			vl;
	int			wl;
	int			xl;
	int			rm;
	int			ts;
	int			co;
	int			z;
	int			style;
	int			head;
	int			mode;
	int			mutex;
	int			prefix;
	int			version;
	long			tp;
	Push_t*			tsp;
	Optpass_t*		o;
	Optpass_t*		q;
	Optpass_t*		e;
	Optpass_t		one;
	Help_t*			hp;
	Optdisc_t*		dp;
	short			ptstk[elementsof(indent) + 2];
	short*			pt;

	char*			opts = (char*)oopts;
	int			flags = 0;
	int			match = 0;
	int			section = 1;
	Push_t*			psp = 0;
	Sfio_t*			sp_text = 0;
	Sfio_t*			sp_plus = 0;
	Sfio_t*			sp_head = 0;
	Sfio_t*			sp_body = 0;
	Sfio_t*			sp_info = 0;
	Sfio_t*			sp_misc = 0;

	dp = opt_info.disc;
	if (!(mp = opt_info.mp) && !(mp = opt_info.mp = sfstropen()))
		goto nospace;
	if (!what)
		style = opt_info.style;
	else if (!*what)
		style = STYLE_options;
	else if (*what != '?')
		style = STYLE_match;
	else if (!*(what + 1))
		style = STYLE_man;
	else if ((hp = (Help_t*)strpsearch(styles, elementsof(styles), sizeof(styles[0]), what + 1, NiL)) && hp->style >= 0)
	{
		style = hp->style;
		if (*hp->name != '?')
			what = hp->name;
	}
	else
	{
		sp = 0;
		style = 0;
		if (!(sp = sfstropen()))
			goto nospace;
		sfwrite(sp, help_head, sizeof(help_head) - 1);
		for (i = 0; i < elementsof(styles); i++)
			sfprintf(sp, "\t  --??%s\t\t\t\t%s\n", styles[i].match, styles[i].text);
		sfwrite(sp, help_tail, sizeof(help_tail) - 1);
		p = sfstruse(sp);
		goto format;
	}
	if (opts)
	{
		for (i = 0; i < opt_info.npass; i++)
			if (opt_info.pass[i].oopts == opts)
			{
				o = &opt_info.pass[i];
				break;
			}
		if (i >= opt_info.npass)
		{
			o = &one;
			init((char*)opts, o);
		}
		e = o + 1;
	}
	else
	{
		o = opt_info.pass;
		e = o + opt_info.npass;
	}
	if (e == o)
		return "[* call optget() before opthelp() *]";
	if (style < STYLE_usage)
	{
		if (!(sp_text = sfstropen()) || !(sp_info = sfstropen()))
			goto nospace;
		if (style >= STYLE_match && !(sp_body = sfstropen()))
			goto nospace;
	}
	switch (style)
	{
	case STYLE_api:
	case STYLE_html:
	case STYLE_nroff:
		opt_info.emphasis = 0;
		break;
	case STYLE_usage:
	case STYLE_keys:
		sfputc(mp, '\f');
		break;
	default:
		if (!opt_info.emphasis)
		{
			if (x = getenv("ERROR_OPTIONS"))
			{
				if (strmatch(x, "*noemphasi*"))
					break;
				if (strmatch(x, "*emphasi*"))
				{
					opt_info.emphasis = 1;
					break;
				}
			}
			if ((x = getenv("TERM")) && strmatch(x, "(ansi|vt100|xterm)*") && isatty(sffileno(sfstderr)))
				opt_info.emphasis = 1;
		}
		break;
	}
	x = "";
	xl = 0;
	for (q = o; q < e; q++)
	{
		if (q->flags & OPT_ignore)
			continue;
		if (section < q->section)
			section = q->section;
		section = q->section;
		flags |= q->flags;
		p = q->opts;
		switch (style)
		{
		case STYLE_usage:
			if (xl)
				sfputc(mp, '\n');
			else
				xl = 1;
			while (c = *p++)
			{
				switch (c)
				{
				case '\a':
					c = 'a';
					break;
				case '\b':
					c = 'b';
					break;
				case '\f':
					c = 'f';
					break;
				case '\n':
					c = 'n';
					break;
				case '\r':
					c = 'r';
					break;
				case '\t':
					c = 't';
					break;
				case '\v':
					c = 'v';
					break;
				case '"':
					c = '"';
					break;
				case '\'':
					c = '\'';
					break;
				case '\\':
					c = '\\';
					break;
				default:
					sfputc(mp, c);
					continue;
				}
				sfputc(mp, '\\');
				sfputc(mp, c);
			}
			continue;
		case STYLE_keys:
			a = 0;
			while (c = *p++)
			{
				if (a == 0 && (c == ' ' || c == '\n' && *p == '\n'))
				{
					if (*p == '\n')
						p++;
					a = c;
				}
				else if (c != '[')
					continue;
				else if (*p == '-')
				{
					while (*p)
						if (*p++ == ']')
						{
							if (!*p || *p != ']')
								break;
							p++;
						}
					continue;
				}
				else
					while (*p)
						if (*p++ == '?')
						{
							if (!*p || *p != '?')
								break;
							p++;
						}
				if (!*p)
					break;
				if (xl)
					sfputc(mp, '\n');
				else
					xl = 1;
				sfputc(mp, '"');
				for (;;)
				{
					if (!(c = *p++))
					{
						p--;
						break;
					}
					if (a > 0)
					{
						if (c == '\n')
						{
							if (a == '\n' && *p == '\n')
							{
								a = -1;
								p++;
								break;
							}
							if (a == ' ')
							{
								a = -1;
								break;
							}
							else if (*p == '\n')
							{
								a = -1;
								p++;
								break;
							}
						}
					}
					else if (c == ']')
					{
						if (*p != ']')
							break;
						p++;
					}
					switch (c)
					{
					case '\a':
						c = 'a';
						break;
					case '\b':
						c = 'b';
						break;
					case '\f':
						c = 'f';
						break;
					case '\n':
						c = 'n';
						break;
					case '\r':
						c = 'r';
						break;
					case '\t':
					case ' ':
						while ((c = *p) == ' ' || c == '\t' || c == '\n')
							p++;
						sfputc(mp, ' ');
						continue;
					case '\v':
						c = 'v';
						break;
					case '"':
						c = '"';
						break;
					case '\\':
						c = '\\';
						break;
					default:
						sfputc(mp, c);
						continue;
					}
					sfputc(mp, '\\');
					sfputc(mp, c);
				}
				sfputc(mp, '"');
			}
			continue;
		}
		z = 0;
		head = 0;
		mode = 0;
		mutex = 0;
		prefix = q->prefix;
		version = q->version;
		opt_info.disc = q->disc;
		if (style > STYLE_short && style < STYLE_nroff && version < 1)
		{
			style = STYLE_short;
			if (sp_body)
			{
				sfclose(sp_body);
				sp_body = 0;
			}
		}
		else if (style <= STYLE_short && prefix < 2)
			style = STYLE_long;
		if (*p == ':')
			p++;
		if (*p == '+')
		{
			p++;
			if (!(sp = sp_plus) && !(sp = sp_plus = sfstropen()))
				goto nospace;
		}
		else if (style >= STYLE_match)
			sp = sp_body;
		else
			sp = sp_text;
		psp = 0;
		for (;;)
		{
			if (!(*(p = next(p, version))))
			{
				if (!(tsp = psp))
					break;
				p = psp->cp;
				sfclose(psp->sp);
				psp = psp->next;
				free(tsp);
			}
			if (*p == '\f')
			{
				if (!(tsp = newof(0, Push_t, 1, 0)) || !(tsp->sp = sfstropen()))
				{
					if (tsp)
						free(tsp);
					goto nospace;
				}
				tsp->next = psp;
				psp = tsp;
				p = info(p + 1, &psp->cp, psp->sp);
				continue;
			}
			if (*p == '\n' || *p == ' ')
			{
				if (*(x = p = next(p + 1, version)))
					while (*++p)
						if (*p == '\n')
						{
							while (*++p == ' ' || *p == '\t' || *p == '\r');
							if (*p == '\n')
								break;
						}
				xl = p - x;
				if (!*p)
					break;
				continue;
			}
			message((-20, "opthelp: opt %-.16s", p));
			if (z < 0)
				z = 0;
			a = 0;
			f = 0;
			w = 0;
			d = 0;
			s = 0;
			sl = 0;
			if (*p == '[')
			{
				if ((c = *(p = next(p + 1, version))) == '-')
				{
					if (style >= STYLE_man)
					{
						if (*(p + 1) != '-')
						{
							if (!sp_misc && !(sp_misc = sfstropen()))
								goto nospace;
							else
								p = text(sp_misc, p, style, 1, 3, sp_info, version);
							continue;
						}
					}
					else if (style == STYLE_match && *what == '-')
					{
						if (*(p + 1) == '?' || *(p + 1) >= '0' && *(p + 1) <= '9')
							s = ERROR_translate(ast.id, "-version");
						else
							s = p;
						w = (char*)what;
						if (*(s + 1) != '-' || *(w + 1) == '-')
						{
							if (*(s + 1) == '-')
								s++;
							if (*(w + 1) == '-')
								w++;
							while (*++w && *w == *++s);
							if (!*w)
							{
								if (*(p + 1) == '-')
									p++;
								p = text(sp, p, style, 1, 3, sp_info, version);
								match = -1;
								continue;
							}
						}
					}
					if (!z)
						z = -1;
				}
				else if (c == '+')
				{
					if (style >= STYLE_man)
					{
						p = text(sp_body, p, style, 0, 0, sp_info, version);
						if (!sp_head)
						{
							sp_head = sp_body;
							if (!(sp_body = sfstropen()))
								goto nospace;
						}
						continue;
					}
					if (!z)
						z = -1;
				}
				else if (*p == '[' || version < 1)
				{
					mutex++;
					continue;
				}
				else
				{
					if (*p == '!')
					{
						a |= OPT_invert;
						p++;
					}
					rb = p;
					if (*p != ':')
					{
						s = p;
						if (*(p + 1) == '|')
						{
							while (*++p && *p != '=' && *p != '!' && *p != ':' && *p != '?');
							if ((p - s) > 1)
								sl = p - s;
							if (*p == '!')
								a |= OPT_invert;
						}
						p = skip(p, ':', '?', 0, 1, 0, 0, version);
						if (sl || (p - s) == 1 || *(s + 1) == '=' || *(s + 1) == '!' && (a |= OPT_invert) || *(s + 1) == '|')
							f = *s;
					}
					re = p;
					if (style == STYLE_short)
					{
						if (!z && !f)
							z = -1;
					}
					else
					{
						if (*p == ':')
						{
							p = skip(w = p + 1, ':', '?', 0, 1, 0, 0, version);
							if (!(wl = p - w))
								w = 0;
						}
						else
							wl = 0;
						if (*p == ':' || *p == '?')
						{
							d = p;
							p = skip(p, 0, 0, 0, 1, 0, 0, version);
						}
						else
							d = 0;
						if (style == STYLE_match)
						{
							for (i = j = 0; j < wl; j++)
								if (!what[i])
									break;
								else if (w[j] == what[i])
									i++;
								else if (w[j] == '-')
									/*OK*/;
								else if (w[j] != '*')
								{
									while (j < wl && w[j] != '|')
										j++;
									if (j >= wl)
										break;
									i = 0;
								}
							if (what[i])
								wl = 0;
							if ((!wl || *w == ':' || *w == '?') && (what[1] || sl && !memchr(s, what[0], sl) || !sl && what[0] != f))
							{
								w = 0;
								if (!z)
									z = -1;
							}
							else
								match = 1;
						}
					}
				}
				p = skip(p, 0, 0, 0, 1, 0, 1, version);
				if (*p == GO)
					p = skip(p + 1, 0, 0, 0, 0, 1, 1, version);
			}
			else if (*p == ']')
			{
				if (mutex)
				{
					if (style >= STYLE_nroff)
						sfputr(sp_body, "\n.OP - - anyof", '\n');
					if (!(mutex & 1))
					{
						mutex--;
						if (style <= STYLE_long)
						{
							sfputc(sp_body, ' ');
							sfputc(sp_body, ']');
						}
					}
					mutex--;
				}
				p++;
				continue;
			}
			else if (*p == '?')
			{
				if (style < STYLE_match)
					z = 1;
				mode |= OPT_hidden;
				p++;
				continue;
			}
			else
			{
				f = *p++;
				s = 0;
				if (style == STYLE_match && !z)
					z = -1;
			}
			if (!z)
			{
				if (style == STYLE_long || prefix < 2 || (q->flags & OPT_long))
					f = 0;
				else if (style == STYLE_short)
					w = 0;
				if (!f && !w)
					z = -1;
			}
			u = v = y = 0;
			if (*p == ':' && (a |= OPT_string) || *p == '#' && (a |= OPT_number))
			{
				message((-21, "opthelp: arg %-.16s", p));
				if (*++p == '?' || *p == *(p - 1))
				{
					p++;
					a |= OPT_optional;
				}
				if (*(p = next(p, version)) == '[')
				{
					if (!z)
					{
						p = skip(y = p + 1, ':', '?', 0, 1, 0, 0, version);
						while (*p == ':')
						{
							p = skip(t = p + 1, ':', '?', 0, 1, 0, 0, version);
							m = p - t;
							for (j = 0; j < elementsof(attrs); j++)
								if (*t == '=')
								{
									v = t + 1;
									vl = m - 1;
								}
								else if (strneq(t, attrs[j].name, m))
								{
									a |= attrs[j].flag;
									break;
								}
						}
						if (*p == '?')
							u = p;
						p = skip(p, 0, 0, 0, 1, 0, 1, version);
					}
					else
						p = skip(p + 1, 0, 0, 0, 1, 0, 1, version);
				}
				else
					y = (a & OPT_number) ? ERROR_translate(ast.id, "#") : ERROR_translate(ast.id, "arg");
			}
			else
				a |= OPT_flag;
			if (!z)
			{
				if (style == STYLE_short && !y && !mutex)
				{
					if (!sfstrtell(sp))
					{
						sfputc(sp, '[');
						if (sp == sp_plus)
							sfputc(sp, '+');
						sfputc(sp, '-');
					}
					if (!sl)
						sfputc(sp, f);
					else
						for (c = 0; c < sl; c++)
							if (s[c] != '|')
								sfputc(sp, s[c]);
				}
				else
				{
					if (style >= STYLE_match)
					{
						sfputc(sp_body, '\n');
						if (!head)
						{
							head = 1;
							item(sp_body, (flags & OPT_functions) ? ERROR_translate(ast.id, "FUNCTIONS") : ERROR_translate(ast.id, "OPTIONS"), 0, style, version);
						}
						if (style >= STYLE_nroff)
						{
							if (mutex & 1)
							{
								mutex++;
								sfputr(sp_body, "\n.OP - - oneof", '\n');
							}
						}
						else
							sfputc(sp_body, '\t');
					}
					else
					{
						if (sp_body)
							sfputc(sp_body, ' ');
						else if (!(sp_body = sfstropen()))
							goto nospace;
						if (mutex)
						{
							if (mutex & 1)
							{
								mutex++;
								sfputc(sp_body, '[');
							}
							else
								sfputc(sp_body, '|');
							sfputc(sp_body, ' ');
						}
						else
							sfputc(sp_body, '[');
					}
					if (style >= STYLE_nroff)
					{
						if (flags & OPT_functions)
						{
							sfputr(sp_body, ".FN", ' ');
							if (re > rb)
								sfwrite(sp_body, rb, re - rb);
							else
								sfputr(sp, "void", -1);
							if (w)
								label(sp_body, ' ', w, -1, style, 0, 1, version);
						}
						else
						{
							sfputr(sp_body, ".OP", ' ');
							if (sl)
								sfwrite(sp_body, s, sl);
							else
								sfputc(sp_body, f ? f : '-');
							sfputc(sp_body, ' ');
							if (w)
								label(sp_body, 0, w, -1, style, 0, 0, version);
							else
								sfputc(sp_body, '-');
							sfputc(sp_body, ' ');
							m = a & OPT_TYPE;
							for (j = 0; j < elementsof(attrs); j++)
								if (m & attrs[j].flag)
								{
									sfputr(sp_body, attrs[j].name, -1);
									break;
								}
							if (m = (a & ~m) | mode)
								for (j = 0; j < elementsof(attrs); j++)
									if (m & attrs[j].flag)
									{
										sfputc(sp_body, ':');
										sfputr(sp_body, attrs[j].name, -1);
									}
							sfputc(sp_body, ' ');
							if (y)
								label(sp_body, 0, y, -1, style, 0, 0, version);
							else
								sfputc(sp_body, '-');
							if (v)
								sfprintf(sp_body, " %-.*s", vl, v);
						}
					}
					else
					{
						if (f)
						{
							if (sp_body == sp_plus)
								sfputc(sp_body, '+');
							sfputc(sp_body, '-');
							sfputr(sp_body, bold(style, 1), -1);
							if (!sl)
								sfputc(sp_body, f);
							else
								sfwrite(sp_body, s, sl);
							sfputr(sp_body, bold(style, 0), -1);
							if (w)
							{
								sfputc(sp_body, ',');
								sfputc(sp_body, ' ');
							}
						}
						else if ((flags & OPT_functions) && re > rb)
						{
							sfwrite(sp_body, rb, re - rb);
							sfputc(sp_body, ' ');
						}
						if (w)
						{
							if (prefix > 0)
							{
								sfputc(sp_body, '-');
								if (prefix > 1)
									sfputc(sp_body, '-');
							}
							label(sp_body, 0, w, -1, style, 0, 1, version);
						}
						if (y)
						{
							if (a & OPT_optional)
								sfputc(sp_body, '[');
							else if (!w)
								sfputc(sp_body, ' ');
							if (w)
								sfputc(sp_body, prefix == 1 ? ' ' : '=');
							label(sp_body, 0, y, -1, style, 1, 0, version);
							if (a & OPT_optional)
								sfputc(sp_body, ']');
						}
					}
					if (style >= STYLE_match)
					{
						if (d)
							text(sp_body, d, style, 0, 3, sp_info, version);
						if (u)
							text(sp_body, u, style, 0, 3, sp_info, version);
						if ((a & OPT_invert) && w && (d || u))
						{
							u = skip(w, ':', '?', 0, 1, 0, 0, version);
							if (f)
								sfprintf(sp_info, " %s; -\b%c\b %s --\bno%-.*s\b.", ERROR_translate(ast.id, "On by default"), f, ERROR_translate(ast.id, "means"), u - w, w);
							else
								sfprintf(sp_info, " %s --\bno%-.*s\b %s.", ERROR_translate(ast.id, "On by default; use"), u - w, w, ERROR_translate(ast.id, "to turn off"));
							text(sp_body, sfstruse(sp_info), style, 0, 0, sp_info, version);
						}
						if (*p == GO)
						{
							p = text(sp_body, p, style, 4, 0, sp_info, version);
							y = "+?";
						}
						else
							y = " ";
						if (a & OPT_optional)
						{
							sfprintf(sp_info, "%s%s", y, ERROR_translate(ast.id, "The option value may be omitted."));
							text(sp_body, sfstruse(sp_info), style, 4, 0, sp_info, version);
							y = " ";
						}
						if (v)
						{
							sfprintf(sp_info, "%s%s \b%-.*s\b.", y, ERROR_translate(ast.id, "The default value is"), vl, v);
							text(sp_body, sfstruse(sp_info), style, 4, 0, sp_info, version);
						}
					}
					else if (!mutex)
						sfputc(sp_body, ']');
				}
				if (*p == GO)
				{
					if (style >= STYLE_match)
						p = text(sp_body, p, style, 4, 0, sp_info, version);
					else
						p = skip(p + 1, 0, 0, 0, 0, 1, 1, version);
				}
			}
			else if (*p == GO)
				p = skip(p + 1, 0, 0, 0, 0, 1, 1, version);
		}
		psp = pop(psp);
		if (sp_misc)
		{
			p = sfstruse(sp_misc); 
			for (t = p; *t == '\t' || *t == '\n'; t++);
			if (*t)
			{
				item(sp_body, ERROR_translate(ast.id, "IMPLEMENTATION"), 0, style, version);
				sfputr(sp_body, p, -1);
			}
		}
	}
	if (style >= STYLE_usage)
	{
		opt_info.disc = dp;
		return opt_info.msg = sfstruse(mp);
	}
	if (sp_info)
	{
		sfclose(sp_info);
		sp_info = 0;
	}
	sp = sp_text;
	if (sfstrtell(sp))
		sfputc(sp, ']');
	if (style == STYLE_nroff)
	{
		sfprintf(sp, "\
.\\\" format with nroff|troff|groff -man\n\
.nr mI 0\n\
.de H1\n\
.if \\\\n(mI!=0 \\{\n\
.	nr mI 0\n\
.	RE\n\
.\\}\n\
.TP\n\
\\fB\\\\$1\\fP\n\
..\n\
.de H3\n\
.if \\\\n(mI=0 \\{\n\
.	nr mI 1\n\
.	RS\n\
.\\}\n\
.TP\n\
\\fB     \\\\$1\\fP\n\
..\n\
.de OP\n\
.if \\\\n(mI!=0 \\{\n\
.	nr mI 0\n\
.	RE\n\
.\\}\n\
.ie !'\\\\$1'-' \\{\n\
.	ds mO \\\\fB\\\\-\\\\$1\\\\fP\n\
.	ds mS ,\\\\0\n\
.\\}\n\
.el \\{\n\
.	ds mO \\\\&\n\
.	ds mS \\\\&\n\
.\\}\n\
.ie '\\\\$2'-' \\{\n\
.	if !'\\\\$4'-' .as mO \\\\0\\\\fI\\\\$4\\\\fP\n\
.\\}\n\
.el \\{\n\
.	as mO \\\\*(mS\\\\fB\\\\-\\\\-\\\\$2\\\\fP\n\
.	if !'\\\\$4'-' .as mO =\\\\fI\\\\$4\\\\fP\n\
.\\}\n\
.TP\n\
\\\\*(mO\n\
..\n\
.de FN\n\
.if \\\\n(mI!=0 \\{\n\
.	nr mI 0\n\
.	RE\n\
.\\}\n\
.TP\n\
\\\\$1 \\\\$2\n\
..\n\
.TH %s %d\n\
", error_info.id, section);
	}
	if (style == STYLE_match)
	{
		if (!match)
		{
			style = 0;
			if (hp = (Help_t*)strpsearch(styles, elementsof(styles), sizeof(styles[0]), what, NiL))
			{
				sfprintf(sp, "[--??%s] [--help=?%s]\nOPTIONS\n\t--??%s\t\t\t\t%s", hp->match, hp->match, hp->match, hp->text);
				p = sfstruse(sp);
				goto format;
			}
			s = (char*)unknown;
			goto nope;
		}
		else if (match < 0)
			x = 0;
	}
	if (sp_plus)
	{
		if (sfstrtell(sp_plus))
		{
			if (sfstrtell(sp))
				sfputc(sp, ' ');
			sfputr(sp, sfstruse(sp_plus), ']');
		}
		sfclose(sp_plus);
	}
	if (style >= STYLE_man)
	{
		if (sp_head)
		{
			for (t = sfstruse(sp_head); *t == '\n'; t++);
			sfputr(sp, t, '\n');
			sfclose(sp_head);
			sp_head = 0;
		}
		item(sp, ERROR_translate(ast.id, "SYNOPSIS"), 0, style, version);
	}
	if (x)
	{
		for (t = x + xl; t > x && (*(t - 1) == '\n' || *(t - 1) == '\r'); t--);
		xl = t - x;
		if (style >= STYLE_match)
		{
			args(sp, x, xl, flags, style, version);
			x = 0;
		}
	}
	if (sp_body)
	{
		if (sfstrtell(sp_body))
		{
			if (style < STYLE_match && sfstrtell(sp))
				sfputc(sp, ' ');
			sfputr(sp, sfstruse(sp_body), -1);
		}
		sfclose(sp_body);
	}
	if (x)
		args(sp, x, xl, flags, style, version);
	if (sp_misc)
		sfclose(sp_misc);
	p = sfstruse(sp);
 format:
	name = error_info.id ? error_info.id : "command";
	m = strlen(name) + 1;
	if (!opt_info.width)
	{
		astwinsize(1, NiL, &opt_info.width);
		if (opt_info.width < 20)
			opt_info.width = OPT_WIDTH;
	}
	if (!(opt_info.flags & OPT_preformat))
	{
		if (style >= STYLE_man || match < 0)
		{
			sfputc(mp, '\f');
			ts = 0;
		}
		else
			ts = OPT_USAGE + m;
		if (style == STYLE_html)
		{
			sfprintf(mp, "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML//EN\">\n<HTML>\n<HEAD>\n<META NAME=\"generator\" CONTENT=\"optget (AT&T Labs Research) 1999-08-11\">\n<TITLE>%s man document</TITLE>\n</HEAD>\n<BODY bgcolor='#ffffff'>\n", name);
			sfprintf(mp, "<H4><TABLE WIDTH=100%%><TR><TH ALIGN=LEFT>&nbsp;%s&nbsp;(&nbsp;%d&nbsp;)&nbsp;<TH ALIGN=CENTER><A HREF=\"\" TITLE=\"Index\">%s</A><TH ALIGN=RIGHT>%s&nbsp;(&nbsp;%d&nbsp;)</TR></TABLE></H4>\n<HR>\n", name, section, heading[section % 10], name, section);
			sfprintf(mp, "<DL COMPACT>\n<DT>");
			co = 2;
			*(pt = ptstk) = 0;
		}
		else
			co = 0;
		if ((rm = opt_info.width - ts - 1) < OPT_MARGIN)
			rm = OPT_MARGIN;
		ip = indent;
		ip->stop = (ip+1)->stop = style >= STYLE_html ? 0 : 2;
		tp = 0;
		n = 0;
		head = 1;
		while (*p == '\n')
			p++;
		while (c = *p++)
		{
			if (c == '\n')
			{
				ip = indent;
				n = 0;
				tp = 0;
				sfputc(mp, '\n');
				co = 0;
				rm = opt_info.width - 1;
				ts = ip->stop;
				if (*p == '\n')
				{
					while (*++p == '\n');
					if ((style == STYLE_man || style == STYLE_html) && (!head || *p != ' ' && *p != '\t'))
					{
						if (style == STYLE_man)
							p--;
						else
							sfprintf(mp, "<P>\n");
					}
				}
				head = *p != ' ' && *p != '\t';
				if (style == STYLE_html && (*p != '<' || !strneq(p, "<BR>", 4) && !strneq(p, "<P>", 3)))
				{
					y = p;
					while (*p == '\t')
						p++;
					if (*p == '\n')
						continue;
					j = p - y;
					if (j > *pt)
					{
						if (pt > ptstk)
							sfprintf(mp, "<DL COMPACT>\n");
						*++pt = j;
						sfprintf(mp, "<DL COMPACT>\n");
					}
					else while (j < *pt)
					{
						if (--pt > ptstk)
							sfprintf(mp, "</DL>\n");
						sfprintf(mp, "</DL>\n");
					}
					co += sfprintf(mp, "<DT>");
				}
			}
			else if (c == '\t')
			{
				if (style == STYLE_html)
				{
					while (*p == '\t')
						p++;
					if (*p != '\n')
						co += sfprintf(mp, "<DD>");
				}
				else
				{
					if ((ip+1)->stop)
					{
						do
						{
							ip++;
							if (*p != '\t')
								break;
							p++;
						} while ((ip+1)->stop);
						if (*p == '\n')
							continue;
						ts = ip->stop;
						if (co >= ts)
						{
							sfputc(mp, '\n');
							co = 0;
							rm = opt_info.width - 1;
							ts = ip->stop;
						}
					}
					while (co < ts)
					{
						sfputc(mp, ' ');
						co++;
					}
				}
			}
			else
			{
				if (c == ' ' && !n)
				{
					if (co >= rm)
						tp = 0;
					else
					{
						tp = sfstrtell(mp);
						pp = p;
					}
				}
				else if (style == STYLE_html)
				{
					if (c == '<')
					{
						if (strneq(p, "NOBR>", 5))
							n++;
						else if (n && strneq(p, "/NOBR>", 6) && !--n)
						{
							for (y = p += 6; (c = *p) && c != ' ' && c != '\t' && c != '\n' && c != '<'; p++)
								if (c == '[')
									sfputr(mp, "&#0091;", -1);
								else if (c == ']')
									sfputr(mp, "&#0093;", -1);
								else
									sfputc(mp, c);
							sfwrite(mp, "</NOBR", 6);
							c = '>';
							tp = 0;
							co += p - y + 6;
						}
					}
					else if (c == '>' && !n)
					{
						for (y = --p; (c = *p) && c != ' ' && c != '\t' && c != '\n' && c != '<'; p++)
							if (c == '[')
								sfputr(mp, "&#0091;", -1);
							else if (c == ']')
								sfputr(mp, "&#0093;", -1);
							else
								sfputc(mp, c);
						c = *sfstrrel(mp, -1);
						if (p > y + 1)
						{
							tp = 0;
							co += p - y - 1;
						}
						if (co >= rm)
							tp = 0;
						else
						{
							tp = sfstrtell(mp);
							pp = p;
						}
					}
					else if (c == '[')
					{
						sfputr(mp, "&#0091", -1);
						c = ';';
					}
					else if (c == ']')
					{
						sfputr(mp, "&#0093", -1);
						c = ';';
					}
					else if (c == 'h')
					{
						y = p;
						if (*y++ == 't' && *y++ == 't' && *y++ == 'p' && (*y == ':' || *y++ == 's' && *y == ':') && *y++ == ':' && *y++ == '/' && *y++ == '/')
						{
							while (isalnum(*y) || *y == '_' || *y == '/' || *y == '-' || *y == '.')
								y++;
							if (*y == '?')
								while (isalnum(*y) || *y == '_' || *y == '/' || *y == '-' || *y == '.' || *y == '?' || *y == '=' || *y == '%' || *y == '&' || *y == ';' || *y == '#')
									y++;
							if (*(y - 1) == '.')
								y--;
							p--;
							sfprintf(mp, "<A HREF=\"%-.*s\">%-.*s</A", y - p, p, y - p, p);
							p = y;
							c = '>';
						}
					}
					else if (c == 'C')
					{
						y = p;
						if (*y++ == 'o' && *y++ == 'p' && *y++ == 'y' && *y++ == 'r' && *y++ == 'i' && *y++ == 'g' && *y++ == 'h' && *y++ == 't' && *y++ == ' ' && *y++ == '(' && (*y++ == 'c' || *(y - 1) == 'C') && *y++ == ')')
						{
							sfputr(mp, "Copyright &copy", -1);
							p = y;
							c = ';';
						}
					}
				}
				else if (c == ']')
				{
					if (n)
						n--;
				}
				else if (c == '[')
					n++;
				if (c == CC_esc)
				{
					sfputc(mp, c);
					do
					{
						if (!(c = *p++))
						{
							p--;
							break;
						}
						sfputc(mp, c);
					} while (c < 'a' || c > 'z');
				}
				else if (co++ >= rm && !n)
				{
					if (tp)
					{
						if (*sfstrset(mp, tp) != ' ')
							sfstrrel(mp, 1);
						tp = 0;
						p = pp;
						n = 0;
					}
					else if (c != ' ' && c != '\n')
						sfputc(mp, c);
					if (*p == ' ')
						p++;
					if (*p != '\n')
					{
						sfputc(mp, '\n');
						for (co = 0; co < ts; co++)
							sfputc(mp, ' ');
						rm = opt_info.width - 1;
					}
				}
				else
					sfputc(mp, c);
			}
		}
		for (d = sfstrbase(mp), t = sfstrrel(mp, 0); t > d && ((c = *(t - 1)) == '\n' || c == '\r' || c == ' ' || c == '\t'); t--);
		sfstrset(mp, t - d);
		if (style == STYLE_html)
		{
			while (pt > ptstk)
			{
				if (--pt > ptstk)
					sfprintf(mp, "\n</DL>");
				sfprintf(mp, "\n</DL>");
			}
			sfprintf(mp, "</DL>\n</BODY>\n</HTML>");
		}
	}
	else
		sfputr(mp, p, 0);
	if (sp)
		sfclose(sp);
	opt_info.disc = dp;
	return opt_info.msg = sfstruse(mp);
 nospace:
	s = "[* out of space *]";
 nope:
	if (psp)
		pop(psp);
	if (sp_text)
		sfclose(sp_text);
	if (sp_plus)
		sfclose(sp_plus);
	if (sp_info)
		sfclose(sp_info);
	if (sp_head)
		sfclose(sp_head);
	if (sp_body)
		sfclose(sp_body);
	if (sp_misc)
		sfclose(sp_misc);
	opt_info.disc = dp;
	return s;
}

/*
 * compatibility wrapper to opthelp()
 */

char*
optusage(const char* opts)
{
	return opthelp(opts, NiL);
}

/*
 * point opt_info.arg to an error/info message for opt_info.name
 * p points to opts location for opt_info.name
 * optget() return value is returned
 */

static int
opterror(register char* p, int version)
{
	register Sfio_t*	mp;
	register char*		s;

	if (opt_info.num != LONG_MIN)
		opt_info.num = 0;
	if (!p || !(mp = opt_info.mp) && !(mp = opt_info.mp = sfstropen()))
		opt_info.arg = "[* out of space *]";
	else
	{
		s = *p == '-' ? p : opt_info.name;
		if (*p == '!')
		{
			while (*s == '-')
				sfputc(mp, *s++);
			sfputc(mp, 'n');
			sfputc(mp, 'o');
		}
		sfputr(mp, s, ':');
		sfputc(mp, ' ');
		if (*p == '#' || *p == ':')
		{
			if (*p == '#')
			{
				s = ERROR_translate(ast.id, "numeric");
				sfputr(mp, s, ' ');
			}
			if (*(p = next(p + 1, version)) == '[')
			{
				p = skip(s = p + 1, ':', '?', 0, 1, 0, 0, version);
				sfwrite(mp, s, p - s);
				sfputc(mp, ' ');
			}
			p = opt_info.name[2] ? ERROR_dictionary("value expected") : ERROR_dictionary("argument expected");
		}
		else if (*p == '=' || *p == '!')
			p = ERROR_dictionary("value not expected");
		else if (*p == '?')
			p = *(p + 1) == '?' ? ERROR_dictionary("optget: option not supported") : ERROR_dictionary("ambiguous option");
		else if (*p == '+')
			p = ERROR_dictionary("section not found");
		else
		{
			if (opt_info.option[0] != '?' && opt_info.option[0] != '-' || opt_info.option[1] != '?' && opt_info.option[1] != '-' || opt_info.option[1] != '?')
				opt_info.option[0] = 0;
			p = ERROR_dictionary("unknown option");
		}
		p = ERROR_translate(ast.id, p);
		sfputr(mp, p, -1);
		opt_info.arg = sfstruse(mp);
	}
	return ':';
}

/*
 * argv:	command line argv where argv[0] is command name
 *
 * opts:	option control string
 *
 *	'[' [flag][=][index][:<long-name>[|<alias-name>...]['?'description]] ']'
 *			long option name, index, description; -index returned
 *	':'		option takes string arg
 *	'#'		option takes numeric arg (concat option may follow)
 *	'?'		(option) following options not in usage
 *			(following # or :) optional arg
 *	'[' '[' ... ] ... '[' ... ']' ']'
 *			mutually exclusive option grouping
 *	'[' name [:attr]* [?description] ']'	
 *			(following # or :) optional option arg description
 *	'\n'[' '|'\t']*	ignored for legibility
 *	' ' ...		optional argument(s) description (to end of string)
 *			or after blank line
 *	']]'		literal ']' within '[' ... ']'
 *
 * return:
 *	0		no more options
 *	'?'		usage: opt_info.arg points to message sans
 *			`Usage: command '
 *	':'		error: opt_info.arg points to message sans `command: '
 *
 * '-' '+' '?' ':' '#' '[' ']' ' '
 *			invalid option chars
 *
 * -- terminates option list and returns 0
 *
 * + as first opts char makes + equivalent to -
 *
 * if any # option is specified then numeric options (e.g., -123)
 * are associated with the leftmost # option in opts
 *
 * usage info in placed opt_info.arg when '?' returned
 * see help_text[] (--???) for more info
 */

int
optget(register char** argv, const char* oopts)
{
	register int	c;
	register char*	s;
	char*		a;
	char*		e;
	char*		f;
	char*		v;
	char*		w;
	char*		b;
	char*		numopt;
	char*		opts;
	int		n;
	int		m;
	int		k;
	int		j;
	int		x;
	int		no;
	int		nov;
	int		num;
	int		numchr;
	int		prefix;
	int		version;
	Help_t*		hp;
	Push_t*		psp;
	Push_t*		tsp;

	opt_info.pindex = opt_info.index;
	opt_info.poffset = opt_info.offset;
	if (!opt_info.index)
	{
		opt_info.index = 1;
		opt_info.offset = 0;
		opt_info.plus = 0;
		if (opt_info.npass)
		{
			opt_info.npass = 0;
			opt_info.join = 0;
		}
	}
	if (!argv)
		n = opt_info.npass ? opt_info.npass : 1;
	else if ((n = opt_info.join - 1) < 0)
		n = 0;
	if (n >= opt_info.npass || opt_info.pass[n].oopts != (char*)oopts)
	{
		for (m = 0; m < opt_info.npass && opt_info.pass[m].oopts != (char*)oopts; m++);
		if (m < opt_info.npass)
			n = m;
		else
		{
			if (n >= elementsof(opt_info.pass))
				n = elementsof(opt_info.pass) - 1;
			init((char*)oopts, &opt_info.pass[n]);
			if (opt_info.npass <= n)
				opt_info.npass = n + 1;
		}
	}
	if (!argv)
		return 0;
	opts = opt_info.pass[n].opts;
	prefix = opt_info.pass[n].prefix;
	version = opt_info.pass[n].version;
 again:
	num = 1;
	w = v = 0;
	for (;;)
	{
		if (!opt_info.offset)
		{
			if (opt_info.index == 1)
			{
				opt_info.argv = argv;
				opt_info.style = STYLE_short;
			}
			if (!(s = argv[opt_info.index]))
				return 0;
			if (!prefix)
			{
				n = 2;
				if ((c = *s) != '-' && c != '+')
				{
					c = '-';
					n = 2;
				}
				else if (*++s == c)
				{
					if (!*++s)
					{
						opt_info.index++;
						return 0;
					}
				}
				else if (*s == '?')
					n = 1;
			}
			else
			{
				if ((c = *s++) != '-' && (c != '+' || !opt_info.plus && (*s < '0' || *s > '9' || !strchr(opts, '#'))))
					return 0;
				if (*s == c)
				{
					if (!*++s)
					{
						opt_info.index++;
						return 0;
					}
					n = 2;
				}
				else if (prefix == 1 && *s != '?')
					n = 2;
				else
					n = 1;
			}
			if (!*s)
				return 0;
			if (c == '+')
				opt_info.arg = 0;
			if (n == 2)
			{
				x = 0;
				opt_info.style = STYLE_long;
				opt_info.option[0] = opt_info.name[0] = opt_info.name[1] = c;
				w = &opt_info.name[prefix];
				if ((*s == 'n' || *s == 'N') && (*(s + 1) == 'o' || *(s + 1) == 'O') && *(s + 2) && *(s + 2) != '=')
					no = *(s + 2) == '-' ? 3 : 2;
				else
					no = 0;
				for (c = *s; *s; s++)
				{
					if (*s == '=')
					{
						v = ++s;
						break;
					}
					if (w < &opt_info.name[elementsof(opt_info.name) - 1] && *s != ':' && *s != '|' && *s != '[' && *s != ']')
						*w++ = *s;
				}
				*w = 0;
				w = &opt_info.name[prefix];
				c = *w;
				opt_info.offset = 0;
				opt_info.index++;
				break;
			}
			opt_info.offset++;
		}
		if (c = argv[opt_info.index][opt_info.offset++])
		{
			opt_info.option[0] = opt_info.name[0] = argv[opt_info.index][0];
			opt_info.option[1] = opt_info.name[1] = c;
			opt_info.option[2] = opt_info.name[2] = 0;
			break;
		}
		opt_info.offset = 0;
		opt_info.index++;
	}
	if (c == '?')
	{
		if (w && !v && (*(w + 1) || !(v = argv[opt_info.index]) || !++opt_info.index))
			v = w + 1;
		opt_info.option[1] = c;
		opt_info.option[2] = 0;
		if (!w)
		{
			opt_info.name[1] = c;
			opt_info.name[2] = 0;
		}
		goto help;
	}
	numopt = 0;
	f = 0;
	s = opts;
	if (c == ':' || c == '#' || c == ' ' || c == '[' || c == ']')
	{
		if (c != *s)
			s = "";
	}
	else
	{
		psp = 0;
		for (;;)
		{
			if (!(*(s = next(s, version))))
			{
				if (!(tsp = psp))
					break;
				s = psp->cp;
				sfclose(psp->sp);
				psp = psp->next;
				free(tsp);
			}
			if (*s == '\f')
			{
				if (!(tsp = newof(0, Push_t, 1, 0)) || !(tsp->sp = sfstropen()))
				{
					if (tsp)
						free(tsp);
					psp = pop(psp);
					return opterror(NiL, version);
				}
				tsp->next = psp;
				psp = tsp;
				s = info(s + 1, &psp->cp, psp->sp);
				continue;
			}
			if (*s == '\n' || *s == ' ')
			{
				s = "";
				break;
			}
			message((-20, "optget: opt %-.16s", s));
			if (*s == c && !w)
				break;
			else if (*s == '[')
			{
				f = s = next(s + 1, version);
				k = *f;
				if (*s == '+' || *s == '-')
					/* ignore */;
				else if (*s == '[' || version < 1)
					continue;
				else if (w)
				{
					nov = no;
					if (*s != ':')
						s = skip(s, ':', '?', 0, 1, 0, 0, version);
					if (*s == ':')
					{
						for (;;)
						{
							n = m = 0;
							e = s + 1;
							while (*++s)
							{
								if (*s == '*')
									m = 1;
								else if (*s == *w)
									w++;
								else if (*s != '-')
									break;
							}
							if (!*w)
							{
								nov = 0;
								break;
							}
							if (n = no)
							{
								s = e - 1;
								w = &opt_info.name[prefix] + n;
								while (*++s)
								{
									if (*s == '*')
										m = 1;
									else if (*s == *w)
										w++;
									else if (*s != '-')
										break;
								}
								if (!*w)
									break;
							}
							if (*(s = skip(s, ':', '|', '?', 1, 0, 0, version)) != '|')
								break;
							w = &opt_info.name[prefix];
						}
						if (!*w)
						{
							if (!(n = (num = !n) && (m || *s == ':' || *s == '|' || *s == '?' || *s == ']')) && x)
							{
								psp = pop(psp);
								return opterror("?", version);
							}
							for (x = k; *(f + 1) == '|' && (j = *(f + 2)) && j != '!' && j != '=' && j != ':' && j != '?' && j != ']'; f += 2);
							if (*f == ':')
							{
								x = -1;
								opt_info.option[1] = '-';
								opt_info.option[2] = 0;
							}
							else if (*(f + 1) == ':' || *(f + 1) == '!' && *(f + 2) == ':')
							{
								opt_info.option[1] = x;
								opt_info.option[2] = 0;
							}
							else
							{
								a = f;
								if (*a == '=')
									a++;
								else
								{
									if (*(a + 1) == '!')
										a++;
									if (*(a + 1) == '=')
										a += 2;
								}
								x = -strtol(a, &b, 0);
								if ((b - a) > sizeof(opt_info.option) - 2)
									b = a + sizeof(opt_info.option) - 2;
								memcpy(&opt_info.option[1], a, b - a);
								opt_info.option[b - a + 1] = 0;
							}
							b = e;
							a = s = skip(s, 0, 0, 0, 1, 0, 0, version);
							if (n)
							{
								w = &opt_info.name[prefix];
								break;
							}
						}
						w = &opt_info.name[prefix];
					}
				}
				s = skip(s, 0, 0, 0, 1, 0, 1, version);
				if (*s == GO)
					s = skip(s + 1, 0, 0, 0, 0, 1, 1, version);
				m = 0;
				if (!w)
				{
					if (isdigit(*f) && isdigit(*(f + 1)))
						k = -1;
					if (c == k)
						m = 1;
					while (*(f + 1) == '|')
					{
						f += 2;
						if (!(j = *f))
						{
							m = 0;
							break;
						}
						else if (j == c)
							m = 1;
						else if (j == '!' || j == '=' || j == ':' || j == '?' || j == ']')
							break;
					}
				}
				if (m)
				{
					s--;
					if (*++f == '!')
					{
						f++;
						num = 0;
					}
					if (*f == '=')
					{
						c = -strtol(++f, &b, 0);
						if ((b - f) > sizeof(opt_info.option) - 2)
							b = f + sizeof(opt_info.option) - 2;
						memcpy(&opt_info.option[1], f, b - f);
						opt_info.option[b - f + 1] = 0;
					}
					else
						c = k;
					break;
				}
				if (*s == '#')
				{
					if (!numopt && s > opts)
					{
						numopt = s - 1;
						numchr = k;
						if (*f == ':')
							numchr = -1;
						else if (*(f + 1) != ':' && *(f + 1) != '!' && *(f + 1) != ']')
						{
							a = f;
							if (*a == '=')
								a++;
							else
							{
								if (*(a + 1) == '!')
									a++;
								if (*(a + 1) == '=')
									a += 2;
							}
							numchr = -strtol(a, NiL, 0);
						}
					}
				}
				else if (*s != ':')
					continue;
			}
			else if (*s == ']')
			{
				s++;
				continue;
			}
			else if (*s == '#')
			{
				if (!numopt && s > opts)
					numchr = *(numopt = s - 1);
			}
			else if (*s != ':')
			{
				s++;
				continue;
			}
			message((-21, "optget: opt %-.16s", s));
			if (*++s == '?' || *s == *(s - 1))
				s++;
			if (*(s = next(s, version)) == '[')
			{
				s = skip(s + 1, 0, 0, 0, 1, 0, 1, version);
				if (*s == GO)
					s = skip(s + 1, 0, 0, 0, 0, 1, 1, version);
			}
		}
		psp = pop(psp);
		if (w && x)
		{
			s = skip(b, '|', '?', 0, 1, 0, 0, version);
			if (v && *(a + 1) != ':' && *(a + 1) != '#' && (*v == '0' || *v == '1') && !*(v + 1))
			{
				if (*v == '0')
					num = !num;
				v = 0;
			}
			if ((s - b) >= elementsof(opt_info.name))
				s = b + elementsof(opt_info.name) - 1;
			while (b < s)
				if ((*w = *b++) != '*')
					w++;
			*w = 0;
			if (!num && v)
				return opterror(no ? "!" : "=", version);
			w = &opt_info.name[prefix];
			c = x;
			s = a;
		}
	}
	if (!*s)
	{
		if (w)
		{
			if (hp = (Help_t*)strpsearch(styles, elementsof(styles), sizeof(styles[0]), w, NiL))
			{
				if (!v)
					v = (char*)hp->name;
				goto help;
			}
			if (!v)
			{
				v = opt_info.name;
				goto help;
			}
		}
		if (w || c < '0' || c > '9' || !numopt)
			return opterror("", version);
		s = numopt;
		c = opt_info.option[1] = numchr;
		opt_info.offset--;
	}
	opt_info.arg = 0;

	/*
	 * this is a ksh getopts workaround
	 */

	if (opt_info.num != LONG_MIN)
		opt_info.num = num;
	if (*++s == ':' || *s == '#')
	{
		if (w)
		{
			if (nov)
			{
				if (v)
					return opterror("!", version);
				opt_info.num = 0;
			}
			else
			{
				if (!v && *(s + 1) != '?' && (v = argv[opt_info.index]))
					opt_info.index++;
				if (!(opt_info.arg = v))
				{
					if (*(s + 1) != '?')
						return opterror(s, version);
				}
				else if (*s == '#')
				{
					opt_info.num = strton(opt_info.arg, &e, NiL, 0);
					if (e == opt_info.arg)
						return opterror(s, version);
				}
			}
			return c;
		}
		else if (*(opt_info.arg = &argv[opt_info.index++][opt_info.offset]))
		{
			if (*s == '#')
			{
				opt_info.num = strton(opt_info.arg, &e, NiL, 0);
				if (e == opt_info.arg)
				{
					if (*(s + 1) == '?')
					{
						opt_info.arg = 0;
						opt_info.index--;
						return c;
					}
					else
						c = opterror(s, version);
				}
				else if (*e)
				{
					opt_info.offset += e - opt_info.arg;
					opt_info.index--;
					return c;
				}
			}
		}
		else if (opt_info.arg = argv[opt_info.index])
		{
			opt_info.index++;
			if (*(s + 1) == '?' && (*opt_info.arg == '-' || opt_info.plus && *opt_info.arg == '+') && *(opt_info.arg + 1))
			{
				opt_info.index--;
				opt_info.arg = 0;
			}
			else if (*s == '#')
			{
				opt_info.num = strton(opt_info.arg, &e, NiL, 0);
				if (*e)
				{
					if (*(s + 1) == '?')
					{
						opt_info.arg = 0;
						opt_info.index--;
					}
					else
						c = opterror(s, version);
				}
			}
		}
		else if (*(s + 1) != '?')
			c = opterror(s, version);
		opt_info.offset = 0;
	}
	else if (w && v)
		return opterror("=", version);
	else
	{
		opt_info.num = num;
		if (!w && !argv[opt_info.index][opt_info.offset])
		{
			opt_info.offset = 0;
			opt_info.index++;
		}
	}
	return c;
 help:
	if (v && *v == '?' && *(v + 1) == '?' && *(v + 2))
	{
		s = v + 2;
		if ((s[0] == 'n' || s[0] == 'N') && (s[1] == 'o' || s[1] == 'O'))
		{
			s += 2;
			n = -1;
		}
		else
			n = 1;
		if (strpsearch(styles, elementsof(styles), sizeof(styles[0]), s, NiL))
		{
			opt_info.arg = sfprints("\fversion=%d", version);
			return '?';
		}
		if (streq(s, "ESC") || streq(s, "EMPHASIS"))
		{
			opt_info.emphasis = n;
			goto again;
		}
		if (streq(s, "PREFORMAT"))
		{
			opt_info.flags |= OPT_preformat;
			goto again;
		}
		if (streq(s, "TEST"))
		{
			opt_info.width = OPT_WIDTH;
			opt_info.emphasis = 1;
			goto again;
		}
		return opterror(v, version);
	}
	if ((opt_info.arg = opthelp(NiL, v)) == (char*)unknown)
		return opterror(v, version);
	return '?';
}

/*
 * parse long options sans leading -- from string and pass to optget()
 * syntax is
 *
 *	[no]name[[:]=['"{(]value[)}"']][, ]...
 *
 * \x escapes passed to chresc()
 * := negates opt_info.num
 *
 * return '#' for `label:', with opt_info.name==label
 * str[opt_info.offset]	next arg
 *
 *	optstr(s, 0)
 *		return '-' if arg, 0 otherwise
 *	optstr(0, opts)
 *		use previous parsed str
 */

int
optstr(const char* str, const char* opts)
{
	register char*		s = (char*)str;
	register Sfio_t*	mp;
	register int		c;
	register int		ql;
	register int		qr;
	register int		qc;
	int			v;
	char*			e;

	if (s)
	{
		if (!(mp = opt_info.strp) && !(mp = opt_info.strp = sfstropen()))
			return 0;
		if (opt_info.str != s)
			opt_info.str = s;
		else if (opt_info.index == 1)
			s += opt_info.offset;
		while (*s == ',' || *s == ' ' || *s == '\t' || *s == '\n' || *s == '\r')
			s++;
		if (!*s)
			return 0;
		sfputc(mp, '-');
		sfputc(mp, '-');
		while (*s && *s != ',' && *s != ' ' && *s != '\t' && *s != '\n' && *s != '\r' && *s != '=' && *s != ':')
			sfputc(mp, *s++);
		if (*s != ':')
			opt_info.colon = 0;
		else if (*++s != '=')
		{
			opt_info.index = 1;
			opt_info.offset = s - (char*)str;
			s = sfstruse(mp);
			e = opt_info.name;
			while (e < &opt_info.name[sizeof(opt_info.name)-1] && (*e++ = *s++));
			return '#';
		}
		else
			opt_info.colon = 1;
		if (*s == '=')
		{
			sfputc(mp, '=');
			ql = qr = 0;
			while (c = *++s)
			{
				if (c == '\\')
				{
					sfputc(mp, chresc(s, &e));
					s = e - 1;
				}
				else if (c == qr)
				{
					if (qr != ql)
						sfputc(mp, c);
					if (--qc <= 0)
						qr = ql = 0;
				}
				else if (c == ql)
				{
					sfputc(mp, c);
					qc++;
				}
				else if (qr)
					sfputc(mp, c);
				else if (c == ',' || c == ' ' || c == '\t' || c == '\n' || c == '\r')
					break;
				else if (c == '"' || c == '\'')
				{
					ql = qr = c;
					qc = 1;
				}
				else
				{
					sfputc(mp, c);
					if (c == GO)
					{
						ql = c;
						qr = OG;
						qc = 1;
					}
					else if (c == '(')
					{
						ql = c;
						qr = ')';
						qc = 1;
					}
				}
			}
		}
		opt_info.argv = opt_info.strv;
		opt_info.strv[0] = ERROR_translate(ast.id, "option");
		opt_info.strv[1] = sfstruse(mp);
		opt_info.strv[2] = 0;
		opt_info.offset = s - (char*)str;
	}
	if (opts)
	{
		if (!opt_info.strv[1])
			return 0;
		opt_info.index = 1;
		v = opt_info.offset;
		opt_info.offset = 0;
		c = optget(opt_info.strv, opts);
		opt_info.index = 1;
		opt_info.offset = v;
		if ((c == '?' || c == ':') && (opt_info.arg[0] == '-' && opt_info.arg[1] == '-'))
			opt_info.arg += 2;
		s = opt_info.name;
		if (*s++ == '-' && *s++ == '-')
		{
			e = opt_info.name;
			while (*e++ = *s++);
		}
		if (opt_info.colon)
			opt_info.num = -opt_info.num;
	}
	else
		c = '-';
	return c;
}
