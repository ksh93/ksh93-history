/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1997-2002 AT&T Corp.                *
*        and it may only be used by you under license from         *
*                       AT&T Corp. ("AT&T")                        *
*         A copy of the Source Code Agreement is available         *
*                at the AT&T Internet web site URL                 *
*                                                                  *
*       http://www.research.att.com/sw/license/ast-open.html       *
*                                                                  *
*    If you have copied or used this software without agreeing     *
*        to the terms of the license you are infringing on         *
*           the license and copyright and are violating            *
*               AT&T's intellectual property rights.               *
*                                                                  *
*            Information and Software Systems Research             *
*                        AT&T Labs Research                        *
*                         Florham Park NJ                          *
*                                                                  *
*               Glenn Fowler <gsf@research.att.com>                *
*                                                                  *
*******************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Labs Research
 */

#define _DLLINFO_PRIVATE_ \
	char*	sib[3]; \
	char	buf[64];

#define _DLLSCAN_PRIVATE_ \
	Dllent_t	entry; \
	Uniq_t*		uniq; \
	int		flags; \
	Vmalloc_t*	vm; \
	Dt_t*		dict; \
	Dtdisc_t	disc; \
	FTS*		fts; \
	FTSENT*		ent; \
	Sfio_t*		tmp; \
	char**		sb; \
	char**		sp; \
	char*		pb; \
	char*		pp; \
	char*		pe; \
	int		off; \
	int		prelen; \
	int		suflen; \
	char		nam[64]; \
	char		pat[64]; \
	char		buf[64];

#define DLL_MATCH_DONE		0x8000
#define DLL_MATCH_NAME		0x4000
#define DLL_MATCH_VERSION	0x2000

#define DIG			01
#define SEP			02

#include <ast.h>
#include <cdt.h>
#include <ctype.h>
#include <error.h>
#include <fts.h>
#include <sfstr.h>
#include <vmalloc.h>

typedef struct Uniq_s
{
	Dtlink_t	link;
	char		name[1];
} Uniq_t;

#include <dlldefs.h>

static char		bin[] = "bin";
static char		lib[] = "lib";

/*
 * we need a sibling dir in PATH to search for dlls
 * the confstr LIBPATH almost does it, except for sgi
 * who really got carried away trying to handle binary
 * compatibility between *three* different object formats
 * this list will surely grow given all vendor's weakness
 * for `enhancing' standard practice, de-facto or otherwise
 *
 *	HOSTTYPE	sibling
 *	----------	-------
 *	sgi.mips4	lib64
 *	sgi.mips3	lib32
 *	sgi.*		lib
 *
 * otherwise the first dir in LIBPATH which has the format
 *
 *	<sibling-dir>:<env-var>,...
 *
 * gives us the sibling, bin being the overall default
 */

Dllinfo_t*
dllinfo(void)
{
	register char*		s;
	register char*		t;
	register int		n;

	static Dllinfo_t	info;

	if (!info.sibling)
	{
		info.sibling = info.sib;
		t = astconf("HOSTTYPE", NiL, NiL);
		if (strmatch(t, "sgi.*"))
		{
			if (strmatch(t, "sgi.mips3|sgi.*-n32"))
				info.sibling[0] = "lib32";
			else if (strmatch(t, "sgi.mips[4-9]|sgi.*-64"))
				info.sibling[0] = "lib64";
			else
				info.sibling[0] = lib;
		}
		else if (*(s = astconf("LIBPATH", NiL, NiL)))
		{
			for (t = s; *t && *t != ':' && *t != ','; t++);
			if (*t == ':')
			{
				n = t - s;
				if ((n + 1) < sizeof(info.buf))
				{
					info.sibling[0] = info.buf;
					memcpy(info.buf, s, n);
				}
			}
		}
		if (!info.sibling[0] || streq(info.sibling[0], bin))
			info.sibling[0] = bin;
		if (!streq(info.sibling[0], lib))
			info.sibling[1] = lib;
		info.prefix = astconf("LIBPREFIX", NiL, NiL);
		info.suffix = astconf("LIBSUFFIX", NiL, NiL);
		if (streq(info.suffix, ".dll"))
			info.flags |= DLL_INFO_PREVER;
		else
			info.flags |= DLL_INFO_DOTVER;
	}
	return &info;
}

/*
 * fts version sort order
 * higher versions appear first
 */

