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
 * string interface to confstr(),pathconf(),sysconf()
 * extended to allow some features to be set
 */

static const char id[] = "\n@(#)$Id: getconf (AT&T Labs Research) 2001-04-20 $\0\n";

#include "univlib.h"

#include <ast.h>
#include <error.h>
#include <stk.h>
#include <fs3d.h>
#include <ctype.h>

#include "conftab.h"
#include "FEATURE/libpath"

#define OP_conformance		1
#define OP_fs_3d		2
#define OP_hosttype		3
#define OP_libpath		4
#define OP_libsuffix		5
#define OP_path_attributes	6
#define OP_path_resolve		7
#define OP_universe		8

#define CONF_ERROR	(CONF_USER<<0)
#define CONF_READONLY	(CONF_USER<<1)

#define INITIALIZE()	do{if(!state.data)synthesize(NiL,NiL,NiL);}while(0)

#define MAXVAL		256

#if MAXVAL <= UNIV_SIZE
#undef	MAXVAL
#define	MAXVAL		(UNIV_SIZE+1)
#endif

#ifndef _UNIV_DEFAULT
#define _UNIV_DEFAULT	"att"
#endif

typedef struct Feature_s
{
	struct Feature_s*next;
	const char*	name;
	char		value[MAXVAL];
	char		strict[MAXVAL];
	short		length;
	short		standard;
	short		flags;
	short		op;
} Feature_t;

typedef struct
{
	Conf_t*		conf;
	const char*	name;
	const char*	error;
	short		flags;
	short		call;
	short		standard;
	short		section;
} Lookup_t;

static Feature_t	dynamic[] =
{
	{
		&dynamic[1],
		"CONFORMANCE",
		"ast",
		"standard",
		11,
		CONF_AST,
		0,
		OP_conformance
	},
	{
		&dynamic[2],
		"FS_3D",
		"",
		"0",
		5,
		CONF_AST,
		0,
		OP_fs_3d
	},
	{
		&dynamic[3],
		"HOSTTYPE",
		HOSTTYPE,
		"",
		8,
		CONF_AST,
		CONF_READONLY,
		OP_hosttype
	},
	{
		&dynamic[4],
		"LIBPATH",
#ifdef CONF_LIBPATH
		CONF_LIBPATH,
#else
		"",
#endif
		"",
		7,
		CONF_AST,
		0,
		OP_libpath
	},
	{
		&dynamic[5],
		"LIBSUFFIX",
#ifdef CONF_LIBSUFFIX
		CONF_LIBSUFFIX,
#else
		"lib",
#endif
		"",
		7,
		CONF_AST,
		0,
		OP_libsuffix
	},
	{
		&dynamic[6],
		"PATH_ATTRIBUTES",
#if _UWIN
		"c",
#else
		"",
#endif
		"",
		15,
		CONF_AST,
		CONF_READONLY,
		OP_path_attributes
	},
	{
		&dynamic[7],
		"PATH_RESOLVE",
		"",
		"metaphysical",
		12,
		CONF_AST,
		0,
		OP_path_resolve
	},
	{
		0,
		"UNIVERSE",
		"",
		"att",
		8,
		CONF_AST,
		0,
		OP_universe
	},
	{
		0
	}
};

typedef struct
{

	const char*	id;
	const char*	name;
	Feature_t*	features;

	/* default initialization from here down */

	int		prefix;
	int		synthesizing;

	char*		data;
	char*		last;

	Ast_confdisc_f	notify;

} State_t;

static State_t	state = { "getconf", "_AST_FEATURES", dynamic };

static char*	feature(const char*, const char*, const char*, Ast_conferror_f);

/*
 * synthesize state for fp
 * fp==0 initializes from getenv(state.name)
 * value==0 just does lookup
 * otherwise state is set to value
 */

