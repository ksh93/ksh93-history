/*
 * CDE - Common Desktop Environment
 *
 * Copyright (c) 1993-2012, The Open Group. All rights reserved.
 *
 * These libraries and programs are free software; you can
 * redistribute them and/or modify them under the terms of the GNU
 * Lesser General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * These libraries and programs are distributed in the hope that
 * they will be useful, but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with these librararies and programs; if not, write
 * to the Free Software Foundation, Inc., 51 Franklin Street, Fifth
 * Floor, Boston, MA 02110-1301 USA
 */
/***************************************************************
*                                                              *
*                      AT&T - PROPRIETARY                      *
*                                                              *
*         THIS IS PROPRIETARY SOURCE CODE LICENSED BY          *
*                          AT&T CORP.                          *
*                                                              *
*                Copyright (c) 1995 AT&T Corp.                 *
*                     All Rights Reserved                      *
*                                                              *
*           This software is licensed by AT&T Corp.            *
*       under the terms and conditions of the license in       *
*       http://www.research.att.com/orgs/ssr/book/reuse        *
*                                                              *
*               This software was created by the               *
*           Software Engineering Research Department           *
*                    AT&T Bell Laboratories                    *
*                                                              *
*               For further information contact                *
*                     gsf@research.att.com                     *
*                                                              *
***************************************************************/

/* : : generated by proto : : */

#if !defined(__PROTO__)
#if defined(__STDC__) || defined(__cplusplus) || defined(_proto) || defined(c_plusplus)
#if defined(__cplusplus)
#define __MANGLE__	"C"
#else
#define __MANGLE__
#endif
#define __STDARG__
#define __PROTO__(x)	x
#define __OTORP__(x)
#define __PARAM__(n,o)	n
#if !defined(__STDC__) && !defined(__cplusplus)
#if !defined(c_plusplus)
#define const
#endif
#define signed
#define void		int
#define volatile
#define __V_		char
#else
#define __V_		void
#endif
#else
#define __PROTO__(x)	()
#define __OTORP__(x)	x
#define __PARAM__(n,o)	o
#define __MANGLE__
#define __V_		char
#define const
#define signed
#define void		int
#define volatile
#endif
#if defined(__cplusplus) || defined(c_plusplus)
#define __VARARG__	...
#else
#define __VARARG__
#endif
#if defined(__STDARG__)
#define __VA_START__(p,a)	va_start(p,a)
#else
#define __VA_START__(p,a)	va_start(p)
#endif
#endif
#include "pax.h"

/*
 * return read file descriptor for filtered current input file
 */

int
filter __PARAM__((register Archive_t* ap, register File_t* f), (ap, f)) __OTORP__(register Archive_t* ap; register File_t* f;){
	register int	n;
	int		rfd;
	int		wfd;
	Proc_t*		proc;

	if ((wfd = open(state.tmp.file, O_CREAT|O_TRUNC|O_WRONLY, S_IRUSR)) < 0)
	{
		error(2, "%s: cannot create filter temporary %s", f->path, state.tmp.file);
		return(-1);
	}
	if ((rfd = open(state.tmp.file, O_RDONLY)) < 0)
	{
		error(2, "%s: cannot open filter temporary %s", f->path, state.tmp.file);
		close(wfd);
		if (remove(state.tmp.file)) error(1, "%s: cannot remove filter temporary %s", f->path, state.tmp.file);
		return(-1);
	}
	if (remove(state.tmp.file)) error(1, "%s: cannot remove filter temporary %s", f->path, state.tmp.file);
	*state.filterarg = f->path;
	if (!(proc = procopen(*state.filter, state.filter, NiL, NiL, PROC_READ)))
	{
		error(2, "%s: cannot execute filter %s", f->path, *state.filter);
		close(rfd);
		close(wfd);
		return(-1);
	}
	if (ap->format == ASCHK) f->checksum = 0;
	f->st->st_size = 0;
	holeinit(wfd);
	while ((n = read(proc->rfd, state.tmp.buffer, state.buffersize)) > 0)
	{
		if (holewrite(wfd, state.tmp.buffer, n) != n)
		{
			error(2, "%s: filter write error", f->path);
			break;
		}
		if (ap->format == ASCHK) f->checksum = asc_checksum(state.tmp.buffer, n, f->checksum);
		f->st->st_size += n;
	}
	holedone(wfd);
	if (n < 0) error(ERROR_SYSTEM|2, "%s: %s filter read error", f->path, *state.filter);
	if (n = procclose(proc)) error(2, "%s: %s filter exit code %d", f->path, *state.filter, n);
	close(wfd);
	message((-1, "%s: filter file size = %ld", f->path, f->st->st_size));
	return(rfd);
}

