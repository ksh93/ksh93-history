/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1997-2001 AT&T Corp.                *
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
*******************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Labs Research
 */

#include <ast.h>
#include <dlldefs.h>
#include <ctype.h>
#include <fts.h>

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
 * find and load library lib with optional version ver
 * and dlopen() flags
 */

extern void*
dllfind(const char* lib, const char* ver, int flags)
{
	register char*		s;
	register char*		t;
	register char*		b;
	register char*		e;
	register char**		sv;
	register char**		nv;
	char*			x;
	char*			a;
	char*			u;
	char*			p;
	char*			suf;
	int			dot;
	int			try;
	void*			dll;
	char			bas[64];
	char			gen[64];
	char			pat[64];
	char			spc[64];
	char			dir[64];
	char			tmp[PATH_MAX];
	char*			sib[4];
	char*			nam[4];

	/*
	 * lib==0 for special ops determined by ver
	 */

	if (!lib)
	{
		/*
		 * ver==0 to uncover next layer
		 */

		if (!ver)
			return dllnext(flags);

		/*
		 * ver!=0 for future ops
		 * just fail for now
		 */

		return 0;
	}

	/*
	 * qualified paths go right to dlopen()
	 */

	if (strchr(lib, '/') || strchr(lib, '\\'))
		return dlopen(lib, flags);

	/*
	 * invalid version qualifiers are ignored
	 */

	if (ver && (!*ver || !isdigit(*ver)))
		ver = 0;

	/*
	 * we need a sibling dir in PATH to search for dlls
	 * the confstr LIBPATH almost does it, except for sgi
	 * who really got carried away with changing the user
	 * view of the world in trying to handle binary
	 * compatibility between *three* different object formats
	 * this list will surely grow given all vendor's weakness
	 * for `enhancing' standard practice
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
	 * (and represented by "" here)
	 */

	dir[0] = 0;
	t = astconf("HOSTTYPE", NiL, NiL);
	if (strmatch(t, "sgi.*"))
	{
		if (strmatch(t, "sgi.mips3|sgi.*-n32"))
			s = "lib32";
		else if (strmatch(t, "sgi.mips[4-9]|sgi.*-64"))
			s = "lib64";
		else
			s = "lib";
		strcpy(dir, s);
	}
	else if (*(s = astconf("LIBPATH", NiL, NiL)))
	{
		for (t = s; *t && *t != ':' && *t != ','; t++);
		if (*t == ':')
		{
			dot = t - s;
			if ((dot + 1) < sizeof(dir))
			{
				memcpy(dir, s, dot);
				dir[dot] = 0;
			}
		}
	}
	x = 0;
	for (s = (char*)lib; *s; s++)
		if (*s == '.' || *s == '-')
		{
			if (strmatch(s + 1, "+([0-9.])"))
			{
				ver = (const char*)(s + 1);
				sfsprintf(bas, sizeof(bas), "%-.*s", s - (char*)lib, lib);
				lib = (const char*)bas;
			}
			else
				x = s;
			break;
		}
#if DEBUG
sfprintf(sfstderr, "dllfind: lib=%s ver=%s\n", lib, ver);
#endif
	nv = nam;
	sv = sib;
	*sv++ = dir;
	if (dir[0])
	{
		if (streq(dir, "bin"))
			dir[0] = 0;
		else if (!streq(dir, "lib"))
			*sv++ = "lib";
	}
	if (x)
	{
		*nv++ = (char*)lib;
		pat[0] = 0;
	}
	else
	{
		suf = astconf("LIBSUFFIX", NiL, NiL);
		if (streq(suf, ".dll"))
		{
			sfsprintf(gen, sizeof(gen), "%s%s", lib, suf);
			if (ver)
			{
				s = spc;
				t = (char*)lib;
				while (s < &spc[sizeof(spc)-1] && *t)
				for (; s < &spc[sizeof(spc)-1] && *ver; ver++)
					if (isdigit(*ver))
						*s++ = *ver;
				t = suf;
				while (s < &spc[sizeof(spc)-1] && *t)
					*s++ = *t++;
				*s = 0;
				*nv++ = spc;
			}
			*nv++ = gen;
			sfsprintf(pat, sizeof(pat), "%s+([0-9])%s", lib, suf);
		}
		else
		{
			sfsprintf(gen, sizeof(gen), "lib%s%s", lib, suf);
			if (ver)
			{
				sfsprintf(spc, sizeof(spc), "%s.%s", gen, ver);
				*nv++ = spc;
			}
			*nv++ = gen;
			sfsprintf(pat, sizeof(pat), "lib%s%s.+([0-9.])", lib, suf);
		}		
	}
	*nv = 0;
	*sv = 0;
	try = 0;
	dll = 0;
	x = &tmp[sizeof(tmp) - 1];
#if DEBUG
	for (nv = nam; *nv; nv++)
		sfprintf(sfstderr, "dllfind: nam[%d]=%s\n", nv - nam, *nv);
#endif

	/*
	 * now the search
	 */

	do
	{
		t = pathbin();
		while (s = t)
		{
			sv = sib;
			if (*s == ':')
			{
				t = s + 1;
				b = s;
			}
			else
			{
				if (t = strchr(s, ':'))
					b = t++;
				else
					b = s + strlen(s);
				a = b;
				if (*sv && **sv)
					while (b > s && *(b - 1) != '/')
						b--;
			}
			p = s;
			dot = b == s;
			do
			{
#if DEBUG
sfprintf(sfstderr, "dllfind: attempt sib[%d]=%s\n", sv - sib + 1, *sv);
#endif
				e = tmp;
				s = p;
				if (dot)
				{
					*e++ = '.';
					*e++ = '/';
				}
				while (s < b)
				{
					if (e >= x)
						goto next;
					*e++ = *s++;
				}
				if (e >= x)
					goto next;
				if (s = *sv)
				{
					for (u = e; *e = *s++; e++)
						if (e >= x)
							goto next;
#if DEBUG
sfprintf(sfstderr, "dllfind: access(%s)\n", tmp);
#endif
					if (access(tmp, X_OK))
					{
						if (*(sv + 1))
							continue;
						e = tmp + dot * 2;
						for (s = b; s < a && (*e = *s++); e++)
							if (e >= x)
								goto next;
					}
				}
				if (e >= x)
					goto next;
				*e++ = '/';
				for (nv = nam; *nv; nv++)
				{
					if (*nv != pat)
					{
						for (u = e, s = *nv; (*u = *s++) && u < x; u++);
#if DEBUG
sfprintf(sfstderr, "dllfind: access(%s) dot=%d nv=%s sv=%s\n", tmp, dot, *nv, *sv);
#endif
						if (!access(tmp, 0))
							return dlopen(tmp, flags);
					}
					else
					{
						/*UNDENT...*/

	FTS*	fts;
	FTSENT*	ent;

	if (e < x)
	{
		*e = '.';
		*(e + 1) = 0;
	}
	else
		*e = 0;
#if DEBUG
sfprintf(sfstderr, "dllfind: fts_open(%s) pat=%s\n", tmp, pat);
#endif
	if (fts = fts_open((char**)tmp, FTS_LOGICAL|FTS_NOPOSTORDER|FTS_ONEPATH, vercmp))
	{
		if (ent = fts_read(fts))
			for (ent = fts_children(fts, FTS_NOSTAT); ent; ent = ent->fts_link)
			{
#if DEBUG
sfprintf(sfstderr, "dllfind: fts_child=%s\n", ent->fts_name);
#endif
				if (strmatch(ent->fts_name, pat))
				{
					for (u = e, s = ent->fts_name; (*u = *s++) && u < x; u++);
#if DEBUG
sfprintf(sfstderr, "dllfind: attempt dlopen %s\n", tmp);
#endif
					try = 1;
					dll = dlopen(tmp, flags);
					break;
				}
			}
		fts_close(fts);
		if (try)
			return dll;
	}

						/*...INDENT*/
					}
				}
 next:
				;
			} while (*sv && *++sv);
		}
	} while (*(nv = nam) != pat && *(*nv++ = pat));
	return dlopen(lib, flags);
}
