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
 * iconv intercept
 * minimally provides { utf*<=>bin ascii<=>ebcdic* }
 */

#include <ast.h>
#include <dirent.h>

#if _UWIN
#define _ICONV_LIST_PRIVATE_ \
	DIR*		dir;
#endif

#include <ccode.h>
#include <ctype.h>
#include <iconv.h>

#include "loclib.h"

#if _lib_iconv_open

#undef	iconv_t
#undef	iconv_f
#undef	iconv_list_t
#undef	iconv_open
#undef	iconv
#undef	iconv_close
#undef	iconv_list
#undef	iconv_move
#undef	iconv_name
#undef	iconv_write

#else

#define _ast_iconv_t		iconv_t
#define _ast_iconv_f		iconv_f
#define _ast_iconv_list_t	iconv_list_t
#define _ast_iconv_open		iconv_open
#define _ast_iconv		iconv
#define _ast_iconv_close	iconv_close
#define _ast_iconv_list		iconv_list
#define _ast_iconv_move		iconv_move
#define _ast_iconv_name		iconv_name
#define _ast_iconv_write	iconv_write

#endif

typedef struct
{
	iconv_t			cvt;
	const unsigned char*	map;
	_ast_iconv_f		fun;
#if _UWIN
	int			from;
	int			to;
#endif
} Conv_t;

static const _ast_iconv_list_t	codes[] =
{
	{
	"ascii",
	"(a|ascii|?(iso)646|?(iso)8859|latin)",
	"8 bit ascii",
	"ISO8859-%s",
	"1",
	CC_ASCII,
	},

	{
	"ebcdic",
	"(e|ebcdic?(-)?(1))",
	"X/Open ebcdic",
	"EBCDIC",
	0,
	CC_EBCDIC1,
	},

	{
	"ebcdic-2",
	"(i|ebcdic?(-)2|ibm)",
	"X/Open ibm",
	"EBCDIC-2",
	0,
	CC_EBCDIC2,
	},

	{
	"ebcdic-3",
	"(o|ebcdic?(-)3|?(cp|ibm)1047|mvs|openedition)",
	"mvs OpenEdition ebcdic",
	"EBCDIC-3",
	0,
	CC_EBCDIC3,
	},

	{
	"native",
	"(n|native|local)",
	"native code set",
	0,
	0,
	CC_NATIVE,
	},

	{
	"utf",
	"(u|unicode|utf)",
	"multibyte 8-bit unicode",
	"UTF-%s",
	"8",
	CC_UTF,
	},

	{
	"ume",
	"(m|ume|utf?(-)7)",
	"multibyte 7-bit unicode",
	"UTF-7",
	0,
	CC_UME,
	},

	{
	"euc",
	"((big|euc)*)",
	"euc family",
	0,
	0,
	CC_ICONV,
	},

	{
	"dos",
	"dos?(-)?(855)",
	"dos code page",
	"DOS855",
	0,
	CC_ICONV,
	},

	{
	"ucs",
	"ucs?(-)?(2)?(be)|utf-16?(be)",
	"unicode runes",
	"UCS-%s",
	"2",
	CC_UCS,
	},

	{
	"ucs-le",
	"ucs?(-)?(2)le|utf-16le",
	"little endian unicode runes",
	"UCS-%sLE",
	"2",
	CC_SCU,
	},

	{
	"win",
	"win?(-)?(1251)",
	"win code page",
	"WIN1251",
	0,
	CC_ICONV,
	},

	{ 0 },
};

#if _UWIN

#include <windows.h>
#include <dirent.h>

#ifndef CP_UCS2
#define CP_UCS2	0x0000
#endif

static char	_win_maps[] = "/reg/local_machine/SOFTWARE/Classes/MIME/Database/Charset";

/*
 * return the codeset index given its name or alias
 * the map is in the what? oh, the registry
 */