/*
 * return read file descriptor for current input file
 */

int
openin __PARAM__((register Archive_t* ap, register File_t* f), (ap, f)) __OTORP__(register Archive_t* ap; register File_t* f;){
	register int	n;
	int		rfd;

	if (f->type != X_IFREG)
		rfd = -1;
	else if (state.filter)
		rfd = filter(ap, f);
	else if ((rfd = open(f->st->st_size ? f->path : "/dev/null", O_RDONLY)) < 0)
		error(ERROR_SYSTEM|2, "%s: cannot read", f->path);
	else if (ap->format == ASCHK)
	{
		f->checksum = 0;
		if (lseek(rfd, 0L, SEEK_SET) != 0)
			error(ERROR_SYSTEM|1, "%s: %s checksum seek error", f->path, format[ap->format].name);
		else
		{
			while ((n = read(rfd, state.tmp.buffer, state.buffersize)) > 0)
				f->checksum = asc_checksum(state.tmp.buffer, n, f->checksum);
			if (n < 0)
				error(ERROR_SYSTEM|2, "%s: %s checksum read error", f->path, format[ap->format].name);
			if (lseek(rfd, 0L, SEEK_SET) != 0)
				error(ERROR_SYSTEM|1, "%s: %s checksum seek error", f->path, format[ap->format].name);
		}
	}
	if (rfd < 0)
		f->st->st_size = 0;
	return(rfd);
}

/*
 * create directory and all path name components leading to directory
 */

static int
missdir __PARAM__((register Archive_t* ap, register File_t* f), (ap, f)) __OTORP__(register Archive_t* ap; register File_t* f;){
	register char*	s;
	register char*	t;
	long		pp;
	struct stat*	st;
	struct stat*	sp;
	struct stat	st0;
	struct stat	st1;

	s = f->name;
	pathcanon(s, 0);
	if (t = strchr(*s == '/' ? s + 1 : s, '/'))
	{
		if (!state.intermediate)
		{
			static int	warned;

			if (!warned)
			{
				error(1, "omit the -d option to create intermediate directories");
				warned = 1;
			}
			return(-1);
		}
		st = 0;
		sp = &st0;
		do
		{
			*t = 0;
			if (stat(s, sp))
			{
				*t = '/';
				break;
			}
			*t = '/';
			st = sp;
			sp = (sp == &st0) ? &st1 : &st0;
		} while (t = strchr(t + 1, '/'));
		if (t)
		{
			if (!st && stat(".", st = &st0))
				error(ERROR_SYSTEM|3, "%s: cannot stat .", s);
			pp = f->perm;
			f->perm = st->st_mode & state.modemask;
			sp = f->st;
			f->st = st;
			do
			{
				*t = 0;
				if (mkdir(s, f->perm))
				{
					error(ERROR_SYSTEM|2, "%s: cannot create directory", s);
					*t = '/';
					f->perm = pp;
					f->st = sp;
					return(-1);
				}
				setfile(ap, f);
				*t = '/';
			} while (t = strchr(t + 1, '/'));
			f->perm = pp;
			f->st = sp;
		}
	}
	return(0);
}

/*
 * open file for writing, set all necessary info
 */