static char*
synthesize(register Feature_t* fp, const char* path, const char* value)
{
	register char*		s;
	register char*		d;
	register char*		v;
	register int		n;

	if (state.synthesizing)
		return "";
	if (!state.data)
	{
		char*		se;
		char*		de;
		char*		ve;

		state.prefix = strlen(state.name) + 1;
		n = state.prefix + 3 * MAXVAL;
		if (s = getenv(state.name))
			n += strlen(s);
		n = roundof(n, 32);
		if (!(state.data = newof(0, char, n, 0)))
			return 0;
		strcpy(state.data, state.name);
		state.data += state.prefix - 1;
		*state.data++ = '=';
		if (s)
			strcpy(state.data, s);
		ve = state.data;
		state.synthesizing = 1;
		for (;;)
		{
			for (s = ve; isspace(*s); s++);
			for (d = s; *d && !isspace(*d); d++);
			for (se = d; isspace(*d); d++);
			for (v = d; *v && !isspace(*v); v++);
			for (de = v; isspace(*v); v++);
			if (!*v)
				break;
			for (ve = v; *ve && !isspace(*ve); ve++);
			if (*ve)
				*ve = 0;
			else
				ve = 0;
			*de = 0;
			*se = 0;
			feature(s, d, v, 0);
			*se = ' ';
			*de = ' ';
			if (!ve)
				break;
			*ve++ = ' ';
		}
		state.synthesizing = 0;
		state.last = state.data + n - 1;
	}
	if (!fp)
		return state.data;
	if (!state.last)
	{
		n = strlen(value);
		goto ok;
	}
	s = (char*)fp->name;
	n = fp->length;
	d = state.data;
	for (;;)
	{
		while (isspace(*d))
			d++;
		if (!*d)
			break;
		if (strneq(d, s, n) && isspace(d[n]))
		{
			if (!value)
			{
				for (d += n + 1; *d && !isspace(*d); d++);
				for (; isspace(*d); d++);
				for (s = d; *s && !isspace(*s); s++);
				n = s - d;
				value = (const char*)d;
				goto ok;
			}
			for (s = d + n + 1; *s && !isspace(*s); s++);
			for (; isspace(*s); s++);
			for (v = s; *s && !isspace(*s); s++);
			n = s - v;
			if (strneq(v, value, n))
				goto ok;
			for (; isspace(*s); s++);
			if (*s)
				for (; *d = *s++; d++);
			else if (d != state.data)
				d--;
			break;
		}
		for (; *d && !isspace(*d); d++);
		for (; isspace(*d); d++);
		for (; *d && !isspace(*d); d++);
		for (; isspace(*d); d++);
		for (; *d && !isspace(*d); d++);
	}
	if (!value)
	{
		if (!fp->op)
			fp->value[0] = 0;
		return 0;
	}
	if (!value[0])
		value = "0";
	if (!path || !path[0] || path[0] == '/' && !path[1])
		path = "-";
	n += strlen(path) + strlen(value) + 3;
	if (d + n >= state.last)
	{
		int	c;
		int	i;

		i = d - state.data;
		state.data -= state.prefix;
		c = n + state.last - state.data + 3 * MAXVAL;
		c = roundof(c, 32);
		if (!(state.data = newof(state.data, char, c, 0)))
			return 0;
		state.last = state.data + c - 1;
		state.data += state.prefix;
		d = state.data + i;
	}
	if (d != state.data)
		*d++ = ' ';
	for (s = (char*)fp->name; *d = *s++; d++);
	*d++ = ' ';
	for (s = (char*)path; *d = *s++; d++);
	*d++ = ' ';
	for (s = (char*)value; *d = *s++; d++);
	setenviron(state.data - state.prefix);
	if (state.notify)
		(*state.notify)(NiL, NiL, state.data - state.prefix);
	n = s - (char*)value - 1;
 ok:
	if (n >= sizeof(fp->value))
		n = sizeof(fp->value) - 1;
	else if (n == 1 && (*value == '0' || *value == '-'))
		n = 0;
	strncpy(fp->value, value, n);
	fp->value[n] = 0;
	return fp->value;
}

/*
 * initialize the value for fp
 * if command!=0 then it is checked for on $PATH
 * synthesize(fp,path,succeed) called on success
 * otherwise synthesize(fp,path,fail) called
 */

