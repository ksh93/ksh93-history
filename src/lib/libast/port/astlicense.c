/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1985-2004 AT&T Corp.                  *
*                      and is licensed under the                       *
*          Common Public License, Version 1.0 (the "License")          *
*                        by AT&T Corp. ("AT&T")                        *
*      Any use, downloading, reproduction or distribution of this      *
*      software constitutes acceptance of the License.  A copy of      *
*                     the License is available at                      *
*                                                                      *
*         http://www.research.att.com/sw/license/cpl-1.0.html          *
*         (with md5 checksum 8a5e0081c856944e76c69a1cf29c2e8b)         *
*                                                                      *
*              Information and Software Systems Research               *
*                            AT&T Research                             *
*                           Florham Park NJ                            *
*                                                                      *
*                 Glenn Fowler <gsf@research.att.com>                  *
*                  David Korn <dgk@research.att.com>                   *
*                   Phong Vo <kpv@research.att.com>                    *
*                                                                      *
***********************************************************************/
#pragma prototyped

/*
 * Glenn Fowler
 * AT&T Research
 *
 * generate a license comment -- see proto(1)
 *
 * NOTE: coded for minimal library dependence
 *	 not so for the legal department
 */

#ifndef	_PPLIB_H
#include <ast.h>
#include <time.h>
#endif

#include <hashkey.h>

#undef	copy
#undef	END

#define USAGE			1
#define SPECIAL			2
#define PROPRIETARY		3
#define NONEXCLUSIVE		4
#define NONCOMMERCIAL		5
#define OPEN			6
#define GPL			7
#define FREE			8
#define CPL			9

#define AUTHOR			0
#define COMPANY			1
#define CORPORATION		2
#define DOMAIN			3
#define LOCATION		4
#define NOTICE			5
#define ORGANIZATION		6
#define PACKAGE			7
#define SINCE			8
#define STYLE			9
#define URL			10
#define URLMD5			11
#define VERSION			12
#define ITEMS			13

#define IDS			64

#define COMDATA			70
#define COMLINE			(COMDATA+4)
#define COMLONG			(COMDATA-32)
#define COMMENT(x,b,s,u)	comment(x,b,s,sizeof(s)-1,u)

#define PUT(b,c)		(((b)->nxt<(b)->end)?(*(b)->nxt++=(c)):((c),(-1)))
#define BUF(b)			((b)->buf)
#define USE(b)			((b)->siz=(b)->nxt-(b)->buf,(b)->nxt=(b)->buf,(b)->siz)
#define SIZ(b)			((b)->nxt-(b)->buf)
#define END(b)			(*((b)->nxt>=(b)->end?((b)->nxt=(b)->end-1):(b)->nxt)=0,(b)->nxt-(b)->buf)

#ifndef NiL
#define NiL			((char*)0)
#endif

typedef struct
{
	char*		buf;
	char*		nxt;
	char*		end;
	int		siz;
} Buffer_t;

typedef struct
{
	char*		data;
	int		size;
	int		quote;
} Item_t;

typedef struct
{
	Item_t		name;
	Item_t		value;
} Id_t;

typedef struct
{
	int		test;
	int		type;
	int		verbose;
	int		ids;
	Item_t		item[ITEMS];
	Id_t		id[IDS];
	char		cc[3];
} Notice_t;

/*
 * return variable index given hash
 */

static int
index(unsigned long h)
{
	switch (h)
	{
	case HASHKEY6('a','u','t','h','o','r'):
		return AUTHOR;
	case HASHKEY6('c','o','m','p','a','n'):
		return COMPANY;
	case HASHKEY6('c','o','r','p','o','r'):
		return CORPORATION;
	case HASHKEY6('d','o','m','a','i','n'):
		return DOMAIN;
	case HASHKEY6('l','o','c','a','t','i'):
		return LOCATION;
	case HASHKEY6('n','o','t','i','c','e'):
		return NOTICE;
	case HASHKEY6('o','r','g','a','n','i'):
		return ORGANIZATION;
	case HASHKEY6('p','a','c','k','a','g'):
		return PACKAGE;
	case HASHKEY5('s','i','n','c','e'):
		return SINCE;
	case HASHKEY4('t','y','p','e'):
		return STYLE;
	case HASHKEY3('u','r','l'):
		return URL;
	case HASHKEY6('u','r','l','m','d','5'):
		return URLMD5;
	case HASHKEY6('v','e','r','s','i','o'):
		return VERSION;
	}
	return -1;
}