int
openout __PARAM__((register Archive_t* ap, register File_t* f), (ap, f)) __OTORP__(register Archive_t* ap; register File_t* f;){
	register int	fd;
	int		exists;
	struct stat	st;

	pathcanon(f->name, 0);

	/*
	 * if not found and state.update then check down the view
	 *
	 * NOTE: VPATH in app code is ugly but the benefits of the
	 *	 combination with state.update win over beauty
	 */

	if (exists = !(*state.statf)(f->name, &st))
	{
		if (!state.clobber && !S_ISDIR(st.st_mode))
		{
			error(1, "%s: already exists -- not overwritten", f->name);
			return(-1);
		}
		st.st_mode = modex(st.st_mode);
	}
	else
	{
		typedef struct View
		{
			struct View*	next;
			char*		root;
			dev_t		dev;
			ino_t		ino;
		} View_t;

		View_t*			vp;
		View_t*			tp;
		char*			s;
		char*			e;

		static View_t*		view;
		static char*		offset;

		if (state.update && !offset)
		{
			if (s = getenv("VPATH"))
			{
				if (!(s = strdup(s)))
					error(ERROR_SYSTEM|3, "out of space [VPATH]");
				do
				{
					if (e = strchr(s, ':')) *e++ = 0;
					if (!(vp = newof(0, View_t, 1, 0)))
						error(ERROR_SYSTEM|3, "out of space [view]");
					vp->root = s;
					if (stat(s, &st))
					{
						vp->dev = 0;
						vp->ino = 0;
					}
					else
					{
						vp->dev = st.st_dev;
						vp->ino = st.st_ino;
					}
					if (view) tp = tp->next = vp;
					else view = tp = vp;
				} while (s = e);
				s = state.pwd;
				e = 0;
				for (;;)
				{
					if (stat(s, &st))
						error(ERROR_SYSTEM|3, "%s: cannot stat pwd component", s);
					for (vp = view; vp; vp = vp->next)
						if (vp->ino == st.st_ino && vp->dev == st.st_dev)
						{
							offset = e ? e + 1 : ".";
							tp = view;
							view = vp->next;
							while (tp && tp != view)
							{
								vp = tp;
								tp = tp->next;
								free(vp);
							}
							goto found;
						}
					if (e) *e = '/';
					else e = s + strlen(s);
					while (e > s && *--e != '/');
					if (e <= s) break;
					*e = 0;
				}
			}
		found:
			if (!offset) offset = ".";
		}
		st.st_mode = 0;
		st.st_mtime = 0;
		if (*f->name != '/')
			for (vp = view; vp; vp = vp->next)
			{
				sfsprintf(state.tmp.buffer, state.tmp.buffersize - 1, "%s/%s/%s", vp->root, offset, f->name);
				if (!stat(state.tmp.buffer, &st)) break;
			}
	}
	if (f->delta.op == DELTA_delete)
	{
		if (exists) switch (X_ITYPE(st.st_mode))
		{
		case X_IFDIR:
			if (!streq(f->name, ".") && !streq(f->name, ".."))
			{
				if (rmdir(f->name)) error(ERROR_SYSTEM|2, "%s: cannot remove directory", f->name);
				else listentry(f);
			}
			break;
		default:
			if (remove(f->name)) error(ERROR_SYSTEM|2, "%s: cannot remove file", f->name);
			else listentry(f);
			break;
		}
		return(-1);
	}
	if (state.operation == (IN|OUT))
	{
		if (exists && f->st->st_ino == st.st_ino && f->st->st_dev == st.st_dev)
		{
			error(2, "attempt to pass %s to self", f->name);
			return(-1);
		}
		if (state.linkf && f->type != X_IFDIR && (state.linkf == pathsetlink || f->st->st_dev == state.dev))
		{
			if (exists) remove(f->name);
			if ((*state.linkf)(f->path, f->name))
			{
				if (!exists && missdir(ap, f))
				{
					error(ERROR_SYSTEM|2, "%s: cannot create intermediate directories", f->name);
					return(-1);
				}
				if (exists || (*state.linkf)(f->path, f->name))
				{
					error(ERROR_SYSTEM|2, "%s: cannot link to %s", f->path, f->name);
					return(-1);
				}
			}
			setfile(ap, f);
			return(-2);
		}
	}
	if (prune(ap, f, &st)) return(-1);
	switch (f->type)
	{
	case X_IFDIR:
		f->st->st_size = 0;
		if (f->name[0] == '.' && (f->name[1] == 0 || f->name[1] == '.' && f->name[2] == 0)) return(-1);
		if (exists)
		{
			if (X_ITYPE(st.st_mode) != X_IFDIR)
			{
				error(1, "current %s is not a directory", f->name);
				return(-1);
			}
		}
		else if (mkdir(f->name, f->perm) && (missdir(ap, f) || mkdir(f->name, f->perm)))
		{
			error(ERROR_SYSTEM|2, "%s: cannot create directory", f->name);
			return(-1);
		}
		setfile(ap, f);
		return(state.update && exists ? -1 : -2);
	case X_IFLNK:
		if (streq(f->name, f->linkname))
		{
			error(1, "%s: symbolic link loops to self", f->name);
			return(-1);
		}
		if (exists && remove(f->name))
		{
			error(ERROR_SYSTEM|2, "cannot remove current %s", f->name);
			return(-1);
		}
		if (pathsetlink(f->linkname, f->name))
		{
			if (!exists && missdir(ap, f))
			{
				error(ERROR_SYSTEM|2, "%s: cannot create intermediate directories", f->name);
				return(-1);
			}
			if (exists || pathsetlink(f->linkname, f->name))
			{
				error(ERROR_SYSTEM|2, "%s: cannot symlink to %s", f->name, f->linkname);
				return(-1);
			}
		}
		return(-2);
	}
	if (!addlink(ap, f)) return(-1);
	if (exists && remove(f->name))
	{
		error(ERROR_SYSTEM|2, "cannot remove current %s", f->name);
		return(-1);
	}
	switch (f->type)
	{
	case X_IFIFO:
	case X_IFSOCK:
		IDEVICE(f->st, 0);
	case X_IFBLK:
	case X_IFCHR:
		f->st->st_size = 0;
		if (mknod(f->name, f->st->st_mode, idevice(f->st)))
		{
			if (errno == EPERM)
			{
				error(ERROR_SYSTEM|2, "%s: cannot create %s special file", f->name, (f->type == X_IFBLK) ? "block" : "character");
				return(-1);
			}
			if (!exists && missdir(ap, f))
			{
				error(ERROR_SYSTEM|2, "%s: cannot create intermediate directories", f->name);
				return(-1);
			}
			if (exists || mknod(f->name, f->st->st_mode, idevice(f->st)))
			{
				error(ERROR_SYSTEM|2, "%s: cannot mknod", f->name);
				return(-1);
			}
		}
		setfile(ap, f);
		return(-2);
	default:
		error(1, "%s: unknown file type 0%03o -- creating regular file", f->name, f->type >> 12);
		/*FALLTHROUGH*/
	case X_IFREG:
		if ((fd = open(f->name, O_CREAT|O_TRUNC|O_WRONLY, f->perm)) < 0)
		{
			if (!exists && missdir(ap, f))
			{
				error(ERROR_SYSTEM|2, "%s: cannot create intermediate directories", f->name);
				return(-1);
			}
			if (exists || (fd = open(f->name, O_CREAT|O_TRUNC|O_WRONLY, f->perm)) < 0)
			{
				error(ERROR_SYSTEM|2, "%s: cannot create", f->name);
				return(-1);
			}
		}
		return(fd);
	}
}