static void
initialize(register Feature_t* fp, const char* path, const char* command, const char* succeed, const char* fail)
{
	register char*	p;
	register int	ok = 1;

	switch (fp->op)
	{
	case OP_hosttype:
		ok = 1;
		break;
	case OP_path_attributes:
		ok = 1;
		break;
	case OP_path_resolve:
		ok = fs3d(FS3D_TEST);
		break;
	case OP_universe:
		ok = streq(_UNIV_DEFAULT, "att");
		/*FALLTHROUGH...*/
	default:
		if (p = getenv("PATH"))
		{
			register int	r = 1;
			register char*	d = p;
			int		offset = stktell(stkstd);

			for (;;)
			{
				switch (*p++)
				{
				case 0:
					break;
				case ':':
					if (command && (fp->op != OP_universe || !ok))
					{
						if (r = p - d - 1)
						{
							sfwrite(stkstd, d, r);
							sfputc(stkstd, '/');
							sfputr(stkstd, command, 0);
							stkseek(stkstd, offset);
							if (!access(stkptr(stkstd, offset), X_OK))
							{
								ok = 1;
								if (fp->op != OP_universe)
									break;
							}
						}
						d = p;
					}
					r = 1;
					continue;
				case '/':
					if (r)
					{
						r = 0;
						if (fp->op == OP_universe)
						{
							if (strneq(p, "bin:", 4) || strneq(p, "usr/bin:", 8))
								break;
						}
					}
					if (fp->op == OP_universe)
					{
						if (strneq(p, "5bin", 4))
						{
							ok = 1;
							break;
						}
						if (strneq(p, "bsd", 3) || strneq(p, "ucb", 3))
						{
							ok = 0;
							break;
						}
					}
					continue;
				default:
					r = 0;
					continue;
				}
				break;
			}
		}
		break;
	}
	synthesize(fp, path, ok ? succeed : fail);
}

/*
 * value==0 get feature name
 * value!=0 set feature name
 * 0 returned if error or not defined; otherwise previous value
 */