static int
_win_codeset(const char* name)
{
	register char*	s;
	char*		e;
	char*		prefix;
	int		n;
	Sfio_t*		sp;
	char		aka[128];
	char		tmp[128];

#if 0
sfprintf(sfstderr, "AHA#%d name=%s\n", __LINE__, name);
#endif
	if (!name[0] || name[0] == '-' && !name[1] || !strcasecmp(name, "local") || !strcasecmp(name, "native"))
		return CP_ACP;
	if (!strcasecmp(name, "utf") || !strcasecmp(name, "utf8") || !strcasecmp(name, "utf-8"))
		return CP_UTF8;
	if (!strcasecmp(name, "ucs") || !strcasecmp(name, "ucs2") || !strcasecmp(name, "ucs-2"))
		return CP_UCS2;
	if (name[0] == '0' && name[1] == 'x' && (n = strtol(name, &e, 0)) > 0 && !*e)
		return n;
	prefix = isdigit(name[0]) ? "windows-" : "";
	for (;;)
	{
		sfsprintf(tmp, sizeof(tmp), "%s/%s%s", _win_maps, prefix, name);
		if (!(sp = sfopen(0, tmp, "r")))
			break;
		for (;;)
		{
			if (!(s = sfgetr(sp, '\n', 0)))
			{
				sfclose(sp);
				return -1;
			}
			if (!strncasecmp(s, "AliasForCharSet=", 16))
			{
				n = sfvalue(sp) - 17;
				s += 16;
				if (n >= sizeof(aka))
					n = sizeof(aka) - 1;
				memcpy(aka, s, n);
				aka[n] = 0;
				sfclose(sp);
				name = (const char*)aka;
				break;
			}
			if (!strncasecmp(s, "CodePage=", 9))
			{
				s += 9;
				n = strtol(s, 0, 0);
				sfclose(sp);
				return n;
			}
		}
	}
	return -1;
}

/*
 * get and check the codeset indices
 */

static _ast_iconv_t
_win_iconv_open(register Conv_t* cc, const char* t, const char* f)
{
#if 0
sfprintf(sfstderr, "AHA#%d f=%s t=%s\n", __LINE__, f, t);
#endif
	if ((cc->from = _win_codeset(f)) < 0)
		return (_ast_iconv_t)(-1);
	if ((cc->to = _win_codeset(t)) < 0)
		return (_ast_iconv_t)(-1);
#if 0
sfprintf(sfstderr, "AHA#%d f=0x%04x t=0x%04x\n", __LINE__, cc->from, cc->to);
#endif
	return (_ast_iconv_t)cc;
}

/*
 * even though the indices already check out
 * they could still be rejected
 */

