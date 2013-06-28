/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1985-2013 AT&T Intellectual Property          *
*                      and is licensed under the                       *
*                 Eclipse Public License, Version 1.0                  *
*                    by AT&T Intellectual Property                     *
*                                                                      *
*                A copy of the License is available at                 *
*          http://www.eclipse.org/org/documents/epl-v10.html           *
*         (with md5 checksum b35adb5213ca9657e911e9befb180842)         *
*                                                                      *
*              Information and Software Systems Research               *
*                            AT&T Research                             *
*                           Florham Park NJ                            *
*                                                                      *
*                 Glenn Fowler <gsf@research.att.com>                  *
*                  David Korn <dgk@research.att.com>                   *
*                     Phong Vo <phongvo@gmail.com>                     *
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Research
 *
 * path name canonicalization -- preserves the logical view
 *
 *	remove redundant .'s and /'s
 *	move ..'s to the front
 *	/.. preserved (for pdu and newcastle hacks)
 *	FS_3D handles ...
 *	if (flags&PATH_PHYSICAL) then symlinks resolved at each component
 *	if (flags&PATH_DOTDOT) then each .. checked for access
 *	if (flags&PATH_EXISTS) then path must exist at each component
 *	if (flags&PATH_VERIFIED(n)) then first n chars of path exist
 * 
 * longer pathname possible if (flags&PATH_PHYSICAL) or FS_3D ... involved
 * 0 returned on error and if (flags&(PATH_DOTDOT|PATH_EXISTS)) then canon
 * will contain the components following the failure point
 *
 * pathcanon() and pathdev() return pointer to trailing 0 in canon
 * pathdev() handles ast specific /dev/ and /proc/ special files
 * see pathopen() for the api that ties it all together
 */

#define _AST_API_H	1

#include <ast.h>
#include <ls.h>
#include <fs3d.h>
#include <error.h>

char*
pathcanon(char* path, int flags)
{
	return pathcanon_20100601(path, PATH_MAX, flags);
}

#undef	_AST_API_H

#include <ast_api.h>

char*
pathcanon_20100601(char* path, size_t size, int flags)
{
	return pathdev(path, path, size, flags, NiL);
}

#define NEXT(s,n) \
	do { \
		for (s += n;; s++) \
			if (*s == '.' && *(s + 1) == '/') \
				s++; \
			else if (*s != '/') \
				break; \
	} while (0)

#define DIGITS(s,n) \
	do { \
		for (n = 0; *s >= '0' && *s <= '9'; s++) \
			n = n * 10 + (*s - '0'); \
	} while (0)

