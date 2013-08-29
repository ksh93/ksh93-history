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
 *	if (flags&PATH_ABSOLUTE) then pwd prepended to relative paths
 *	if (flags&PATH_PHYSICAL) then symlinks resolved at each component
 *	if (flags&PATH_DOTDOT) then each .. checked for access
 *	if (flags&PATH_EXISTS) then path must exist at each component
 *	if (flags&PATH_VERIFIED(n)) then first n chars of path exist
 * 
 * longer pathname possible if (flags&PATH_PHYSICAL) involved
 * 0 returned on error and if (flags&(PATH_DOTDOT|PATH_EXISTS)) then canon
 * will contain the components following the failure point
 *
 * pathcanon() and pathdev() return pointer to trailing 0 in canon
 * pathdev() handles ast specific /dev/ and /proc/ special files
 * pathdev(PATH_DEV) returns 0 if path is not a valid special file
 * pathdev(PATH_AUX) Pathdev_t.flags|=PATH_AUX if AUX fd was created
 * see pathopen() for the api that ties it all together
 */

#if !_BLD_3d

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
	return pathdev(AT_FDCWD, path, path, size, flags|PATH_CANON, NiL);
}

#endif

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
		if (*s >= '0' && *s <= '9') \
			for (n = 0; *s >= '0' && *s <= '9'; s++) \
				n = n * 10 + (*s - '0'); \
	} while (0)

/*
 * check for ast pseudo dev/attribute paths and optionally canonicalize
 *
 * if (dev.oflags & O_INTERCEPT) on return then dev.fd must be closed
 * by the caller after using it
 */