/*
 * get file info for output
 */

int
getfile __PARAM__((register Archive_t* ap, register File_t* f, register Ftw_t* ftw), (ap, f, ftw)) __OTORP__(register Archive_t* ap; register File_t* f; register Ftw_t* ftw;){
	register char*		name;

	static struct stat	st;
	static char		pathbuffer[PATH_MAX];
	static char		namebuffer[PATH_MAX];

	name = ftw->path;
	message((-4, "getfile(%s)", name));
	if (ftw->pathlen >= sizeof(namebuffer))
	{
		error(2, "%s: file name too long", name);
		return(0);
	}
	switch (ftw->info)
	{
	case FTW_NS:
		error(2, "%s: not found", name);
		return(0);
	case FTW_DNR:
		if (state.files)
			error(2, "%s: cannot read directory", name);
		break;
	case FTW_D:
	case FTW_DNX:
	case FTW_DP:
		/*
		 * stdin files most likely come from tw/find with
		 * directory descendents already included; in posix
		 * omitting -d would result in duplicate output copies
		 * so we avoid the problem by peeking ahead and pruning
		 * the descents if they are already included
		 */

		if (!state.descend || state.descend == RESETABLE && !state.files && (state.peekfile || (state.peekfile = sfgetr(sfstdin, '\n', 1))) && dirprefix(name, state.peekfile) && !(state.descend = 0))
			ftw->status = FTW_SKIP;
		else if (ftw->info == FTW_DNX)
		{
			error(2, "%s: cannot search directory", name);
			ftw->status = FTW_SKIP;
		}
		break;
	}
	if (state.xdev && ftw->statb.st_dev != ftw->parent->statb.st_dev)
	{
		ftw->status = FTW_SKIP;
		return(0);
	}
	if (ap->delta)
		ap->delta->hdr = ap->delta->hdrbuf;
	f->path = strcpy(pathbuffer, name);
	pathcanon(strcpy(namebuffer, name), 0);
	f->name = map(namebuffer);
	if (state.files && state.operation == (IN|OUT) && dirprefix(state.destination, name))
		return(0);
	f->namesize = strlen(f->name) + 1;
	st = ftw->statb;
	f->st = &st;
	f->perm = st.st_mode & S_IPERM;
	f->st->st_mode = modex(f->st->st_mode);
	f->type = X_ITYPE(f->st->st_mode);
	f->linktype = NOLINK;
	f->linkname = 0;
	f->linknamesize = 0;
	f->uidname = 0;
	f->gidname = 0;
	if (!validout(ap, f))
		return(0);
	if (state.operation == OUT && f->type != X_IFDIR)
	{
		if (!addlink(ap, f)) f->st->st_size = 0;
		message((-3, "getfile(%s): dev'=%d ino'=%d", f->name, f->st->st_dev, f->st->st_ino));
	}
	ap->entries++;
	f->delta.op = 0;
	f->skip = 0;
	message((-2, "getfile(): path=%s name=%s mode=%s size=%ld", name, f->name, fmtmode(f->st->st_mode, 1), f->st->st_size));
	return(1);
}