char*
pathdev(const char* path, char* canon, size_t size, int flags, Pathdev_t* dev)
{
	register char*	p;
	register char*	r;
	register char*	s;
	register char*	t;
	register int	dots;
	char*		v;
	int		n;
	int		loop;
	int		oerrno;
	Pathdev_t	nodev;
#if defined(FS_3D)
	long		visits = 0;
#endif

	oerrno = errno;

	/* lazy here -- we will never produce a path longer than strlen(path)+1 */

	if (canon)
	{
		if (!size)
			size = strlen(path) + 1;
		else if (size <= strlen(path))
		{
			errno = EINVAL;
			return 0;
		}
	}
	else if (!size)
		size = strlen(path);
	if (dev)
		dev->path.offset = 0;
	else
		dev = &nodev;
	if (*path == '/')
	{
		if (path[1] == '/' && *astconf("PATH_LEADING_SLASHES", NiL, NiL) == '1')
			do path++; while (path[0] == '/' && path[1] == '/');
		if (!path[1])
		{
			if (flags & PATH_DEV)
				return 0;
			if (canon)
			{
				canon[0] = '/';
				canon[1] = 0;
			}
			else
				canon = (char*)path;
			return (char*)canon + 1;
		}
		r = 0;
		p = s = (char*)path;
		if (size > 16 && s[1] == 'p' && s[2] == 'r' && s[3] == 'o' && s[4] == 'c' && s[5] == '/')
		{
			NEXT(s, 6);
			if (s[0] == 's' && s[1] == 'e' && s[2] == 'l' && s[3] == 'f')
			{
				s += 4;
				dev->pid = -1;
			}
			else
				DIGITS(s, dev->pid);
			if (s[0] == '/')
			{
				NEXT(s, 1);
				if (s[0] == 'f' && s[1] == 'd' && s[2] == '/')
				{
					NEXT(s, 3);
					DIGITS(s, dev->fd);
					if (*s == '/' || *s == 0)
					{
						NEXT(s, 0);
						r = s;
						dev->prot.offset = 0;
					}
				}
			}
		}
		else if (size > 8 && s[1] == 'd' && s[2] == 'e' && s[3] == 'v' && s[4] == '/')
		{
			NEXT(s, 5);
			if (s[0] == 'f' && s[1] == 'd' && s[2] == '/')
			{
				NEXT(s, 3);
				DIGITS(s, dev->fd);
				if (*s == '/' || *s == 0)
				{
					NEXT(s, 0);
					r = s;
					dev->prot.offset = 0;
					dev->pid = -1;
				}
			}
			else if (s[0] == 's' && s[1] == 'c' && s[2] == 't' && s[3] == 'p' && s[4] == '/' && (n = 5) ||
				 s[0] == 't' && s[1] == 'c' && s[2] == 'p' && s[3] == '/' && (n = 4) ||
				 s[0] == 'u' && s[1] == 'd' && s[2] == 'p' && s[3] == '/' && (n = 4))
			{
				dev->prot.offset = canon ? 5 : (s - p);
				dev->prot.size = n - 1;
				NEXT(s, n);
				if (t = strchr(s, '/'))
				{
					dev->host.offset = canon ? (dev->prot.offset + n) : (s - p);
					dev->host.size = t - s;
					NEXT(t, 0);
					if (s = strchr(t, '/'))
						NEXT(s, 0);
					else
						s = t + strlen(t);
					dev->port.offset = canon ? (dev->host.offset + dev->host.size + 1) : (s - p);
					dev->port.size = s - t;
					dev->fd = -1;
					dev->pid = -1;
					r = s;
				}
			}
			else if (s[0] == 's' && s[1] == 't' && s[2] == 'd')
			{
				if (s[3] == 'e' && s[4] == 'r' && s[5] == 'r' && s[6] == 0)
				{
					r = s + 6;
					dev->fd = 2;
					dev->prot.offset = 0;
				}
				else if (s[3] == 'i' && s[4] == 'n' && s[5] == 0)
				{
					r = s + 5;
					dev->fd = 0;
					dev->prot.offset = 0;
				}
				else if (s[3] == 'o' && s[4] == 'u' && s[5] == 't' && s[6] == 0)
				{
					r = s + 6;
					dev->fd = 1;
					dev->prot.offset = 0;
				}
			}
		}
		if (r)
		{
			if (!(t = canon))
			{
				dev->path.offset = r - p;
				return r;
			}
			s = p;
			while (s <= r)
				if ((*t++ = *s++) == '/')
					while (*s == '/' || *s == '.' && *(s + 1) == '/')
						s++;
			if (!*(t - 1))
				t--;
			dev->path.offset = t - canon;
			if (!*t)
				return t;
			path = (const char*)(canon = t);
			if (s != t)
				while (*t++ = *s++);
		}
		else if (flags & PATH_DEV)
			return 0;
	}
	else if (flags & PATH_DEV)
		return 0;
	if (!canon)
		return (char*)path;
	dots = loop = 0;
	p = t = r = canon;
	v = t + ((flags >> 5) & 01777);
	s = (char*)path;
	for (;;)
		switch (*t++ = *s++)
		{
		case '.':
			dots++;
			break;
		case 0:
			s--;
			/*FALLTHROUGH*/
		case '/':
			while (*s == '/')
				s++;
			switch (dots)
			{
			case 1:
				t -= 2;
				break;
			case 2:
				if ((flags & (PATH_DOTDOT|PATH_EXISTS)) == PATH_DOTDOT && (t - 2) >= v)
				{
					struct stat	st;

					*(t - 2) = 0;
					if (stat(canon, &st))
					{
						strcpy(canon, s);
						return 0;
					}
					*(t - 2) = '.';
				}
#if PRESERVE_TRAILING_SLASH
				if (t - 5 < r)
					r = t;
#else
				if (t - 5 < r)
				{
					if (t - 4 == r)
						t = r + 1;
					else
						r = t;
				}
#endif
				else
					for (t -= 5; t > r && *(t - 1) != '/'; t--);
				break;
			case 3:
#if defined(FS_3D)
				{
					char*		x;
					char*		o;
					int		c;

					o = t;
					if ((t -= 5) <= canon)
						t = canon + 1;
					c = *t;
					*t = 0;
					if (x = pathnext(canon, s - (*s != 0), &visits))
					{
						r = canon;
						if (t == r + 1)
							x = r;
						v = s = t = x;
					}
					else
					{
						*t = c;
						t = o;
					}
				}
#else
				r = t;
#endif
				break;
			default:
				if ((flags & PATH_PHYSICAL) && loop < 32 && (t - 1) > canon)
				{
					int	c;
					char	buf[PATH_MAX];

					c = *(t - 1);
					*(t - 1) = 0;
					dots = pathgetlink(canon, buf, sizeof(buf));
					*(t - 1) = c;
					if (dots > 0)
					{
						loop++;
						strcpy(buf + dots, s - (*s != 0));
						if (*buf == '/')
							p = r = canon;
						v = s = t = p;
						strcpy(p, buf);
					}
					else if (dots < 0 && errno == ENOENT)
					{
						if (flags & PATH_EXISTS)
						{
							strcpy(canon, s);
							return 0;
						}
						flags &= ~(PATH_PHYSICAL|PATH_DOTDOT);
					}
					dots = 4;
				}
				break;
			}
			if (dots >= 4 && (flags & PATH_EXISTS) && (t - 1) >= v && (t > canon + 1 || t > canon && *(t - 1) && *(t - 1) != '/'))
			{
				struct stat	st;

				*(t - 1) = 0;
				if (stat(canon, &st))
				{
					strcpy(canon, s);
					return 0;
				}
				v = t;
				if (*s)
					*(t - 1) = '/';
			}
			if (!*s)
			{
				if (t > canon && !*(t - 1))
					t--;
				if (t == canon)
					*t++ = '.';
#if DONT_PRESERVE_TRAILING_SLASH
				else if (t > canon + 1 && *(t - 1) == '/')
					t--;
#else
				else if ((s <= canon || *(s - 1) != '/') && t > canon + 1 && *(t - 1) == '/')
					t--;
#endif
				*t = 0;
				errno = oerrno;
				return t;
			}
			dots = 0;
			p = t;
			break;
		default:
			dots = 4;
			break;
		}
}