char*
pathdev(int dfd, const char* path, char* canon, size_t size, int flags, Pathdev_t* dev)
{
	register char*	p;
	register char*	r;
	register char*	s;
	register char*	t;
	register int	dots;
	char*		v;
	char*		x;
	char*		z;
	char*		a;
	char*		b;
	int		c;
	int		n;
	int		loop;
	int		oerrno;
	int		inplace;
	Pathdev_t	nodev;

	static int	path_one_head_slash = -1;

	oerrno = errno;
	if (!dev)
		dev = &nodev;
	dev->oflags = 0;
	dev->path.offset = 0;

	/* lazy here -- we will never (modulo PATH_ABSOLUTE) produce a path longer than strlen(path)+1 */

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
	p = (char*)path;
	inplace = p == canon;
 again:
	r = 0;
	if (path[0] == '/')
	{
		for (s = (char*)path; s[1] == '/'; s++);
		if (!s[1])
		{
			if (flags & PATH_DEV)
			{
				errno = ENODEV;
				return 0;
			}
			if (canon)
			{
				s = canon;
				*s++ = '/';
				if (path[1] == '/' && !(flags & PATH_DROP_HEAD_SLASH2))
					*s++ = '/';
				*s = 0;
			}
			else
				s++;
			return s;
		}
		if (size > 16 && s[1] == 'p' && s[2] == 'r' && s[3] == 'o' && s[4] == 'c' && s[5] == '/')
		{
			NEXT(s, 6);
			if (s[0] == 's' && s[1] == 'e' && s[2] == 'l' && s[3] == 'f')
			{
				s += 4;
				dev->pid = -1;
			}
			else
#ifdef _fd_pid_dir_fmt
				DIGITS(s, dev->pid);
#else
			{
				errno = ENOENT;
				return 0;
			}
#endif
			if (s[0] == '/')
			{
				NEXT(s, 1);
				if (s[0] == 'f' && s[1] == 'd' && s[2] == '/')
				{
					NEXT(s, 3);
					DIGITS(s, dev->fd);
					if (dev->fd >= 0 && (*s == '/' || *s == 0))
					{
						NEXT(s, 0);
						r = s;
						dev->prot.offset = 0;
					}
				}
			}
		}
		else if (size > 7 && s[1] == 'd' && s[2] == 'e' && s[3] == 'v' && s[4] == '/')
		{
			NEXT(s, 5);
			if (s[0] == 'f')
			{
				if (s[1] == 'd' && (s[2] == '/' || s[2] == 0))
				{
					NEXT(s, 2);
					if (*s)
					{
						DIGITS(s, dev->fd);
						if (dev->fd >= 0 && (*s == '/' || *s == 0))
						{
							NEXT(s, 0);
							r = s;
							dev->pid = -1;
						}
					}
					else if (flags & PATH_DEV)
					{
						r = s;
						dev->fd = AT_FDCWD;
						dev->pid = -1;
					}
				}
				else if (s[1] == 'i' && s[2] == 'l' && s[3] == 'e' && (s[4] == '/' || s[4] == 0))
				{
					NEXT(s, 4);
					for (;;)
					{
						if (s[0] == 'a' && s[1] == 's' && s[2] == 'y' && s[3] == 'n' && s[4] == 'c' && (s[5] == ',' || s[5] == ':'))
						{
#ifdef O_ASYNC
							s += 5;
							dev->oflags |= O_ASYNC;
#else
							errno = ENXIO;
							return 0;
#endif
						}
						else if (s[0] == 'd' && s[1] == 'i' && s[2] == 'r' && s[3] == 'e' && s[4] == 'c' && s[5] == 't')
						{
							if (s[6] == ',' || s[6] == ':')
							{
#ifdef O_DIRECT
								s += 6;
								dev->oflags |= O_DIRECT;
#else
								errno = ENXIO;
								return 0;
#endif
							}
							else if (s[6] == 'o' && s[7] == 'r' && s[8] == 'y' && (s[9] == ',' || s[9] == ':'))
							{
#ifdef O_DIRECTORY
								s += 9;
								dev->oflags |= O_DIRECTORY;
#else
								errno = ENXIO;
								return 0;
#endif
							}
							else
							{
								errno = EINVAL;
								return 0;
							}
						}
						else if (s[0] == 'n' && s[1] == 'o' && s[2] == 'n' && s[3] == 'b' && s[4] == 'l' && s[5] == 'o' && s[6] == 'c' && s[7] == 'k' && (s[8] == ',' || s[8] == ':'))
						{
#ifdef O_NONBLOCK
							s += 8;
							dev->oflags |= O_NONBLOCK;
#else
							errno = ENXIO;
							return 0;
#endif
						}
						else if (s[0] == 's' && s[1] == 'y' && s[2] == 'n' && s[3] == 'c' && (s[4] == ',' || s[4] == ':'))
						{
#ifdef O_ASYNC
							s += 4;
							dev->oflags |= O_ASYNC;
#else
							errno = ENXIO;
							return 0;
#endif
						}
						else
						{
							errno = EINVAL;
							return 0;
						}
						if (s[0] == ':')
						{
							if (*++s)
							{
								flags &= ~PATH_DEV;
								path = (const char*)s;
								if (!canon)
								{
									dev->path.offset = s - p;
									dev->fd = -1;
									dev->pid = -1;
									dev->prot.offset = 0;
								}
								goto again;
							}
							break;
						}
						s++;
					}
					if (!s[0] && (flags & PATH_DEV))
					{
						r = s;
						dev->fd = AT_FDCWD;
						dev->pid = -1;
					}
				}
			}
			else if (s[0] == 's' && s[1] == 'c' && s[2] == 't' && s[3] == 'p' && (s[4] == '/' || s[4] == 0) && (n = 4) ||
				 s[0] == 't' && s[1] == 'c' && s[2] == 'p' && (s[3] == '/' || s[3] == 0) && (n = 3) ||
				 s[0] == 'u' && s[1] == 'd' && s[2] == 'p' && (s[3] == '/' || s[3] == 0) && (n = 3))
			{
				dev->prot.offset = canon ? 5 : (s - p);
				dev->prot.size = n;
				NEXT(s, n);
				if (*s)
				{
					dev->host.offset = canon ? (dev->prot.offset + n + 1) : (s - p);
					if (t = strchr(s, '/'))
					{
						dev->host.size = t - s;
						NEXT(t, 0);
					}
					else
					{
						t = s + strlen(s);
						dev->host.size = t - s;
					}
					if (*t)
					{
						dev->port.offset = canon ? (dev->host.offset + dev->host.size + 1) : (t - p);
						if (s = strchr(t, '/'))
						{
							dev->port.size = s - t;
							NEXT(s, 0);
						}
						else
						{
							s = t + strlen(t);
							dev->port.size = s - t;
						}
						dev->fd = -1;
						dev->pid = -1;
						r = s;
					}
					else if (flags & PATH_DEV)
					{
						dev->port.offset = 0;
						dev->fd = -1;
						dev->pid = -1;
						r = t;
					}
				}
				else if (flags & PATH_DEV)
				{
					dev->host.offset = 0;
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
					dev->pid = -1;
				}
				else if (s[3] == 'i' && s[4] == 'n' && s[5] == 0)
				{
					r = s + 5;
					dev->fd = 0;
					dev->pid = -1;
				}
				else if (s[3] == 'o' && s[4] == 'u' && s[5] == 't' && s[6] == 0)
				{
					r = s + 6;
					dev->fd = 1;
					dev->pid = -1;
				}
			}
		}
	}
	else
		dev->fd = -1;
	if (r)
	{
		if (!(t = canon))
		{
			dev->path.offset = r - p;
			return p + strlen(p);
		}
		for (s = p; s < r && (*t = *s++); t++);
		dev->path.offset = t - canon;
		if (!*t && (!(flags & PATH_PHYSICAL) || dev->fd < 0))
			return t;
	}
	else if (!canon)
	{
		for (t = (char*)path; r = strchr(t, '@'); t = r + 1)
			if ((r - t) >= 2 && r[-2] == '/' && r[-1] == '/' && r[1] == '/' && r[2] == '/')
			{
#if O_XATTR
				char	buf[2*PATH_MAX];

				t = r - 2;
				r = (char*)path;
				if (t == r)
				{
					buf[0] = *path == '/' ? '/' : '.';
					buf[1] = 0;
				}
				else
				{
					memcpy(buf, r, t - r);
					for (r = buf + (t - r); r > buf && *(r - 1) == '/'; r--);
					*r = 0;
				}
				if ((dev->fd = openat(dfd, buf, O_INTERCEPT|O_RDONLY|O_CLOEXEC|dev->oflags)) < 0)
					r = 0;
				else if ((n = openat(dev->fd, ".", O_INTERCEPT|O_RDONLY|O_XATTR)) < 0)
				{
					r = 0;
					close(dev->fd);
					dev->fd = -1;
				}
				else
				{
					dev->oflags |= O_INTERCEPT;
					close(dev->fd);
					if (dev == &nodev)
						close(n);
					dev->fd = n;
					for (r = t + 3; *r == '/'; r++);
					dev->pid = -1;
					dev->path.offset = r - (char*)path;
					return r + strlen(r);
				}
				break;
#else
				errno = ENOTDIR;
				return 0;
#endif
			}
		if (flags & PATH_DEV)
		{
			errno = ENODEV;
			return 0;
		}
		r = 0;
	}
	if (!canon)
		return p + strlen(p);
	dots = loop = 0;
	p = canon;
	if (r)
		s = r;
	else
	{
		s = (char*)path;
		t = p;
	}
	b = s;
	r = t;
	v = p + PATH_GET_VERIFIED(flags);
	if ((flags & PATH_ABSOLUTE) && dev->fd < 0 && *s != '/' || (flags & PATH_PHYSICAL) && dev->fd >= 0)
	{
		if (inplace)
		{
			/* XXX -- TODO -- avoid fmtbuf() by sliding this part to the right and adjusting fgetcwd() size below */
			n = strlen(path);
			x = fmtbuf(n + 1);
			memcpy(x, path, n + 1);
		}
		else
			x = (char*)path;
		z = dev->fd >= 0 ? p : t;
		if (fgetcwd(dev->fd >= 0 ? dev->fd : AT_FDCWD, z, size - strlen(r)))
		{
			t = r = z;
			t += strlen(t);
			*t++ = '/';
			s = x;
			v = t;
			if (dev->fd >= 0)
			{
				s += dev->path.offset;
				dev->path.offset = 0;
			}
		}
		else if (*s != '/')
			return 0;
	}
	if (!(flags & PATH_DROP_HEAD_SLASH2) && s[0] == '/' && s[1] == '/')
	{
		for (a = s + 2; *a == '/'; a++);
		if (a[0] == '@' && a[1] == '/' && a[2] == '/')
		{
			if ((a - s) >= 4)
			{
				*t++ = *s++;
				*t++ = *s++;
			}
		}
		else
			*t++ = *s++;
	}
	x = 0;
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
			for (a = s; *s == '/'; s++);
			switch (dots)
			{
			case 1:
				if (t - 2 >= r)
					t -= 2;
				break;
			case 2:
				if ((flags & (PATH_DOTDOT|PATH_EXISTS)) == PATH_DOTDOT && (t - 2) >= v)
				{
					struct stat	st;

					*(t - 2) = 0;
					if (fstatat(dfd, canon, &st, 0))
					{
						if (inplace)
							memmove(canon, s, strlen(s) + 1);
						else
							strcpy(canon, s);
						return 0;
					}
					*(t - 2) = '.';
				}
				if (t - 5 < r)
				{
					if (t - 4 == r)
						t = r + 1;
					else
						r = t;
				}
				else
					for (t -= 5; t > r && *(t - 1) != '/'; t--);
				break;
			case 3:
				r = t;
				break;
			default:
				if ((flags & PATH_PHYSICAL) && loop < 32 && (t - 1) > canon)
				{
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
							if (inplace)
								memmove(canon, s, strlen(s) + 1);
							else
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
				if (fstatat(dfd, canon, &st, 0))
				{
					if (inplace)
						memmove(canon, s, strlen(s) + 1);
					else
						strcpy(canon, s);
					return 0;
				}
				v = t;
				if (*s)
					*(t - 1) = '/';
			}
			switch (*s)
			{
			case 0:
				if (t > canon && !*(t - 1))
					t--;
				if (t == canon)
					*t++ = '.';
				else if (((flags & PATH_DROP_TAIL_SLASH) || s <= b || *(s - 1) != '/') && t > r + 1 && *(t - 1) == '/')
					t--;
				*t = 0;
				errno = oerrno;
				if (x && !(flags & PATH_CANON))
				{
#if O_XATTR
					r = x - 5;
					if (r == canon)
						r++;
					*r = 0;
					dev->fd = openat(dfd, canon, O_INTERCEPT|O_RDONLY|O_CLOEXEC|dev->oflags);
					*r = '/';
					if (dev->fd < 0)
						t = 0;
					else if ((n = openat(dev->fd, ".", O_INTERCEPT|O_RDONLY|O_XATTR)) < 0)
					{
						close(dev->fd);
						dev->fd = -1;
						t = 0;
					}
					else
					{
						dev->oflags |= O_INTERCEPT;
						close(dev->fd);
						if (dev == &nodev)
							close(n);
						dev->fd = n;
						dev->pid = -1;
						dev->path.offset = x - canon;
					}
#else
					errno = ENOTDIR;
					t = 0;
#endif
				}
				return t;
			case '@':
				if (!x && s > a && s[1] == '/' && s[2] == '/')
				{
					for (s += 3; *s == '/'; s++);
					if (dots == 1 && t == r)
					{
						*t++ = '.';
						*t++ = '/';
					}
					*t++ = '/';
					*t++ = '@';
					*t++ = '/';
					*t++ = '/';
					r = x = t;
				}
				break;
			}
			dots = 0;
			p = t;
			break;
		default:
			dots = 4;
			break;
		}
 	return 0;
}