static size_t
_win_iconv(_ast_iconv_t cd, char** fb, size_t* fn, char** tb, size_t* tn)
{
	Conv_t*	cc = (Conv_t*)cd;
	size_t	un;
	size_t	tz;
	size_t	fz;
	size_t	bz;
	size_t	pz;
	size_t	oz;
	LPWSTR	ub;

#if 0
sfprintf(sfstderr, "AHA#%d from=0x%04x to=0x%04x\n", __LINE__, cc->from, cc->to);
#endif
	if (cc->from == cc->to)
	{
		/*
		 * easy
		 */

		fz = tz = (*fn < *tn) ? *fn : *tn;
		memcpy(*tb, *fb, fz);
	}
	else
	{
		ub = 0;
		un = *fn;

		/*
		 * from => ucs-2
		 */

		if (cc->to == CP_UCS2)
		{
			if ((tz = MultiByteToWideChar(cc->from, 0, (LPCSTR)*fb, (int)*fn, (LPWSTR)*tb, *tn)) && tz <= *tn)
			{
				fz = *fn;
				tz *= sizeof(WCHAR);
			}
			else
			{
				/*
				 * target too small
				 * binary search on input size to make it fit
				 */

				oz = 0;
				pz = *fn / 2;
				fz = *fn - pz;
				for (;;)
				{
					while (!(tz = MultiByteToWideChar(cc->from, 0, (LPCSTR)*fb, (int)fz, (LPWSTR)*tb, 0)))
						if (++fz >= *fn)
							goto nope;
					tz *= sizeof(WCHAR);
					if (tz == *tn)
						break;
					if (!(pz /= 2))
					{
						if (!(fz = oz))
							goto nope;
						break;
					}
					if (tz > *tn)
						fz -= pz;
					else
					{
						oz = fz;
						fz += pz;
					}
				}
			}
		}
		else
		{
			if (cc->from == CP_UCS2)
			{
				un = *fn / sizeof(WCHAR);
				ub = (LPWSTR)*fb;
			}
			else if (!(un = MultiByteToWideChar(cc->from, 0, (LPCSTR)*fb, (int)*fn, (LPWSTR)*tb, 0)))
				goto nope;
			else if (!(ub = (LPWSTR)malloc(un * sizeof(WCHAR))))
				goto nope;
			else if (!(un = MultiByteToWideChar(cc->from, 0, (LPCSTR)*fb, (int)*fn, (LPWSTR)ub, un)))
				goto nope;

			/*
			 * ucs-2 => to
			 */

			if (tz = WideCharToMultiByte(cc->to, 0, (LPCWSTR)ub, un, *tb, *tn, 0, 0))
				fz = *fn;
			else
			{
				/*
				 * target too small
				 * binary search on input size to make it fit
				 */

				oz = 0;
				pz = *fn / 2;
				bz = *fn - pz;
				for (;;)
				{
					while (!(fz = MultiByteToWideChar(cc->from, 0, (LPCSTR)*fb, (int)bz, (LPWSTR)ub, un)))
						if (++bz > *fn)
							goto nope;
					if (!(tz = WideCharToMultiByte(cc->to, 0, (LPCWSTR)ub, fz, *tb, 0, 0, 0)))
						goto nope;
					if (tz == *tn)
						break;
					if (!(pz /= 2))
					{
						if (!(fz = oz))
							goto nope;
						break;
					}
					if (tz > *tn)
						bz -= pz;
					else
					{
						oz = bz;
						bz += pz;
					}
				}
				if (!(tz = WideCharToMultiByte(cc->to, 0, (LPCWSTR)ub, fz, *tb, tz, 0, 0)))
					goto nope;
				fz *= sizeof(WCHAR);
			}
			if (ub != (LPWSTR)*fb)
				free(ub);
		}
	}
	*fb += fz;
	*fn -= fz;
	*tb += tz;
	*tn -= tz;
	return 0;
 nope:
	if (ub && ub != (LPWSTR)*fb)
		free(ub);
	return (size_t)(-1);
}

#endif

/*
 * return canonical character code set name for m
 * if b!=0 then canonical name placed in b of size n
 * <ccode.h> index returned
 */

int
_ast_iconv_name(register const char* m, register char* b, size_t n)
{
	register const _ast_iconv_list_t*	cp;
	const _ast_iconv_list_t*		bp;
	register int				c;
	register char*				e;
	register char*				s;
	int					sub[2];
	char					buf[16];

	if (!b)
	{
		b = buf;
		n = sizeof(buf);
	}
	e = b + n - 1;
	bp = 0;
	n = 0;
	if (!*m || *m == '-' && !*(m + 1))
		m = "native";
	for (cp = codes; cp->name; cp++)
		if (strgrpmatch(m, cp->match, sub, elementsof(sub) / 2, STR_MAXIMAL|STR_LEFT|STR_ICASE))
		{
			if (!(c = m[sub[1]]))
			{
				bp = cp;
				break;
			}
			if (sub[1] > n && !isalpha(c))
			{
				bp = cp;
				n = sub[1];
			}
		}
	if (cp = bp)
	{
		if (cp->canon)
		{
			if (cp->index)
			{
				for (m += sub[1]; *m && !isalnum(*m); m++);
				if (!isdigit(*m))
					m = cp->index;
			}
			else
				m = "1";
			b += sfsprintf(b, e - b, cp->canon, m);
		}
		else
		{
			if (cp->ccode == CC_NATIVE)
			{
				if ((ast.locale.set & (1<<AST_LC_CTYPE)) && (s = strchr(locales[AST_LC_CTYPE].sys, '.')))
				{
					m = (const char*)++s;
					if (s = strchr(s, '@'))
						c = s - (char*)m;
					else
						c = strlen(m);
					b += sfsprintf(b, e - b, "%-.*s", c, m);
				}
				else
				{
					switch (CC_NATIVE)
					{
					case CC_EBCDIC1:
						m = (const char*)"EBCDIC";
						break;
					case CC_EBCDIC2:
						m = (const char*)"EBCDIC-2";
						break;
					case CC_EBCDIC3:
						m = (const char*)"EBCDIC-3";
						break;
					default:
						m = (const char*)"ISO8859-1";
						break;
					}
					b += sfsprintf(b, e - b, "%s", m);
				}
			}
		}
		*b = 0;
		return cp->ccode;
	}
	while (b < e && (c = *m++))
	{
		if (islower(c))
			c = toupper(c);
		*b++ = c;
	}
	*b = 0;
	return CC_ICONV;
}