/*
 * check that f is valid for archive output
 */

int
validout __PARAM__((register Archive_t* ap, register File_t* f), (ap, f)) __OTORP__(register Archive_t* ap; register File_t* f;){
	register char*	s;

	static char	linkbuffer[PATH_MAX];
	static char	idbuffer[ALAR_NAMESIZE + 1];

	switch (f->type)
	{
	case X_IFCHR:
	case X_IFBLK:
		f->st->st_size = 0;
		break;
	case X_IFREG:
		IDEVICE(f->st, 0);
		break;
	case X_IFLNK:
		if (f->st->st_size >= sizeof(linkbuffer) - 1)
		{
			error(2, "%s: symbolic link too long", f->path);
			return(0);
		}
		if (pathgetlink(f->path, linkbuffer, sizeof(linkbuffer) - 1) != f->st->st_size)
		{
			error(2, "%s: cannot read symbolic link", f->path);
			return(0);
		}
		f->linktype = SOFTLINK;
		pathcanon(linkbuffer, 0);
		f->linkname = (state.ftwflags & FTW_PHYSICAL) ? linkbuffer : map(linkbuffer);
		f->linknamesize = strlen(f->linkname) + 1;
		if (streq(f->path, f->linkname))
		{
			error(2, "%s: symbolic link loops to self", f->path);
			return(0);
		}
		f->st->st_size = 0;
		IDEVICE(f->st, 0);
		break;
	case X_IFDIR:
		if (streq(f->path, ".") || streq(f->path, "..")) return(0);
		f->st->st_size = 0;
		IDEVICE(f->st, 0);
		break;
	}
	switch (ap->format)
	{
	case ALAR:
	case IBMAR:
	case SAVESET:
		if (f->type != X_IFREG)
		{
			error(2, "%s: only regular files copied in %s format", f->path, format[ap->format].name);
			return(0);
		}
		if (s = strrchr(f->name, '/'))
		{
			s++;
			error(1, "%s: file name stripped to %s", f->name, s);
		}
		else s = f->name;
		if (strlen(s) > sizeof(idbuffer) - 1)
		{
			error(2, "%s: file name too long", f->name);
			return(0);
		}
		f->id = strupper(strcpy(idbuffer, s));
		break;
	case BINARY:
		if (f->namesize > BINARY_NAMESIZE)
		{
			error(2, "%s: file name too long", f->name);
			return(0);
		}
		break;
	case PAX:
	case TAR:
	case USTAR:
		if (f->namesize > sizeof(tar_header.name) + ((ap->format == TAR) ? -(f->type == X_IFDIR) : sizeof(tar_header.prefix)))
		{
			error(2, "%s: file name too long", f->name);
			return(0);
		}
		if (f->linknamesize > sizeof(tar_header.linkname))
		{
			error(2, "%s: link name too long", f->name);
			return(0);
		}
		break;
	}
	return(1);
}