static int
vercmp(FTSENT* const* ap, FTSENT* const* bp)
{
	register unsigned char*	a = (unsigned char*)(*ap)->fts_name;
	register unsigned char*	b = (unsigned char*)(*bp)->fts_name;
	register int		n;
	register int		m;
	char*			e;

	for (;;)
	{
		if (isdigit(*a) && isdigit(*b))
		{
			m = strtol((char*)a, &e, 10);
			a = (unsigned char*)e;
			n = strtol((char*)b, &e, 10);
			b = (unsigned char*)e;
			if (n -= m)
				return n;
		}
		if (n = *a - *b)
			return n;
		if (!*a++)
			return *b ? 0 : -1;
		if (!*b++)
			return 1;
	}
	/*NOTREACHED*/
}

/*
 * open a scan stream
 */

Dllscan_t*
dllsopen(const char* lib, const char* name, const char* version)
{
	register char*	s;
	register char*	t;
	Dllscan_t*	scan;
	Dllinfo_t*	info;
	Vmalloc_t*	vm;
	char		buf[32];

	if (!(vm = vmopen(Vmdcheap, Vmlast, 0)))
		return 0;
	if (!(scan = vmnewof(vm, 0, Dllscan_t, 1, 0)) || !(scan->tmp = sfstropen()))
	{
		vmclose(vm);
		return 0;
	}
	scan->vm = vm;
	info = dllinfo();
	scan->flags = info->flags;
	if (!lib)
		lib = "";
	if (!name)
	{
		name = (const char*)"?*";
		scan->flags |= DLL_MATCH_NAME;
	}
	if (!version)
	{
		scan->flags |= DLL_MATCH_VERSION;
		sfsprintf(scan->nam, sizeof(scan->nam), "%s%s%s%s", info->prefix, lib, name, info->suffix);
	}
	else if (scan->flags & DLL_INFO_PREVER)
	{
		sfprintf(scan->tmp, "%s%s%s", info->prefix, lib, name);
		for (s = (char*)version; *s; s++)
			if (isdigit(*s))
				sfputc(scan->tmp, *s);
		sfprintf(scan->tmp, "%s", info->suffix);
		sfsprintf(scan->nam, sizeof(scan->nam), "%s", sfstruse(scan->tmp));
	}
	else
	{
		scan->flags |= DLL_MATCH_VERSION;
		sfsprintf(scan->nam, sizeof(scan->nam), "%s%s%s%s.%s", info->prefix, lib, name, info->suffix, version);
	}
	if (scan->flags & (DLL_MATCH_NAME|DLL_MATCH_VERSION))
	{
		if (scan->flags & DLL_INFO_PREVER)
		{
			if (!version)
				version = "*([0-9_])";
			else
			{
				t = buf;
				for (s = (char*)version; *s; s++)
					if (isdigit(*s) && t < &buf[sizeof(buf)-1])
						*t++ = *s;
				*t = 0;
				version = (const char*)buf;
			}
			sfsprintf(scan->pat, sizeof(scan->pat), "%s%s%s%s%s", info->prefix, lib, name, version, info->suffix);
		}
		else if (version)
			sfsprintf(scan->pat, sizeof(scan->pat), "%s%s%s@(%s([-.])%s%s|%s.%s)", info->prefix, lib, name, strchr(version, '.') ? "@" : "?", version, info->suffix, info->suffix, version);
		else
		{
			version = "*([0-9.])";
			sfsprintf(scan->pat, sizeof(scan->pat), "%s%s%s@(?([-.])%s%s|%s%s)", info->prefix, lib, name, version, info->suffix, info->suffix, version);
		}
	}
	scan->sb = scan->sp = info->sibling;
	scan->prelen = strlen(info->prefix) + strlen(lib);
	scan->suflen = strlen(info->suffix);
	return scan;
}

/*
 * close a scan stream
 */

int
dllsclose(Dllscan_t* scan)
{
	if (!scan)
		return -1;
	if (scan->fts)
		fts_close(scan->fts);
	if (scan->dict)
		dtclose(scan->dict);
	if (scan->tmp)
		sfclose(scan->tmp);
	if (scan->vm)
		vmclose(scan->vm);
	return 0;
}

/*
 * return the next scan stream entry
 */