/*
 * copy s of size n to b
 * n<0 means 0 terminated string
 */

static void
copy(register Buffer_t* b, register char* s, int n)
{
	if (n < 0)
		n = strlen(s);
	while (n--)
		PUT(b, *s++);
}

/*
 * center and copy comment line s to p
 * if s==0 then
 *	n>0	first frame line
 *	n=0	blank line
 *	n<0	last frame line
 * if u>0 then s converted to upper case
 * if u<0 then s is left justified
 */

static void
comment(Notice_t* notice, register Buffer_t* b, register char* s, register int n, int u)
{
	register int	i;
	register int	m;
	register int	x;
	int		cc;

	cc = notice->cc[1];
	if (!s)
	{
		if (n)
		{
			PUT(b, notice->cc[n > 0 ? 0 : 1]);
			for (i = 0; i < COMDATA; i++)
				PUT(b, cc);
			PUT(b, notice->cc[n > 0 ? 1 : 2]);
		}
		else
			s = "";
	}
	if (s)
	{
		if (n > COMDATA)
			n = COMDATA;
		PUT(b, cc);
		m = (u < 0) ? 1 : (COMDATA - n) / 2;
		if ((x = COMDATA - m - n) < 0)
			n--;
		while (m-- > 0)
			PUT(b, ' ');
		while (n-- > 0)
		{
			i = *s++;
			if (u > 0 && i >= 'a' && i <= 'z')
				i = i - 'a' + 'A';
			PUT(b, i);
		}
		while (x-- > 0)
			PUT(b, ' ');
		PUT(b, cc);
	}
	PUT(b, '\n');
}

/*
 * expand simple ${...}
 */

static void
expand(Notice_t* notice, Item_t* item, register Buffer_t* b)
{
	register char*	t = item->data;
	register char*	e = t + item->size;
	register int	q = item->quote;
	register char*	x;
	register char*	z;
	register int	c;
	int		n;
	unsigned long	h;

	while (t < e)
	{
		if (*t == '$' && t < (e + 2) && *(t + 1) == '{')
		{
			h = 0;
			n = 0;
			t += 2;
			while (t < e && (c = *t++) != '}')
			{
				if (c == '.')
				{
					h = 0;
					n = 0;
				}
				else if (n++ < HASHKEYMAX)
					h = HASHKEYPART(h, c);
			}
			if ((c = index(h)) >= 0 && (x = notice->item[c].data))
			{
				z = x + notice->item[c].size;
				while (x < z)
					PUT(b, *x++);
			}
		}
		else if (q && *t == '\\' && (*(t + 1) == q || *(t + 1) == '\\'))
			t++;
		else
			PUT(b, *t++);
	}
}

/*
 * generate a copright notice
 */

static void
copyright(Notice_t* notice, register Buffer_t* b)
{
	register char*	x;
	register char*	t;
	time_t		clock;

	copy(b, "Copyright (c) ", -1);
	if (notice->test)
		clock = (time_t)1000212300;
	else
		time(&clock);
	t = ctime(&clock) + 20;
	if ((x = notice->item[SINCE].data) && strncmp(x, t, 4))
	{
		expand(notice, &notice->item[SINCE], b);
		PUT(b, '-');
	}
	copy(b, t, 4);
	if (notice->item[CORPORATION].data)
	{
		PUT(b, ' ');
		expand(notice, &notice->item[CORPORATION], b);
		PUT(b, ' ');
		copy(b, "Corp.", -1);
	}
	else if (notice->item[COMPANY].data)
	{
		PUT(b, ' ');
		expand(notice, &notice->item[COMPANY], b);
	}
}

/*
 * read the license file and generate a comment in p, length size
 * license length in p returned, -1 on error
 * -1 return places 0 terminated error string in p
 */