/*
 * convert utf-8 to bin
 */

static size_t
utf2bin(_ast_iconv_t cd, char** fb, size_t* fn, char** tb, size_t* tn)
{
	register unsigned char*		f;
	register unsigned char*		fe;
	register unsigned char*		t;
	register unsigned char*		te;
	register unsigned char*		p;
	register int			c;
	register int			w;

	f = (unsigned char*)(*fb);
	fe = f + (*fn);
	t = (unsigned char*)(*tb);
	te = t + (*tn);
	p = 0;
	while (t < te && f < fe)
	{
		p = f;
		c = *f++;
		if (c & 0x80)
		{
			if (!(c & 0x40))
			{
				f = p;
				break;
			}
			if (c & 0x20)
			{
				w = (c & 0x0F) << 12;
				if (f >= fe)
				{
					f = p;
					break;
				}
				c = *f++;
				if (c & 0x40)
				{
					f = p;
					break;
				}
				w |= (c & 0x3F) << 6;
			}
			else
				w = (c & 0x1F) << 6;
			if (f >= fe)
			{
				f = p;
				break;
			}
			c = *f++;
			w |= (c & 0x3F);
		}
		else
			w = c;
		*t++ = w;
	}
	*fn -= (char*)f - (*fb);
	*fb = (char*)f;
	*tn -= (char*)t - (*tb);
	*tb = (char*)t;
	return f == p ? (size_t)(-1) : 0;
}

/*
 * convert bin to utf-8
 */

static size_t
bin2utf(_ast_iconv_t cd, char** fb, size_t* fn, char** tb, size_t* tn)
{
	register unsigned char*		f;
	register unsigned char*		fe;
	register unsigned char*		t;
	register unsigned char*		te;
	register unsigned char*		p;
	register int			c;
	wchar_t				w;

	f = (unsigned char*)(*fb);
	fe = f + (*fn);
	t = (unsigned char*)(*tb);
	te = t + (*tn);
	p = 0;
	while (f < fe && t < te)
	{
		p = f;
#if _lib_mbtowc && MB_LEN_MAX > 1
		if (ast.locale.set & (1<<AST_LC_CTYPE))
		{
			if ((c = mbtowc(&w, (char*)f, fe - f)) <= 0)
				break;
			f += c;
		}
		else
#endif
			w = *f++;
		if (!(w & ~0x7F))
			*t++ = w;
		else
		{
			if (!(w & ~0x7FF))
			{
				if (t >= (te - 2))
					break;
				*t++ = 0xC0 + (w >> 6);
			}
			else if (!(w & ~0xffff))
			{
				if (t >= (te - 3))
					break;
				*t++ = 0xE0 + (w >> 12);
				*t++ = 0x80 + ((w >> 6 ) & 0x3F);
			}
			else
				break;
			*t++ = 0x80 + (w & 0x3F);
		}
	}
	*fn -= (char*)f - (*fb);
	*fb = (char*)f;
	*tn -= (char*)t - (*tb);
	*tb = (char*)t;
	return f == p ? (size_t)(-1) : 0;
}