Dllent_t*
dllsread(register Dllscan_t* scan)
{
	register char*		p;
	register char*		b;
	register char*		t;
	register Uniq_t*	u;
	register int		n;

	if (scan->flags & DLL_MATCH_DONE)
		return 0;
 again:
	do
	{
		while (!scan->ent || !(scan->ent = scan->ent->fts_link))
		{
			if (scan->fts)
			{
				fts_close(scan->fts);
				scan->fts = 0;
			}
			if (!scan->pb)
				scan->pb = pathbin();
			else if (!*scan->sp)
			{
				scan->sp = scan->sb;
				if (!*scan->pe++)
					return 0;
				scan->pb = scan->pe;
			}
			for (p = scan->pp = scan->pb; *p && *p != ':'; p++)
				if (*p == '/')
					scan->pp = p;
			scan->pe = p;
			if (*scan->sp == bin)
				scan->off = sfprintf(scan->tmp, "%-.*s", scan->pe - scan->pb, scan->pb);
			else
				scan->off = sfprintf(scan->tmp, "%-.*s/%s", scan->pp - scan->pb, scan->pb, *scan->sp);
			scan->sp++;
			if (!(scan->flags & DLL_MATCH_NAME))
			{
				sfprintf(scan->tmp, "/%s", scan->nam);
				p = sfstruse(scan->tmp);
				if (!access(p, R_OK))
				{
					b = scan->nam;
					goto found;
				}
				if (errno != ENOENT)
					continue;
			}
			if (scan->flags & (DLL_MATCH_NAME|DLL_MATCH_VERSION))
			{
				sfstrset(scan->tmp, scan->off);
				if ((scan->fts = fts_open((char**)sfstruse(scan->tmp), FTS_LOGICAL|FTS_NOPOSTORDER|FTS_ONEPATH, vercmp)) && (scan->ent = fts_read(scan->fts)) && (scan->ent = fts_children(scan->fts, FTS_NOSTAT)))
					break;
			}
		}
	} while (!strmatch(scan->ent->fts_name, scan->pat));
	b = scan->ent->fts_name;
	sfstrset(scan->tmp, scan->off);
	sfprintf(scan->tmp, "/%s", b);
	p = sfstruse(scan->tmp);
 found:
	b = scan->buf + sfsprintf(scan->buf, sizeof(scan->buf), "%s", b + scan->prelen);
	n = 0;
	if (!(scan->flags & DLL_INFO_PREVER))
		while (b > scan->buf)
		{
			if (isdigit(*(b - 1)))
				n |= DIG;
			else if (*(b - 1) == '.')
				n |= SEP;
			else
				break;
			b--;
		}
	b -= scan->suflen;
	if (b > (scan->buf + 2) && (*(b - 1) == 'g' || *(b - 1) == 'O') && *(b - 2) == '-')
		b -= 2;
	if (!n)
	{
		for (t = b; t > scan->buf; t--)
			if (isdigit(*(t - 1)))
				n |= DIG;
			else if (*(t - 1) == '.')
				n |= SEP;
			else if (*(t - 1) == '-')
			{
				t--;
				n |= SEP;
				break;
			}
			else if (*(t - 1) == '_' && isdigit(*t))
				/* ignore */;
			else
				break;
		if ((n & DIG) && ((n & SEP) || (scan->flags & DLL_INFO_PREVER) && ((b - t) != 2 || !(t[0] == '1' && t[1] == '6' || t[0] == '3' && t[1] == '2'))))
			b = t;
	}
	*b = 0;
	if (!*(b = scan->buf))
		goto again;
	if (scan->uniq)
	{
		if (!scan->dict)
		{
			scan->disc.key = offsetof(Uniq_t, name);
			scan->disc.size = 0;
			scan->disc.link = offsetof(Uniq_t, link);
			if (!(scan->dict = dtopen(&scan->disc, Dthash)))
				return 0;
			dtinsert(scan->dict, scan->uniq);
		}
		if (dtmatch(scan->dict, b))
			goto again;
		if (!(u = vmnewof(scan->vm, 0, Uniq_t, 1, strlen(b))))
			return 0;
		strcpy(u->name, b);
		dtinsert(scan->dict, u);
	}
	else if (!(scan->flags & DLL_MATCH_NAME))
		scan->flags |= DLL_MATCH_DONE;
	else if (!(scan->uniq = vmnewof(scan->vm, 0, Uniq_t, 1, strlen(b))))
		return 0;
	else
		strcpy(scan->uniq->name, b);
	scan->entry.name = b;
	scan->entry.path = p;
	return &scan->entry;
}