/*
 * add file which may be a link
 * 0 returned if <dev,ino> already added
 */

int
addlink __PARAM__((register Archive_t* ap, register File_t* f), (ap, f)) __OTORP__(register Archive_t* ap; register File_t* f;){
	register Link_t*	p;
	register char*		s;
	Fileid_t		id;
	unsigned short		us;

	static int			warned;

	id.dev = f->st->st_dev;
	id.ino = f->st->st_ino;
	if (!ap->delta) switch (state.operation)
	{
	case IN:
		us = id.dev;
		if (us > state.devcnt)
		{
			state.devcnt = us;
			state.inocnt = id.ino;
		}
		else if (us == state.devcnt)
		{
			us = id.ino;
			if (us > state.inocnt) state.inocnt = us;
		}
		break;
	case OUT:
		if (!++state.inocnt)
		{
			if (!++state.devcnt) goto toomany;
			state.inocnt = 1;
		}
		f->st->st_dev = state.devcnt;
		f->st->st_ino = state.inocnt;
		break;
	}
	if (f->type == X_IFDIR) return(0);
	switch (ap->format)
	{
	case ALAR:
	case IBMAR:
	case SAVESET:
		if (state.operation == IN || f->st->st_nlink <= 1) return(1);
		break;
	case PAX:
	case TAR:
	case USTAR:
		if (state.operation == IN)
		{
			if (f->linktype == NOLINK) return(1);
			goto linked;
		}
		/*FALLTHROUGH*/
	default:
		if (f->st->st_nlink <= 1) return(1);
		break;
	}
	if (p = (Link_t*)hashget(state.linktab, (char*)&id))
	{
		switch (ap->format)
		{
		case ALAR:
		case IBMAR:
		case SAVESET:
			error(1, "%s: hard link information lost in %s format", f->name, format[ap->format].name);
			return(1);
		}
		f->st->st_dev = p->id.dev;
		f->st->st_ino = p->id.ino;
		f->linktype = HARDLINK;
		f->linkname = p->name;
		f->linknamesize = p->namesize;
		if (state.operation == OUT) return(0);
	linked:
		message((-1, "addlink(%s,%s)", f->name, f->linkname));
		if (streq(f->name, f->linkname))
		{
			error(2, "%s: hard link loops to self", f->name);
			return(0);
		}
		if (!state.list)
		{
			s = f->linkname;
			if (access(s, 0))
			{
				f->skip = 1;
				error(2, "%s must exist for hard link %s", s, f->name);
				return(0);
			}
			remove(f->name);
			if (state.operation == IN && *s != '/')
			{
				strcpy(state.pwd + state.pwdlen, s);
				s = state.pwd;
			}
			if (link(s, f->name))
			{
				if (missdir(ap, f))
				{
					error(ERROR_SYSTEM|2, "%s: cannot create intermediate directories", f->name);
					return(0);
				}
				if (link(s, f->name))
				{
					error(ERROR_SYSTEM|2, "%s: cannot link to %s", f->linkname, f->name);
					return(-1);
				}
			}
		}
		return(0);
	}
	if (!(p = newof(0, Link_t, 1, 0)) || !(p->name = strdup(f->name))) goto toomany;
	p->namesize = strlen(p->name) + 1;
	p->id.dev = f->st->st_dev;
	p->id.ino = f->st->st_ino;
	hashput(state.linktab, NiL, p);
	return(-1);
 toomany:
	if (!warned)
	{
		warned = 1;
		error(1, "too many hard links -- some links may become copies");
	}
	return(-1);
}