static const unsigned char	ume_D[] =
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789'(),-./:?!\"#$%&*;<=>@[]^_`{|} \t\n";

static const unsigned char	ume_M[] =
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static unsigned char		ume_d[UCHAR_MAX+1];

static unsigned char		ume_m[UCHAR_MAX+1];

#define NOE			0xFF
#define UMEINIT()		(ume_d[ume_D[0]]?0:umeinit())

/*
 * initialize the ume tables
 */

static int
umeinit(void)
{
	register const unsigned char*	s;
	register int			i;
	register int			c;

	if (!ume_d[ume_D[0]])
	{
		s = ume_D; 
		while (c = *s++)
			ume_d[c] = 1;
		memset(ume_m, NOE, sizeof(ume_m));
		for (i = 0; c = ume_M[i]; i++)
			ume_m[c] = i;
	}
	return 0;
}

/*
 * convert utf-7 to bin
 */

static size_t
ume2bin(_ast_iconv_t cd, char** fb, size_t* fn, char** tb, size_t* tn)
{
	register unsigned char*		f;
	register unsigned char*		fe;
	register unsigned char*		t;
	register unsigned char*		te;
	register int			s;
	register int			c;
	register int			w;

	UMEINIT();
	f = (unsigned char*)(*fb);
	fe = f + (*fn);
	t = (unsigned char*)(*tb);
	te = t + (*tn);
	s = 0;
	while (f < fe && t < te)
	{
		c = *f++;
		if (s)
		{
			if (c == '-' && s > 1)
				s = 0;
			else if ((w = ume_m[c]) == NOE)
			{
				s = 0;
				*t++ = c;
			}
			else if (f >= (fe - 2))
				break;
			else
			{
				s = 2;
				w = (w << 6) | ume_m[*f++];
				w = (w << 6) | ume_m[*f++];
				if (!(w & ~0xFF))
					*t++ = w;
				else if (t >= (te - 1))
					break;
				else
				{
					*t++ = (w >> 8) & 0xFF;
					*t++ = w & 0xFF;
				}
			}
		}
		else if (c == '+')
			s = 1;
		else
			*t++ = c;
	}
	*fn -= (char*)f - (*fb);
	*fb = (char*)f;
	*tn -= (char*)t - (*tb);
	*tb = (char*)t;
	return 0;
}

/*
 * convert bin to utf-7
 */

static size_t
bin2ume(_ast_iconv_t cd, char** fb, size_t* fn, char** tb, size_t* tn)
{
	register unsigned char*		f;
	register unsigned char*		fe;
	register unsigned char*		t;
	register unsigned char*		te;
	register unsigned char*		p;
	register int			c;
	register int			s;
	wchar_t				w;

	UMEINIT();
	f = (unsigned char*)(*fb);
	fe = f + (*fn);
	t = (unsigned char*)(*tb);
	te = t + (*tn);
	p = 0;
	s = 0;
	while (f < fe && t < (te - s))
	{
		p = f;
#if _lib_mbtowc && MB_LEN_MAX > 1
		if (ast.locale.set & (1<<AST_LC_CTYPE))
		{
			if ((c = mbtowc(&w, (char*)f, fe - f)) <= 0)
				break;
			f += c;
		}
		else
#endif
			w = *f++;
		if (!(w & ~0x7F) && ume_d[w])
		{
			if (s)
			{
				s = 0;
				*t++ = '-';
			}
			*t++ = w;
		}
		else if (t >= (te - (4 + s)))
				break;
		else
		{
			if (!s)
			{
				s = 1;
				*t++ = '+';
			}
			*t++ = ume_M[(w >> 12) & 0x3F];
			*t++ = ume_M[(w >> 6) & 0x3F];
			*t++ = ume_M[w & 0x3F];
		}
	}
	if (s)
		*t++ = '-';
	*fn -= (char*)f - (*fb);
	*fb = (char*)f;
	*tn -= (char*)t - (*tb);
	*tb = (char*)t;
	return f == p ? (size_t)(-1) : 0;
}