int
astlicense(char* p, int size, char* file, char* options, int cc1, int cc2, int cc3)
{
	register char*	s;
	register char*	v;
	register char*	x;
	register int	c;
	int		i;
	int		k;
	int		n;
	int		q;
	int		contributor;
	int		first;
	int		line;
	int		quote;
	unsigned long	h;
	char		tmpbuf[COMLINE];
	char		info[8 * 1024];
	Notice_t	notice;
	Item_t		item;
	Buffer_t	buf;
	Buffer_t	tmp;

	buf.end = (buf.buf = buf.nxt = p) + size;
	tmp.end = (tmp.buf = tmp.nxt = tmpbuf) + sizeof(tmpbuf);
	if (file && *file)
	{
		if ((i = open(file, O_RDONLY)) < 0)
		{
			copy(&buf, file, -1);
			copy(&buf, ": cannot open", -1);
			PUT(&buf, 0);
			return -1;
		}
		n = read(i, info, sizeof(info) - 1);
		close(i);
		if (n < 0)
		{
			copy(&buf, file, -1);
			copy(&buf, ": cannot read", -1);
			PUT(&buf, 0);
			return -1;
		}
		s = info;
		s[n] = 0;
	}
	else if (!options)
		return 0;
	else
	{
		s = options;
		options = 0;
	}
	notice.test = 0;
	notice.type = 0;
	notice.verbose = 0;
	notice.ids = 0;
	notice.cc[0] = cc1;
	notice.cc[1] = cc2;
	notice.cc[2] = cc3;
	for (i = 0; i < ITEMS; i++)
		notice.item[i].data = 0;
	contributor = i = k = 0;
	line = 0;
	for (;;)
	{
		for (first = 1; c = *s; first = 0)
		{
			while (c == ' ' || c == '\t' || c == '\n' && ++line || c == '\r' || c == ',' || c == ';' || c == ')')
				c = *++s;
			if (!c)
				break;
			if (c == '#')
			{
				while (*++s && *s != '\n');
				if (*s)
					s++;
				line++;
				continue;
			}
			if (c == '\n')
			{
				s++;
				line++;
				continue;
			}
			if (c == '[')
				c = *++s;
			x = s;
			n = 0;
			h = 0;
			while (c && c != '=' && c != ']' && c != ')' && c != ' ' && c != '\t' && c != '\n' && c != '\r')
			{
				if (n++ < HASHKEYMAX)
					h = HASHKEYPART(h, c);
				c = *++s;
			}
			n = s - x;
			if (c == ']')
				c = *++s;
			if (c == '=')
			{
				q = ((c = *++s) == '"' || c == '\'') ? *s++ : 0;
				if (c == '(')
				{
					s++;
					if (h == HASHKEY6('l','i','c','e','n','s'))
						contributor = 0;
					else if (h == HASHKEY6('c','o','n','t','r','i'))
						contributor = 1;
					else
					{
						q = 1;
						i = 0;
						for (;;)
						{
							switch (*s++)
							{
							case 0:
								s--;
								break;
							case '(':
								if (!i)
									q++;
								continue;
							case ')':
								if (!i && !--q)
									break;
								continue;
							case '"':
							case '\'':
								if (!i)
									i = *(s - 1);
								else if (i == *(s - 1))
									i = 0;
								continue;
							case '\\':
								if (*s == i && i == '"')
									i++;
								continue;
							case '\n':
								line++;
								continue;
							default:
								continue;
							}
							break;
						}
					}
					continue;
				}
				quote = 0;
				v = s;
				while ((c = *s) && (q == '"' && (c == '\\' && (*(s + 1) == '"' || *(s + 1) == '\\') && s++ && (quote = q)) || q && c != q || !q && c != ' ' && c != '\t' && c != '\n' && c != '\r' && c != ',' && c != ';'))
				{
					if (c == '\n')
						line++;
					s++;
				}
				if (c == '\n')
					line++;
				if (contributor)
				{
					for (i = 0; i < notice.ids; i++)
						if (n == notice.id[i].name.size && !strncmp(x, notice.id[i].name.data, n))
							break;
					if (i < IDS)
					{
						notice.id[i].name.data = x;
						notice.id[i].name.size = n;
						notice.id[i].name.quote = 0;
						notice.id[i].value.data = v;
						notice.id[i].value.size = s - v;
						notice.id[i].value.quote = quote;
						if (notice.ids <= i)
							notice.ids = i + 1;
					}
				}
				else
				{
					if ((c = index(h)) == STYLE)
					{
						if (!strncmp(v, "cpl", 3))
							notice.type = CPL;
						else if (!strncmp(v, "gpl", 3) || !strncmp(v, "copyleft", 8))
							notice.type = GPL;
						else if (!strncmp(v, "open", 4))
							notice.type = OPEN;
						else if (!strncmp(v, "free", 4))
							notice.type = FREE;
						else if (!strncmp(v, "nonexclusive", 12) || !strncmp(v, "individual", 10))
							notice.type = NONEXCLUSIVE;
						else if (!strncmp(v, "noncommercial", 13))
							notice.type = NONCOMMERCIAL;
						else if (!strncmp(v, "proprietary", 11))
							notice.type = PROPRIETARY;
						else if (!strncmp(v, "special", 7))
							notice.type = SPECIAL;
						else if (!strncmp(v, "none", 4))
							return 0;
						else if (!strncmp(v, "test", 4))
							notice.test = 1;
						else if (!strncmp(v, "usage", 5))
						{
							notice.type = USAGE;
							c = -1;
						}
						else if (!strncmp(v, "verbose", 7))
						{
							notice.verbose = 1;
							c = -1;
						}
						else if (!strncmp(v, "check", 4))
						{
							comment(&notice, &buf, NiL, 0, 0);
							return END(&buf);
						}
					}
					if (c >= 0)
					{
						notice.item[c].data = (notice.item[c].size = s - v) ? v : (char*)0;
						notice.item[c].quote = quote;
						k = 1;
					}
				}
			}
			else if (!first)
			{
				if (file)
				{
					copy(&buf, "\"", -1);
					copy(&buf, file, -1);
					copy(&buf, "\", line ", -1);
					x = &tmpbuf[sizeof(tmpbuf)];
					*--x = 0;
					line++;
					do *--x = ("0123456789")[line % 10]; while (line /= 10);
					copy(&buf, x, -1);
					copy(&buf, ": ", -1);
				}
				copy(&buf, "option error: assignment expected", -1);
				PUT(&buf, 0);
				return -1;
			}
			else if (c == '\n')
				line++;
			if (*s)
				s++;
		}
		if (!options || !*(s = options))
			break;
		options = 0;
	}
	if (!k)
		return 0;
	if (notice.type == SPECIAL && (!notice.verbose || !notice.item[NOTICE].data))
		return 0;
	if (notice.type != USAGE)
	{
		if (!notice.type)
			notice.type = SPECIAL;
		comment(&notice, &buf, NiL, 1, 0);
		comment(&notice, &buf, NiL, 0, 0);
		if (notice.item[PACKAGE].data)
		{
			copy(&tmp, "This software is part of the ", -1);
			expand(&notice, &notice.item[PACKAGE], &tmp);
			copy(&tmp, " package", -1);
			comment(&notice, &buf, BUF(&tmp), USE(&tmp), 0);
			if (notice.type >= OPEN)
			{
				copyright(&notice, &tmp);
				comment(&notice, &buf, BUF(&tmp), USE(&tmp), 0);
			}
		}
		if (notice.type == CPL)
		{
			if (notice.item[PACKAGE].data)
				copy(&tmp, "and", -1);
			else
			{
				copyright(&notice, &tmp);
				comment(&notice, &buf, BUF(&tmp), USE(&tmp), 0);
				copy(&tmp, "This software", -1);
			}
			copy(&tmp, " is licensed under the", -1);
			comment(&notice, &buf, BUF(&tmp), USE(&tmp), 0);
			copy(&tmp, "Common Public License", -1);
			if (notice.item[VERSION].data)
			{
				copy(&tmp, ", Version ", -1);
				expand(&notice, &notice.item[VERSION], &tmp);
			}
			copy(&tmp, " (the \"License\")", -1);
			comment(&notice, &buf, BUF(&tmp), USE(&tmp), 0);
			if (notice.item[CORPORATION].data)
			{
				copy(&tmp, "by ", -1);
				expand(&notice, &notice.item[CORPORATION], &tmp);
				copy(&tmp, " Corp. (\"", -1);
				expand(&notice, &notice.item[CORPORATION], &tmp);
				copy(&tmp, "\")", -1);
				comment(&notice, &buf, BUF(&tmp), USE(&tmp), 0);
			}
			else if (notice.item[COMPANY].data)
			{
				copy(&tmp, "by", -1);
				comment(&notice, &buf, BUF(&tmp), USE(&tmp), 0);
				expand(&notice, &notice.item[COMPANY], &tmp);
				comment(&notice, &buf, BUF(&tmp), USE(&tmp), 0);
			}
			COMMENT(&notice, &buf, "Any use, downloading, reproduction or distribution of this", 0);
			COMMENT(&notice, &buf, "software constitutes acceptance of the License.  A copy of", 0);
			COMMENT(&notice, &buf, "the License is available at", 0);
			comment(&notice, &buf, NiL, 0, 0);
			if (notice.item[URL].data)
			{
				expand(&notice, &notice.item[URL], &tmp);
				comment(&notice, &buf, BUF(&tmp), USE(&tmp), 0);
				if (notice.item[URLMD5].data)
				{
					copy(&tmp, "(with md5 checksum ", -1);
					expand(&notice, &notice.item[URLMD5], &tmp);
					copy(&tmp, ")", -1);
					comment(&notice, &buf, BUF(&tmp), USE(&tmp), 0);
				}
			}
			else
				COMMENT(&notice, &buf, "http://www.opensource.org/licenses/cpl", 0);
			comment(&notice, &buf, NiL, 0, 0);
		}
		else if (notice.type == OPEN)
		{
			copy(&tmp, notice.item[PACKAGE].data ? "and it" : "This software", -1);
			copy(&tmp, " may only be used by you under license from", -1);
			comment(&notice, &buf, BUF(&tmp), USE(&tmp), 0);
			if (notice.item[i = CORPORATION].data)
			{
				expand(&notice, &notice.item[i], &tmp);
				copy(&tmp, " Corp. (\"", -1);
				expand(&notice, &notice.item[i], &tmp);
				copy(&tmp, "\")", -1);
				comment(&notice, &buf, BUF(&tmp), USE(&tmp), 0);
			}
			else if (notice.item[i = COMPANY].data)
			{
				expand(&notice, &notice.item[i], &tmp);
				comment(&notice, &buf, BUF(&tmp), USE(&tmp), 0);
			}
			else
				i = -1;
			if (notice.item[URL].data)
			{
				COMMENT(&notice, &buf, "A copy of the Source Code Agreement is available", 0);
				copy(&tmp, "at the ", -1);
				if (i >= 0)
					expand(&notice, &notice.item[i], &tmp);
				copy(&tmp, " Internet web site URL", -1);
				comment(&notice, &buf, BUF(&tmp), USE(&tmp), 0);
				comment(&notice, &buf, NiL, 0, 0);
				expand(&notice, &notice.item[URL], &tmp);
				comment(&notice, &buf, BUF(&tmp), USE(&tmp), 0);
				if (notice.item[URLMD5].data)
				{
					copy(&tmp, "(with an md5 checksum of ", -1);
					expand(&notice, &notice.item[URLMD5], &tmp);
					copy(&tmp, ")", -1);
					comment(&notice, &buf, BUF(&tmp), USE(&tmp), 0);
				}
				comment(&notice, &buf, NiL, 0, 0);
			}
			COMMENT(&notice, &buf, "If you have copied or used this software without agreeing", 0);
			COMMENT(&notice, &buf, "to the terms of the license you are infringing on", 0);
			COMMENT(&notice, &buf, "the license and copyright and are violating", 0);
			if (i >= 0)
				expand(&notice, &notice.item[i], &tmp);
			copy(&tmp, "'s", -1);
			if (n >= COMLONG)
				comment(&notice, &buf, BUF(&tmp), USE(&tmp), 0);
			else
				PUT(&tmp, ' ');
			copy(&tmp, "intellectual property rights.", -1);
			comment(&notice, &buf, BUF(&tmp), USE(&tmp), 0);
			comment(&notice, &buf, NiL, 0, 0);
		}
		else if (notice.type == GPL)
		{
			if (!notice.item[PACKAGE].data)
			{
				copyright(&notice, &tmp);
				comment(&notice, &buf, BUF(&tmp), USE(&tmp), 0);
			}
			comment(&notice, &buf, NiL, 0, 0);
			COMMENT(&notice, &buf, "This is free software; you can redistribute it and/or", 0);
			COMMENT(&notice, &buf, "modify it under the terms of the GNU General Public License", 0);
			COMMENT(&notice, &buf, "as published by the Free Software Foundation;", 0);
			COMMENT(&notice, &buf, "either version 2, or (at your option) any later version.", 0);
			comment(&notice, &buf, NiL, 0, 0);
			COMMENT(&notice, &buf, "This software is distributed in the hope that it", 0);
			COMMENT(&notice, &buf, "will be useful, but WITHOUT ANY WARRANTY;", 0);
			COMMENT(&notice, &buf, "without even the implied warranty of MERCHANTABILITY", 0);
			COMMENT(&notice, &buf, "or FITNESS FOR A PARTICULAR PURPOSE.", 0);
			COMMENT(&notice, &buf, "See the GNU General Public License for more details.", 0);
			comment(&notice, &buf, NiL, 0, 0);
			COMMENT(&notice, &buf, "You should have received a copy of the", 0);
			COMMENT(&notice, &buf, "GNU General Public License", 0);
			COMMENT(&notice, &buf, "along with this software (see the file COPYING.)", 0);
			COMMENT(&notice, &buf, "If not, a copy is available at", 0);
			COMMENT(&notice, &buf, "http://www.gnu.org/copyleft/gpl.html", 0);
			comment(&notice, &buf, NiL, 0, 0);
		}
		else if (notice.type == FREE)
		{
			if (!notice.item[PACKAGE].data)
			{
				copyright(&notice, &tmp);
				comment(&notice, &buf, BUF(&tmp), USE(&tmp), 0);
			}
			comment(&notice, &buf, NiL, 0, 0);
			COMMENT(&notice, &buf, "Permission is hereby granted, free of charge,", 0);
			COMMENT(&notice, &buf, "to any person obtaining a copy of THIS SOFTWARE FILE", 0);
			COMMENT(&notice, &buf, "(the \"Software\"), to deal in the Software", 0);
			COMMENT(&notice, &buf, "without restriction, including without", 0);
			COMMENT(&notice, &buf, "limitation the rights to use, copy, modify,", 0);
			COMMENT(&notice, &buf, "merge, publish, distribute, and/or", 0);
			COMMENT(&notice, &buf, "sell copies of the Software, and to permit", 0);
			COMMENT(&notice, &buf, "persons to whom the Software is furnished", 0);
			COMMENT(&notice, &buf, "to do so, subject to the following disclaimer:", 0);
			comment(&notice, &buf, NiL, 0, 0);
			copy(&tmp, "THIS SOFTWARE IS PROVIDED ", -1);
			if (notice.item[i = CORPORATION].data || notice.item[i = COMPANY].data)
			{
				copy(&tmp, "BY ", -1);
				expand(&notice, &notice.item[i], &tmp);
			}
			comment(&notice, &buf, BUF(&tmp), USE(&tmp), 0);
			COMMENT(&notice, &buf, "``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,", 0);
			COMMENT(&notice, &buf, "INCLUDING, BUT NOT LIMITED TO, THE IMPLIED", 0);
			COMMENT(&notice, &buf, "WARRANTIES OF MERCHANTABILITY AND FITNESS", 0);
			COMMENT(&notice, &buf, "FOR A PARTICULAR PURPOSE ARE DISCLAIMED.", 0);
			copy(&tmp, "IN NO EVENT SHALL ", -1);
			if (notice.item[i = CORPORATION].data || notice.item[i = COMPANY].data)
				expand(&notice, &notice.item[i], &tmp);
			else
				copy(&tmp, " THE AUTHOR(S)", -1);
			comment(&notice, &buf, BUF(&tmp), USE(&tmp), 0);
			COMMENT(&notice, &buf, "BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,", 0);
			COMMENT(&notice, &buf, "SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES", 0);
			COMMENT(&notice, &buf, "(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT", 0);
			COMMENT(&notice, &buf, "OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,", 0);
			COMMENT(&notice, &buf, "DATA, OR PROFITS; OR BUSINESS INTERRUPTION)", 0);
			COMMENT(&notice, &buf, "HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,", 0);
			COMMENT(&notice, &buf, "WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT", 0);
			COMMENT(&notice, &buf, "(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING", 0);
			COMMENT(&notice, &buf, "IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,", 0);
			COMMENT(&notice, &buf, "EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.", 0);
			comment(&notice, &buf, NiL, 0, 0);
		}
		else
		{
			if (notice.type == PROPRIETARY)
			{
				if (notice.item[i = CORPORATION].data || notice.item[i = COMPANY].data)
				{
					expand(&notice, &notice.item[i], &tmp);
					copy(&tmp, " - ", -1);
				}
				else
					i = -1;
				copy(&tmp, "Proprietary", -1);
				comment(&notice, &buf, BUF(&tmp), USE(&tmp), 1);
				comment(&notice, &buf, NiL, 0, 0);
				if (notice.item[URL].data)
				{
					copy(&tmp, "This is proprietary source code", -1);
					if (notice.item[CORPORATION].data || notice.item[COMPANY].data)
						copy(&tmp, " licensed by", -1);
					comment(&notice, &buf, BUF(&tmp), USE(&tmp), 1);
					if (notice.item[CORPORATION].data)
					{
						expand(&notice, &notice.item[CORPORATION], &tmp);
						copy(&tmp, " Corp.", -1);
						comment(&notice, &buf, BUF(&tmp), USE(&tmp), 1);
					}
					else if (notice.item[COMPANY].data)
					{
						expand(&notice, &notice.item[COMPANY], &tmp);
						comment(&notice, &buf, BUF(&tmp), USE(&tmp), 1);
					}
				}
				else
				{
					copy(&tmp, "This is unpublished proprietary source code", -1);
					if (i >= 0)
						copy(&tmp, " of", -1);
					comment(&notice, &buf, BUF(&tmp), USE(&tmp), 1);
					if (notice.item[CORPORATION].data)
						expand(&notice, &notice.item[CORPORATION], &tmp);
					if (notice.item[COMPANY].data)
					{
						if (SIZ(&tmp))
							PUT(&tmp, ' ');
						expand(&notice, &notice.item[COMPANY], &tmp);
					}
					if (SIZ(&tmp))
						comment(&notice, &buf, BUF(&tmp), USE(&tmp), 1);
					COMMENT(&notice, &buf, "and is not to be disclosed or used except in", 1);
					COMMENT(&notice, &buf, "accordance with applicable agreements", 1);
				}
				comment(&notice, &buf, NiL, 0, 0);
			}
			else if (notice.type == NONEXCLUSIVE)
			{
				COMMENT(&notice, &buf, "For nonexclusive individual use", 1);
				comment(&notice, &buf, NiL, 0, 0);
			}
			else if (notice.type == NONCOMMERCIAL)
			{
				COMMENT(&notice, &buf, "For noncommercial use", 1);
				comment(&notice, &buf, NiL, 0, 0);
			}
			copyright(&notice, &tmp);
			comment(&notice, &buf, BUF(&tmp), USE(&tmp), 0);
			if (notice.type <= PROPRIETARY)
			{
				if (!notice.item[URL].data)
					COMMENT(&notice, &buf, "Unpublished & Not for Publication", 0);
				COMMENT(&notice, &buf, "All Rights Reserved", 0);
			}
			comment(&notice, &buf, NiL, 0, 0);
			if (notice.item[URL].data)
			{
				copy(&tmp, "This software is licensed", -1);
				if (notice.item[CORPORATION].data)
				{
					copy(&tmp, " by", -1);
					if (notice.item[CORPORATION].size >= (COMLONG - 6))
						comment(&notice, &buf, BUF(&tmp), USE(&tmp), 0);
					else
						PUT(&tmp, ' ');
					expand(&notice, &notice.item[CORPORATION], &tmp);
					PUT(&tmp, ' ');
					copy(&tmp, "Corp.", -1);
				}
				else if (notice.item[COMPANY].data)
				{
					copy(&tmp, " by", -1);
					if (notice.item[COMPANY].size >= COMLONG)
						comment(&notice, &buf, BUF(&tmp), USE(&tmp), 0);
					else
						PUT(&tmp, ' ');
					expand(&notice, &notice.item[COMPANY], &tmp);
				}
				comment(&notice, &buf, BUF(&tmp), USE(&tmp), 0);
				COMMENT(&notice, &buf, "under the terms and conditions of the license in", 0);
				expand(&notice, &notice.item[URL], &tmp);
				comment(&notice, &buf, BUF(&tmp), USE(&tmp), 0);
				if (notice.item[URLMD5].data)
				{
					copy(&tmp, "(with an md5 checksum of ", -1);
					expand(&notice, &notice.item[URLMD5], &tmp);
					copy(&tmp, ")", -1);
					comment(&notice, &buf, BUF(&tmp), USE(&tmp), 0);
				}
				comment(&notice, &buf, NiL, 0, 0);
			}
			else if (notice.type == PROPRIETARY)
			{
				COMMENT(&notice, &buf, "The copyright notice above does not evidence any", 0);
				COMMENT(&notice, &buf, "actual or intended publication of such source code", 0);
				comment(&notice, &buf, NiL, 0, 0);
			}
		}
		if (v = notice.item[NOTICE].data)
		{
			x = v + notice.item[NOTICE].size;
			if (*v == '\n')
				v++;
			item.quote = notice.item[NOTICE].quote;
			do
			{
				for (item.data = v; v < x && *v != '\n'; v++);
				item.size = v - item.data;
				expand(&notice, &item, &tmp);
				comment(&notice, &buf, BUF(&tmp), USE(&tmp), -1);
			} while (v++ < x);
			if (item.size)
				comment(&notice, &buf, NiL, 0, 0);
		}
		if (notice.item[ORGANIZATION].data)
		{
			expand(&notice, &notice.item[ORGANIZATION], &tmp);
			comment(&notice, &buf, BUF(&tmp), USE(&tmp), 0);
			if (notice.item[CORPORATION].data)
				expand(&notice, &notice.item[CORPORATION], &tmp);
			if (notice.item[COMPANY].data)
			{
				if (SIZ(&tmp))
					PUT(&tmp, ' ');
				expand(&notice, &notice.item[COMPANY], &tmp);
			}
			if (SIZ(&tmp))
				comment(&notice, &buf, BUF(&tmp), USE(&tmp), 0);
			if (notice.item[LOCATION].data)
			{
				expand(&notice, &notice.item[LOCATION], &tmp);
				comment(&notice, &buf, BUF(&tmp), USE(&tmp), 0);
			}
			comment(&notice, &buf, NiL, 0, 0);
		}
	}
	if (v = notice.item[AUTHOR].data)
	{
		x = v + notice.item[AUTHOR].size;
		q = (x - v) == 1 && (*v == '*' || *v == '-');
		k = q && notice.type != USAGE ? -1 : 0;
		for (;;)
		{
			if (!q)
			{
				while (v < x && (*v == ' ' || *v == '\t' || *v == '\r' || *v == '\n' || *v == ',' || *v == '+'))
					v++;
				if (v >= x)
					break;
				item.data = v;
				while (v < x && *v != ',' && *v != '+' && *v++ != '>');
				item.size = v - item.data;
				item.quote = notice.item[AUTHOR].quote;
			}
			h = 0;
			for (i = 0; i < notice.ids; i++)
				if (q || item.size == notice.id[i].name.size && !strncmp(item.data, notice.id[i].name.data, item.size))
				{
					h = 1;
					if (notice.type == USAGE)
					{
						copy(&buf, "[-author?", -1);
						expand(&notice, &notice.id[i].value, &buf);
						PUT(&buf, ']');
					}
					else
					{
						if (k < 0)
						{
							COMMENT(&notice, &buf, "CONTRIBUTORS", 0);
							comment(&notice, &buf, NiL, 0, 0);
						}
						k = 1;
						expand(&notice, &notice.id[i].value, &tmp);
						comment(&notice, &buf, BUF(&tmp), USE(&tmp), 0);
					}
					if (!q)
						break;
				}
			if (q)
				break;
			if (!h)
			{
				if (notice.type == USAGE)
				{
					copy(&buf, "[-author?", -1);
					expand(&notice, &item, &buf);
					PUT(&buf, ']');
				}
				else
				{
					if (k < 0)
					{
						COMMENT(&notice, &buf, "CONTRIBUTORS", 0);
						comment(&notice, &buf, NiL, 0, 0);
					}
					k = 1;
					expand(&notice, &item, &tmp);
					comment(&notice, &buf, BUF(&tmp), USE(&tmp), 0);
				}
			}
		}
		if (k > 0)
			comment(&notice, &buf, NiL, 0, 0);
	}
	if (notice.type == USAGE)
	{
		copy(&buf, "[-copyright?", -1);
		copyright(&notice, &buf);
		PUT(&buf, ']');
		if (notice.item[URL].data)
		{
			copy(&buf, "[-license?", -1);
			expand(&notice, &notice.item[URL], &buf);
			PUT(&buf, ']');
		}
		PUT(&buf, '\n');
	}
	else
		comment(&notice, &buf, NiL, -1, 0);
	return END(&buf);
}