/*
 * get file uid and gid names given numbers
 */

void
getidnames __PARAM__((register File_t* f), (f)) __OTORP__(register File_t* f;){
	if (!f->uidname) f->uidname = fmtuid(f->st->st_uid);
	if (!f->gidname) f->gidname = fmtgid(f->st->st_gid);
}

/*
 * set file uid and gid numbers given names
 */

void
setidnames __PARAM__((register File_t* f), (f)) __OTORP__(register File_t* f;){
	register int	id;

	if (f->uidname)
	{
		if ((id = struid(f->uidname)) < 0)
		{
			if (id == -1 && state.owner) error(1, "%s: invalid user name", f->uidname);
			f->uidname = 0;
			id = state.uid;
		}
		f->st->st_uid = id;
	}
	if (f->gidname)
	{
		if ((id = strgid(f->gidname)) < 0)
		{
			if (id == -1 && state.owner) error(1, "%s: invalid group name", f->gidname);
			f->gidname = 0;
			id = state.gid;
		}
		f->st->st_gid = id;
	}
}

/*
 * allocate and initialize new archive pointer
 */

Archive_t*
initarchive __PARAM__((const char* name, int mode), (name, mode)) __OTORP__(const char* name; int mode;){
	Archive_t*	ap;

	if (!(ap = newof(0, Archive_t, 1, 0)))
		error(3, "out of space [initarchive]");
	initfile(ap, &ap->file, NiL, 0);
	ap->name = (char*)name;
	ap->format = -1;
	ap->sum = -1;
	ap->io.mode = mode;
	return(ap);
}

/*
 * return pointer to archive for op
 */

Archive_t*
getarchive __PARAM__((int op), (op)) __OTORP__(int op;){
	Archive_t**	app;

	app = (op & OUT) ? &state.out : &state.in;
	if (!*app) *app = initarchive(NiL, (op & OUT) ? (O_CREAT|O_TRUNC|O_WRONLY) : O_RDONLY);
	return(*app);
}

/*
 * initialize file info with name and mode
 */

void
initfile __PARAM__((register Archive_t* ap, register File_t* f, register char* name, int mode), (ap, f, name, mode)) __OTORP__(register Archive_t* ap; register File_t* f; register char* name; int mode;){
	static struct stat	st;

	memzero(f, sizeof(*f));
	f->st = ap ? &ap->st : &st;
	memzero(f->st, sizeof(*f->st));
	if (name)
	{
		f->id = f->name = f->path = name;
		f->namesize = strlen(name) + 1;
	}
	f->st->st_mode = modex(mode);
	f->st->st_nlink = 1;		/* system V needs this!!! */
}

/*
 * set copied file info
 */

void
setfile __PARAM__((register Archive_t* ap, register File_t* f), (ap, f)) __OTORP__(register Archive_t* ap; register File_t* f;){
	register Post_t*	p;
	Post_t			post;

	if (!ap->pass) switch (f->type)
	{
	case X_IFLNK:
		break;
	case X_IFDIR:
		if (state.modtime || state.owner || (f->perm & (S_IWUSR|S_IXUSR)) != (S_IWUSR|S_IXUSR))
		{
			if (!(p = newof(0, Post_t, 1, 0)))
				error(3, "not enough space for file restoration info");
			p->mtime = f->st->st_mtime;
			p->uid = f->st->st_uid;
			p->gid = f->st->st_gid;
			p->mode = f->perm;
			if ((f->perm & S_IRWXU) != S_IRWXU && chmod(f->name, f->perm|S_IRWXU))
				error(1, "%s: cannot chmod to %s", f->name, fmtmode(f->st->st_mode|X_IRWXU, 1) + 1);
			hashput(state.restore, f->name, p);
			break;
		}
		/*FALLTHROUGH*/
	default:
		p = &post;
		p->mtime = f->st->st_mtime;
		p->uid = f->st->st_uid;
		p->gid = f->st->st_gid;
		p->mode = f->perm;
		restore(f->name, (char*)p, NiL);
		break;
	}
}