/*
 * convert ucs-2 to bin with no byte swap
 */

static size_t
ucs2bin(_ast_iconv_t cd, char** fb, size_t* fn, char** tb, size_t* tn)
{
	register unsigned char*		f;
	register unsigned char*		fe;
	register unsigned char*		t;
	register unsigned char*		te;
	register unsigned char*		p;
	register int			w;

	f = (unsigned char*)(*fb);
	fe = f + (*fn);
	t = (unsigned char*)(*tb);
	te = t + (*tn);
	p = 0;
	while (f < (fe - 1) && t < te)
	{
		p = f;
		w = *f++;
		w = (w << 8) | *f++;
		if (!(w & ~0xFF))
			*t++ = w;
		else if (t >= (te - 1))
		{
			f = p;
			break;
		}
		else
		{
			*t++ = (w >> 8) & 0xFF;
			*t++ = w & 0xFF;
		}
	}
	*fn -= (char*)f - (*fb);
	*fb = (char*)f;
	*tn -= (char*)t - (*tb);
	*tb = (char*)t;
	return 0;
}

/*
 * convert bin to ucs-2 with no byte swap
 */

static size_t
bin2ucs(_ast_iconv_t cd, char** fb, size_t* fn, char** tb, size_t* tn)
{
	register unsigned char*		f;
	register unsigned char*		fe;
	register unsigned char*		t;
	register unsigned char*		te;
	register int			c;
	wchar_t				w;

	f = (unsigned char*)(*fb);
	fe = f + (*fn);
	t = (unsigned char*)(*tb);
	te = t + (*tn);
	while (f < fe && t < (te - 1))
	{
#if _lib_mbtowc && MB_LEN_MAX > 1
		if (ast.locale.set & (1<<AST_LC_CTYPE))
		{
			if ((c = mbtowc(&w, (char*)f, fe - f)) <= 0)
				break;
			f += c;
		}
		else
#endif
			w = *f++;
		*t++ = (w >> 8) & 0xFF;
		*t++ = w & 0xFF;
	}
	*fn -= (char*)f - (*fb);
	*fb = (char*)f;
	*tn -= (char*)t - (*tb);
	*tb = (char*)t;
	return 0;
}

/*
 * convert ucs-2 to bin with byte swap
 */

static size_t
scu2bin(_ast_iconv_t cd, char** fb, size_t* fn, char** tb, size_t* tn)
{
	register unsigned char*		f;
	register unsigned char*		fe;
	register unsigned char*		t;
	register unsigned char*		te;
	register unsigned char*		p;
	register int			w;

	f = (unsigned char*)(*fb);
	fe = f + (*fn);
	t = (unsigned char*)(*tb);
	te = t + (*tn);
	p = 0;
	while (f < (fe - 1) && t < te)
	{
		p = f;
		w = *f++;
		w = w | (*f++ << 8);
		if (!(w & ~0xFF))
			*t++ = w;
		else if (t >= (te - 1))
		{
			f = p;
			break;
		}
		else
		{
			*t++ = (w >> 8) & 0xFF;
			*t++ = w & 0xFF;
		}
	}
	*fn -= (char*)f - (*fb);
	*fb = (char*)f;
	*tn -= (char*)t - (*tb);
	*tb = (char*)t;
	return 0;
}

/*
 * convert bin to ucs-2 with byte swap
 */