static char*
feature(const char* name, const char* path, const char* value, Ast_conferror_f conferror)
{
	register Feature_t*	fp;
	register int		n;
	register Feature_t*	sp;

	if (value && (streq(value, "-") || streq(value, "0")))
		value = "";
	for (fp = state.features; fp && !streq(fp->name, name); fp = fp->next);
	if (!fp)
	{
		if (!value)
		{
			if (conferror)
				(*conferror)(&state, &state, 2, "%s: invalid symbol", name);
			return 0;
		}
		if (state.notify && !(*state.notify)(name, path, value))
			return 0;
		n = strlen(name);
		if (!(fp = newof(0, Feature_t, 1, n + 1)))
		{
			if (conferror)
				(*conferror)(&state, &state, 2, "%s: out of space", name);
			return 0;
		}
		fp->name = (const char*)fp + sizeof(Feature_t);
		strcpy((char*)fp->name, name);
		fp->length = n;
		fp->next = state.features;
		state.features = fp;
	}
	else if (value)
	{
		if (fp->flags & CONF_READONLY)
		{
			if (conferror)
				(*conferror)(&state, &state, 2, "%s: cannot set readonly symbol", fp->name);
			return 0;
		}
		if (state.notify && !streq(fp->value, value) && !(*state.notify)(name, path, value))
			return 0;
	}
	switch (fp->op)
	{

	case OP_conformance:
		if (value && (streq(value, "strict") || streq(value, "posix") || streq(value, "xopen")))
			value = fp->strict;
		n = streq(fp->value, fp->strict);
		synthesize(fp, path, value);
		if (!n && streq(fp->value, fp->strict))
			for (sp = state.features; sp; sp = sp->next)
				if (sp->op && sp->op != OP_conformance)
					astconf(sp->name, path, sp->strict);
		break;

	case OP_fs_3d:
		fp->value[0] = fs3d(value ? value[0] ? FS3D_ON : FS3D_OFF : FS3D_TEST) ? '1' : 0;
		break;

	case OP_hosttype:
		break;

	case OP_path_attributes:
#ifdef _PC_PATH_ATTRIBUTES
		{
			register char*	s;
			register char*	e;
			register long	v;

			/*
			 * _PC_PATH_ATTRIBUTES is a bitmap for 'a' to 'z'
			 */

			if ((v = pathconf(path, _PC_PATH_ATTRIBUTES)) == -1L)
				return 0;
			s = fp->value;
			e = s + sizeof(fp->value) - 1;
			for (n = 'a'; n <= 'z'; n++)
				if (v & (1 << (n - 'a')))
				{
					*s++ = n;
					if (s >= e)
						break;
				}
			*s = 0;
		}
#endif
		break;

	case OP_path_resolve:
		if (!synthesize(fp, path, value))
			initialize(fp, path, NiL, "logical", "metaphysical");
		break;

	case OP_universe:
#if _lib_universe
		if (getuniverse(fp->value) < 0)
			strcpy(fp->value, "att");
		if (value)
			setuniverse(value);
#else
#ifdef UNIV_MAX
		n = 0;
		if (value)
		{
			while (n < univ_max && !streq(value, univ_name[n])
				n++;
			if (n >= univ_max)
			{
				if (conferror)
					(*conferror)(&state, &state, 2, "%s: %s: universe value too large", fp->name, value);
				return 0;
			}
		}
#ifdef ATT_UNIV
		n = setuniverse(n + 1);
		if (!value && n > 0)
			setuniverse(n);
#else
		n = universe(value ? n + 1 : U_GET);
#endif
		if (n <= 0 || n >= univ_max)
			n = 1;
		strcpy(fp->value, univ_name[n - 1]);
#else
		if (!synthesize(fp, path, value))
			initialize(fp, path, "echo", "att", "ucb");
#endif
#endif
		break;

	default:
		synthesize(fp, path, value);
		break;

	}
	return fp->value;
}

/*
 * binary search for name in conf[]
 */

static int
lookup(register Lookup_t* look, const char* name)
{
	register Conf_t*	mid = (Conf_t*)conf;
	register Conf_t*	lo = mid;
	register Conf_t*	hi = mid + conf_elements;
	register int		v;
	register int		c;
	const Prefix_t*		p;

	look->flags = 0;
	look->call = -1;
	look->standard = -1;
	look->section = -1;
	while (*name == '_')
		name++;
	for (p = prefix; p < &prefix[prefix_elements]; p++)
		if (strneq(name, p->name, p->length) && ((c = name[p->length] == '_') || isdigit(name[p->length]) && name[p->length + 1] == '_'))
		{
			if ((look->call = p->call) < 0)
			{
				look->flags |= CONF_MINMAX;
				look->standard = p->standard;
			}
			name += p->length + c;
			if (isdigit(name[0]) && name[1] == '_')
			{
				look->section = name[0] - '0';
				name += 2;
			}
			else
				look->section = 1;
			break;
		}
	look->name = name;
	c = *((unsigned char*)name);
	while (lo <= hi)
	{
		mid = lo + (hi - lo) / 2;
		if (!(v = c - *((unsigned char*)mid->name)) && !(v = strcmp(name, mid->name)))
		{
			lo = (Conf_t*)conf;
			hi = lo + conf_elements - 1;
			if (look->standard >= 0 && look->standard != mid->standard)
				do
				{
					if (mid-- <= lo || !streq(mid->name, look->name))
						do
						{
							if (++mid > hi || !streq(mid->name, look->name))
								goto badstandard;
						} while (mid->standard != look->standard);
				} while (mid->standard != look->standard);
			if (look->section >= 0 && look->section != mid->section)
				do
				{
					if (mid-- <= lo || !streq(mid->name, look->name))
						do
						{
							if (++mid > hi || !streq(mid->name, look->name))
								goto badstandard;
						} while (mid->section != look->section);
				} while (mid->section != look->section);
			if (look->call >= 0 && look->call != mid->call)
				goto badcall;
			look->conf = mid;
			return 1;
		}
		else if (v > 0)
			lo = mid + 1;
		else
			hi = mid - 1;
	}
	look->error = 0;
	return 0;
 badcall:
	look->error = "call";
	return 0;
 badstandard:
	look->error = "standard";
	return 0;
 badsection:
	look->error = "section";
	return 0;
}

/*
 * print value line for p
 * if !name then value prefixed by "p->name="
 * if (flags & CONF_MINMAX) then default minmax value used
 */

static char*
print(Sfio_t* sp, register Lookup_t* look, const char* name, const char* path, Ast_conferror_f conferror)
{
	register Conf_t*	p = look->conf;
	register int		flags = look->flags|CONF_DEFINED;
	char*			call;
	int			offset;
	long			v;
	int			olderrno;
	char			buf[PATH_MAX];

	if (!name && p->call != CONF_confstr && (p->flags & (CONF_FEATURE|CONF_LIMIT)) && (p->flags & (CONF_LIMIT|CONF_PREFIXED)) != CONF_LIMIT)
	{
		flags |= CONF_PREFIXED;
		if (p->flags & CONF_DEFINED)
			flags |= CONF_MINMAX;
	}
	olderrno = errno;
	errno = 0;
	switch ((flags & CONF_MINMAX) && (p->flags & CONF_DEFINED) ? 0 : p->call)
	{
	case 0:
		if (p->flags & CONF_DEFINED)
			v = p->value;
		else
		{
			flags &= ~CONF_DEFINED;
			v = -1;
		}
		break;
	case CONF_confstr:
		call = "confstr";
		if (!(v = confstr(p->op, buf, sizeof(buf))))
		{
			v = -1;
			errno = EINVAL;
		}
		break;
	case CONF_pathconf:
		call = "pathconf";
		v = pathconf(path, p->op);
		break;
	case CONF_sysconf:
		call = "sysconf";
		v = sysconf(p->op);
		break;
	default:
		call = "synthesis";
		errno = EINVAL;
		v = -1;
		break;
	}
	if (v == -1)
	{
		if (!errno)
		{
			if ((p->flags & CONF_FEATURE) || !(p->flags & (CONF_LIMIT|CONF_MINMAX)))
				flags &= ~CONF_DEFINED;
		}
		else if (!(flags & CONF_PREFIXED))
		{
			if (!sp)
			{
				if (conferror)
				{
					(*conferror)(&state, &state, ERROR_SYSTEM|2, "%s: %s error", p->name, call);
					return 0;
				}
				return "";
			}
			flags &= ~CONF_DEFINED;
			flags |= CONF_ERROR;
		}
		else
			flags &= ~CONF_DEFINED;
	}
	errno = olderrno;
	if (sp)
		offset = -1;
	else
	{
		sp = stkstd;
		offset = stktell(sp);
	}
	if (!(flags & CONF_PREFIXED))
	{
		if (!name)
			sfprintf(sp, "%s=", p->name);
		if (flags & CONF_ERROR)
			sfprintf(sp, "error");
		else if (p->call == CONF_confstr)
			sfprintf(sp, "%s", buf);
		else if (v != -1)
			sfprintf(sp, "%ld", v);
		else if (flags & CONF_DEFINED)
			sfprintf(sp, "%lu", v);
		else
			sfprintf(sp, "undefined");
		if (!name)
			sfprintf(sp, "\n");
	}
	if (!name && p->call != CONF_confstr && (p->flags & (CONF_FEATURE|CONF_MINMAX)))
	{
		if (p->flags & CONF_UNDERSCORE)
			sfprintf(sp, "_");
		sfprintf(sp, "%s", prefix[p->standard].name);
		if (p->section > 1)
			sfprintf(sp, "%d", p->section);
		sfprintf(sp, "_%s=", p->name);
		if (p->flags & CONF_DEFINED)
		{
			if ((v = p->value) == -1 && ((p->flags & CONF_FEATURE) || !(p->flags & (CONF_LIMIT|CONF_MINMAX))))
				flags &= ~CONF_DEFINED;
			else
				flags |= CONF_DEFINED;
		}
		if (v != -1)
			sfprintf(sp, "%ld", v);
		else if (flags & CONF_DEFINED)
			sfprintf(sp, "%lu", v);
		else
			sfprintf(sp, "undefined");
		sfprintf(sp, "\n");
	}
	if (offset >= 0)
	{
		sfputc(sp, 0);
		stkseek(sp, offset);
		return stkptr(sp, offset);
	}
	return "";
}

/*
 * value==0 gets value for name
 * value!=0 sets value for name and returns previous value
 * path==0 implies path=="/"
 *
 * settable return values are in permanent store
 * non-settable return values are on stkstd
 *
 *	if (!strcmp(astgetconf("PATH_RESOLVE", NiL, NiL), "logical", 0))
 *		our_way();
 *
 *	universe = astgetconf("UNIVERSE", NiL, "att", 0);
 *	astgetconf("UNIVERSE", NiL, universe, 0);
 */

#define ALT	16

char*
astgetconf(const char* name, const char* path, const char* value, Ast_conferror_f conferror)
{
	register char*	s;
	char*		e;
	int		n;
	int		offset;
	long		v;
	Lookup_t	look;

	static char	buf[MAXVAL];

	if (!name)
	{
		if (path)
			return "";
		if (!(name = value))
		{
			if (state.data)
			{
				Ast_confdisc_f	notify;

#if _HUH20000515 /* doesn't work for shell builtins */
				free(state.data - state.prefix);
#endif
				state.data = 0;
				notify = state.notify;
				state.notify = 0;
				INITIALIZE();
				state.notify = notify;
			}
			return "";
		}
		value = 0;
	}
	INITIALIZE();
	if (!path)
		path = "/";
	if (isdigit(*name))
	{
		n = (int)strtol(name, &e, 10);
		if (!*e)
		{
			if (value)
				goto ro;
			v = sysconf(n);
			if (v == -1)
				return "error";
			sfsprintf(buf, sizeof(buf), "%lu", v);
			return buf;
		}
	}
	if (lookup(&look, name))
	{
		if (value)
		{
		ro:
			errno = EINVAL;
			if (conferror)
			{
				(*conferror)(&state, &state, 2, "%s: cannot set value", name);
				return 0;
			}
			return "";
		}
		return print(NiL, &look, name, path, conferror);
	}
	if (look.error)
	{
		errno = EINVAL;
		if (conferror)
		{
			(*conferror)(&state, &state, 2, "%s: invalid %s prefix", name, look.error);
			return 0;
		}
	}
	else
	{
		if ((n = strlen(name)) > 3 && n < (ALT + 3))
		{
			if (!strcmp(name + n - 3, "DEV"))
			{
				offset = stktell(stkstd);
				sfprintf(stkstd, "/dev/");
				for (s = (char*)name; s < (char*)name + n - 3; s++)
					sfputc(stkstd, isupper(*s) ? tolower(*s) : *s);
				sfputc(stkstd, 0);
				stkseek(stkstd, offset);
				s = stkptr(stkstd, offset);
				if (!access(s, F_OK))
				{
					if (value)
						goto ro;
					return s;
				}
			}
			else if (!strcmp(name + n - 3, "DIR"))
			{
				Lookup_t		altlook;
				char			altname[ALT];

				static const char*	dirs[] = { "/usr/lib", "/usr", "" };

				strcpy(altname, name);
				altname[n - 3] = 0;
				if (lookup(&altlook, altname))
				{
					if (value)
					{
						errno = EINVAL;
						if (conferror)
						{
							(*conferror)(&state, &state, 2, "%s: cannot set value", altname);
							return 0;
						}
						return "";
					}
					return print(NiL, &altlook, altname, path, conferror);
				}
				for (s = altname; *s; s++)
					if (isupper(*s))
						*s = tolower(*s);
				offset = stktell(stkstd);
				for (n = 0; n < elementsof(dirs); n++)
				{
					sfprintf(stkstd, "%s/%s/.", dirs[n], altname);
					sfputc(stkstd, 0);
					stkseek(stkstd, offset);
					s = stkptr(stkstd, offset);
					if (!access(s, F_OK))
					{
						if (value)
							goto ro;
						return s;
					}
				}
			}
		}
		if ((look.standard < 0 || look.standard == CONF_AST) && look.call <= 0 && look.section <= 1 && (s = feature(look.name, path, value, conferror)))
			return s;
		errno = EINVAL;
		if (conferror)
			return 0;
	}
	return "";
}

char*
astconf(const char* name, const char* path, const char* value)
{
	return astgetconf(name, path, value, 0);
}

/*
 * set discipline function to be called when features change
 * old discipline function returned
 */

Ast_confdisc_f
astconfdisc(Ast_confdisc_f new_notify)
{
	Ast_confdisc_f	old_notify;

	INITIALIZE();
	old_notify = state.notify;
	state.notify = new_notify;
	return old_notify;
}

/*
 * list all name=value entries on sp
 * path==0 implies path=="/"
 * flags==0 lists all values
 * flags&R_OK lists readonly values
 * flags&W_OK lists writeable values
 * flags&X_OK lists writeable values in inputable form
 */

void
astconflist(Sfio_t* sp, const char* path, int flags)
{
	char*		s;
	Feature_t*	fp;
	Lookup_t	look;
	int		olderrno;

	INITIALIZE();
	if (!path)
		path = "/";
	else if (access(path, F_OK))
	{
		errorf(&state, &state, 2, "%s: not found", path);
		return;
	}
	olderrno = errno;
	look.flags = 0;
	if (!flags)
		flags = R_OK|W_OK;
	else if (flags & X_OK)
		flags = W_OK|X_OK;
	if (flags & R_OK)
		for (look.conf = (Conf_t*)conf; look.conf < (Conf_t*)&conf[conf_elements]; look.conf++)
			print(sp, &look, NiL, path, errorf);
	if (flags & W_OK)
		for (fp = state.features; fp; fp = fp->next)
		{
#if HUH950401 /* don't get prefix happy */
			if (fp->standard >= 0)
				sfprintf(sp, "_%s_", prefix[fp->standard].name);
#endif
			if (!*(s = feature(fp->name, path, NiL, 0)))
				s = "0";
			if (flags & X_OK)
				sfprintf(sp, "%s %s - %s\n", state.id, fp->name, s); 
			else
				sfprintf(sp, "%s=%s\n", fp->name, s);
		}
	errno = olderrno;
}