/*
 * set access and modification times of file
 */

void
settime __PARAM__((const char* name, time_t atime, time_t mtime), (name, atime, mtime)) __OTORP__(const char* name; time_t atime; time_t mtime;){
	if (touch(name, atime, mtime, 0))
		error(1, "%s: cannot set times", name);
}

/*
 * restore file status after processing
 */

int
restore __PARAM__((register const char* name, char* ap, __V_* handle), (name, ap, handle)) __OTORP__(register const char* name; char* ap; __V_* handle;){
	register Post_t*	p = (Post_t*)ap;
	int			m;
	struct stat		st;

	NoP(handle);
	if (state.owner)
	{
		if (state.flags & SETIDS)
		{
			p->uid = state.setuid;
			p->gid = state.setgid;
		}
		if (chown(name, p->uid, p->gid) < 0)
			error(1, "%s: cannot chown to (%d,%d)", name, p->uid, p->gid);
	}
	if ((p->mode & (S_IWUSR|S_IXUSR)) != (S_IWUSR|S_IXUSR))
	{
		if (chmod(name, p->mode & state.modemask))
			error(1, "%s: cannot chmod to %s", name, fmtmode(p->mode & state.modemask, 0) + 1);
		else if (m = p->mode & (S_ISUID|S_ISGID|S_ISVTX))
		{
			if (stat(name, &st))
				error(1, "%s: not found", name);
			else if (m ^= (st.st_mode & (S_ISUID|S_ISGID|S_ISVTX)))
				error(1, "%s: mode %s not set", name, fmtmode(m, 0) + 1);
		}
	}
	if (state.modtime) settime(name, p->mtime, p->mtime);
	return(0);
}

/*
 * return 1 if f output can be pruned
 */

int
prune __PARAM__((register Archive_t* ap, register File_t* f, register struct stat* st), (ap, f, st)) __OTORP__(register Archive_t* ap; register File_t* f; register struct stat* st;){
	if (st->st_mode == f->st->st_mode && (ap->delta && f->st->st_mtime == st->st_mtime || state.update && (unsigned long)f->st->st_mtime <= (unsigned long)st->st_mtime))
		return(1);
	return(0);
}

/*
 * write siz bytes of buf to fd checking for HOLE_MIN hole chunks
 * we assume siz is rounded nicely until the end
 */

ssize_t
holewrite __PARAM__((int fd, __V_* buf, size_t siz), (fd, buf, siz)) __OTORP__(int fd; __V_* buf; size_t siz;){
	register char*	t = (char*)buf;
	register char*	e = t + siz;
	register char*	b = 0;
	register char*	s;
	ssize_t		i;
	ssize_t		n = 0;

	static char	hole[HOLE_MIN];

#if DEBUG
	if (state.test & 0100)
		b = t;
	else
#endif
	while (t < e)
	{
		s = t;
		if ((t += HOLE_MIN) > e)
			t = e;
		if (!*s && !*(t - 1) && !memcmp(s, hole, t - s))
		{
			if (b)
			{
				if (state.hole)
				{
					if (lseek(fd, state.hole, SEEK_CUR) < state.hole)
						return(-1);
					state.hole = 0;
				}
				if ((i = write(fd, b, s - b)) != (s - b))
					return(i);
				n += i;
				b = 0;
			}
			state.hole += t - s;
			n += t - s;
		}
		else if (!b)
			b = s;
	}
	if (b)
	{
		if (state.hole)
		{
			if (lseek(fd, state.hole, SEEK_CUR) < state.hole)
				return(-1);
			state.hole = 0;
		}
		if ((i = write(fd, b, e - b)) != (e - b))
			return(i);
		n += i;
	}
	return(n);
}