static size_t
bin2scu(_ast_iconv_t cd, char** fb, size_t* fn, char** tb, size_t* tn)
{
	register unsigned char*		f;
	register unsigned char*		fe;
	register unsigned char*		t;
	register unsigned char*		te;
	register int			c;
	wchar_t				w;

	f = (unsigned char*)(*fb);
	fe = f + (*fn);
	t = (unsigned char*)(*tb);
	te = t + (*tn);
	while (f < fe && t < (te - 1))
	{
#if _lib_mbtowc && MB_LEN_MAX > 1
		if (ast.locale.set & (1<<AST_LC_CTYPE))
		{
			if ((c = mbtowc(&w, (char*)f, fe - f)) <= 0)
				break;
			f += c;
		}
		else
#endif
			w = *f++;
		*t++ = w & 0xFF;
		*t++ = (w >> 8) & 0xFF;
	}
	*fn -= (char*)f - (*fb);
	*fb = (char*)f;
	*tn -= (char*)t - (*tb);
	*tb = (char*)t;
	return 0;
}

/*
 * open a character code conversion map from f to t
 */

_ast_iconv_t
_ast_iconv_open(const char* t, const char* f)
{
	register Conv_t*	cc;
	int			fc;
	int			tc;

	char			fr[64];
	char			to[64];

#if 0
sfprintf(sfstderr, "AHA#%d f=%s t=%s\n", __LINE__, f, t);
#endif
	if (!(cc = newof(0, Conv_t, 1, 0)))
		return (iconv_t)(-1);
	cc->cvt = (iconv_t)(-1);
	fc = _ast_iconv_name(f, fr, sizeof(fr));
	tc = _ast_iconv_name(t, to, sizeof(to));
#if 0
sfprintf(sfstderr, "AHA#%d f=%s:%s:%d t=%s:%s:%d\n", __LINE__, f, fr, fc, t, to, tc);
#endif

	/*
	 * (soft) fail if to is "" or "-" and conversion is the identity
	 */

	if ((!*t || *t == '-' && !*(t + 1)) && streq(fr, to))
	{
		free(cc);
		return (iconv_t)(-1);
	}

	/*
	 * 8 bit maps are the easiest
	 */

	if (fc >= 0 && tc >= 0)
		cc->map = CCMAP(fc, tc);
#if _lib_iconv_open
	else if ((cc->cvt = iconv_open(to, fr)) != (iconv_t)(-1))
		cc->fun = (_ast_iconv_f)iconv;
#endif
#if _UWIN
	else if ((cc->cvt = _win_iconv_open(cc, to, fr)) != (_ast_iconv_t)(-1))
		cc->fun = (_ast_iconv_f)_win_iconv;
#endif
	else if (fc >= 0)
	{
		switch (tc)
		{
		case CC_UTF:
			cc->fun = bin2utf;
			break;
		case CC_UME:
			cc->fun = bin2ume;
			break;
		case CC_UCS:
			cc->fun = bin2ucs;
			break;
		case CC_SCU:
			cc->fun = bin2scu;
			break;
		default:
			goto nope;
		}
	}
	else if (tc >= 0)
	{
		switch (fc)
		{
		case CC_UTF:
			cc->fun = utf2bin;
			break;
		case CC_UME:
			cc->fun = ume2bin;
			break;
		case CC_UCS:
			cc->fun = ucs2bin;
			break;
		case CC_SCU:
			cc->fun = scu2bin;
			break;
		default:
			goto nope;
		}
	}
	else
	{
	nope:
		free(cc);
		return (iconv_t)(-1);
	}
	return (iconv_t)cc;
}

/*
 * close a character code conversion map
 */

int
_ast_iconv_close(_ast_iconv_t cd)
{
	Conv_t*	cc;
	int	r = 0;

	if (cd == (_ast_iconv_t)(-1))
		return -1;
	cc = (Conv_t*)cd;
#if _lib_iconv_open
	if (cc->cvt != (iconv_t)(-1))
		r = iconv_close(cc->cvt);
#endif
	free(cc);
	return r;
}

/*
 * copy *fb size *fn to *tb size *tn
 * fb,fn tb,tn updated on return
 */

size_t
_ast_iconv(_ast_iconv_t cd, char** fb, size_t* fn, char** tb, size_t* tn)
{
	Conv_t*				cc = (Conv_t*)cd;
	register unsigned char*		f;
	register unsigned char*		t;
	register unsigned char*		e;
	register const unsigned char*	m;
	register size_t			n;

	if (cc->fun)
		return (*cc->fun)(cc->cvt, fb, fn, tb, tn);
	n = *fn;
	if (n > *tn)
		n = *tn;
	if (m = cc->map)
	{
		f = (unsigned char*)(*fb);
		e = f + n;
		t = (unsigned char*)(*tb);
		while (f < e)
			*t++ = m[*f++];
	}
	else
		memcpy(*tb, *fb, n);
	*fb += n;
	*fn -= n;
	*tb += n;
	*tn -= n;
	return 0;
}

/*
 * write *fb size *fn to op
 * fb,fn updated on return
 * total bytes written to op returned
 */

ssize_t
_ast_iconv_write(_ast_iconv_t cd, Sfio_t* op, char** fb, size_t* fn, size_t* e)
{
	char*		tb;
	char*		ts;
	size_t		tn;
	size_t		r;

	r = 0;
	while (*fn > 0)
	{
		if (!(tb = (char*)sfreserve(op, SF_UNBOUND, SF_LOCKR)))
			return r ? r : -1;
		ts = tb;
		tn = sfvalue(op);
		while (_ast_iconv(cd, fb, fn, &ts, &tn) && *fn > 0)
		{
			if (tn > 0)
			{
				*ts++ = '_';
				tn--;
			}
			if (e)
				(*e)++;
			(*fb)++;
			(*fn)--;
		}
		sfwrite(op, tb, ts - tb);
		r += ts - tb;
	}
	return r;
}

/*
 * move n bytes from ip to op
 */

ssize_t
_ast_iconv_move(_ast_iconv_t cd, Sfio_t* ip, Sfio_t* op, size_t n, size_t* e)
{
	char*		fb;
	char*		fs;
	char*		tb;
	char*		ts;
	size_t		fn;
	size_t		tn;
	ssize_t		r = 0;

	fn = n;
	while (fb = (char*)sfreserve(ip, fn, SF_LOCKR))
	{
		fs = fb;
		fn = sfvalue(ip);
		if (!(tb = (char*)sfreserve(op, SF_UNBOUND, SF_LOCKR)))
		{
			sfread(ip, fb, 0);
			return r ? r : -1;
		}
		ts = tb;
		tn = sfvalue(op);
		while (_ast_iconv(cd, &fs, &fn, &ts, &tn) && fn > 0)
		{
			if (tn > 0)
			{
				*ts++ = '_';
				tn--;
			}
			if (e)
				(*e)++;
			fs++;
			fn--;
		}
		sfwrite(op, tb, ts - tb);
		r += ts - tb;
		sfread(ip, fb, fs - fb);
		if (n != SF_UNBOUND)
		{
			if (n <= (fs - fb))
				break;
			n -= fs - fb;
		}
		fn++;
	}
	return r;
}

/*
 * iconv_list_t iterator
 * call with arg 0 to start
 * prev return value is current arg
 */

_ast_iconv_list_t*
_ast_iconv_list(_ast_iconv_list_t* cp)
{
#if _UWIN
	struct dirent*	ent;

	if (!cp)
	{
		if (cp = newof(0, _ast_iconv_list_t, 1, 0))
		{
			if (!(cp->dir = opendir(_win_maps)))
			{
				free(cp);
				return (_ast_iconv_list_t*)codes;
			}
		}
		else
			return (_ast_iconv_list_t*)codes;
	}
	if (cp->dir)
	{
		if (ent = readdir(cp->dir))
		{
			cp->name = cp->match = cp->desc = (const char*)ent->d_name;
			return cp;
		}
		closedir(cp->dir);
		free(cp);
		return (_ast_iconv_list_t*)codes;
	}
#else
	if (!cp)
		return (_ast_iconv_list_t*)codes;
#endif
	if (++cp >= (_ast_iconv_list_t*)&codes[elementsof(codes) - 1])
		return 0;
	return cp;
}
