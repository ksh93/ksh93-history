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
 * mounted filesystem scan support
 * where are the standards when you really need them
 */

#include <ast.h>
#include <mnt.h>
#include <ls.h>

#if _lib_mntopen && _lib_mntread && _lib_mntclose

NoN(mnt)

#else

/*
 * the original interface just had mode
 */

#define FIXARGS(p,m,s)		do {					\
					if ((p)&&*(p)!='/') {		\
						mode = p;		\
						path = 0;		\
					}				\
					if (!path)			\
						path = s;		\
				} while (0)
typedef struct
{
	Mnt_t	mnt;
	char	buf[128];
} Header_t;

static void
set(register Header_t* hp, const char* fs, const char* dir, const char* type, const char* options)
{
	const char*	x;

	hp->mnt.flags = 0;
	if (x = (const char*)strchr(fs, ':'))
	{
		if (*++x)
		{
			hp->mnt.flags |= MNT_REMOTE;
			if (*x == '(')
			{
				fs = x;
				type = "auto";
			}
		}
	}
	else if (x = (const char*)strchr(fs, '@'))
	{
		hp->mnt.flags |= MNT_REMOTE;
		sfsprintf(hp->buf, sizeof(hp->buf) - 1, "%s:%*.*s", x + 1, x - fs, x - fs, fs);
		fs = (const char*)hp->buf;
	}
	if (streq(fs, "none"))
		fs = dir;
	hp->mnt.fs = (char*)fs;
	hp->mnt.dir = (char*)dir;
	hp->mnt.type = (char*)type;
	hp->mnt.options = (char*)options;
}

#undef	MNT_REMOTE

#if _lib_getmntinfo && _sys_mount

/*
 * 4.4 bsd -- we are the world (altogether now)
 *
 * what a crappy interface
 * data returned in static buffer -- ok
 * big chunk of allocated memory that cannot be freed -- come on
 */

#include <sys/param.h>		/* expect some macro redefinitions here */
#include <sys/mount.h>

typedef struct
{
	Header_t	hdr;
	struct statfs*	next;
	struct statfs*	last;
} Handle_t;

#ifdef INITMOUNTNAMES
#define TYPE(f,x)	((char*)type[x])
static const char*	type[] = INITMOUNTNAMES;
#else
#if _sys_fs_types
#define TYPE(f,x)	((char*)mnt_names[x])
#include <sys/fs_types.h>
#else
#define TYPE(f,x)	(strchr(f,':')?"nfs":"ufs")
#endif
#endif

void*
mntopen(const char* path, const char* mode)
{
	register Handle_t*	mp;
	register int		n;

	FIXARGS(path, mode, 0);
	if (!(mp = newof(0, Handle_t, 1, 0)))
		return 0;
	if ((n = getmntinfo(&mp->next, 0)) <= 0)
	{
		free(mp);
		return 0;
	}
	mp->last = mp->next + n;
	return (void*)mp;
}

Mnt_t*
mntread(void* handle)
{
	register Handle_t*	mp = (Handle_t*)handle;

	if (mp->next < mp->last)
	{
		set(&mp->hdr, mp->next->f_mntfromname, mp->next->f_mntonname, TYPE(mp->next->f_mntfromname, mp->next->f_type), NiL);
		mp->next++;
		return &mp->hdr.mnt;
	}
	return 0;
}

int
mntclose(void* handle)
{
	register Handle_t*	mp = (Handle_t*)handle;

	if (!mp)
		return -1;
	free(mp);
	return 0;
}

#else

#if _lib_mntctl && _sys_vmount

/*
 * aix
 */

#include <sys/vmount.h>

#define SIZE		(16 * 1024)

static const char*	type[] =
{
	"aix", "aix#1", "nfs", "jfs", "aix#4", "cdrom"
};

typedef struct
{
	Header_t	hdr;
	long		count;
	struct vmount*	next;
	char		remote[128];
	struct vmount	info[1];
} Handle_t;

void*
mntopen(const char* path, const char* mode)
{
	register Handle_t*	mp;

	FIXARGS(path, mode, 0);
	if (!(mp = newof(0, Handle_t, 1, SIZE)))
		return 0;
	if ((mp->count = mntctl(MCTL_QUERY, sizeof(Handle_t) + SIZE, &mp->info)) <= 0)
	{
		free(mp);
		return 0;
	}
	mp->next = mp->info;
	return (void*)mp;
}

Mnt_t*
mntread(void* handle)
{
	register Handle_t*	mp = (Handle_t*)handle;
	register char*		s;
	register int		n;

	if (mp->count > 0)
	{
		if (vmt2datasize(mp->next, VMT_HOST) && (s = vmt2dataptr(mp->next, VMT_HOST)) && !streq(s, "-"))
		{
			sfsprintf(mp->remote, sizeof(mp->remote) - 1, "%s:%s", s, vmt2dataptr(mp->next, VMT_OBJECT));
			s = mp->remote;
		}
		else s = vmt2dataptr(mp->next, VMT_OBJECT);
		if ((n = mp->next->vmt_gfstype) < 0 || n >= elementsof(type))
			n = 0;
		set(&mp->hdr, s, vmt2dataptr(mp->next, VMT_STUB), type[n], NiL);
		if (--mp->count > 0)
			mp->next = (struct vmount*)((char*)mp->next + mp->next->vmt_length);
		return &mp->hdr.mnt;
	}
	return 0;
}

int
mntclose(void* handle)
{
	register Handle_t*	mp = (Handle_t*)handle;

	if (!mp)
		return -1;
	free(mp);
	return 0;
}

#else

#if !_lib_setmntent
#undef	_lib_getmntent
#if !_SCO_COFF && !_SCO_ELF
#undef	_hdr_mnttab
#endif
#endif

#if _lib_getmntent && ( _hdr_mntent || _sys_mntent && !_sys_mnttab )

#if defined(__STDPP__directive) && defined(__STDPP__hide)
__STDPP__directive pragma pp:hide endmntent getmntent
#else
#define endmntent	______endmntent
#define getmntent	______getmntent
#endif

#include <stdio.h>
#if _hdr_mntent
#include <mntent.h>
#else
#include <sys/mntent.h>
#endif

#if defined(__STDPP__directive) && defined(__STDPP__hide)
__STDPP__directive pragma pp:nohide endmntent getmntent
#else
#undef	endmntent
#undef	getmntent
#endif

extern int		endmntent(FILE*);
extern struct mntent*	getmntent(FILE*);

#else

#undef	_lib_getmntent

#if _hdr_mnttab
#include <mnttab.h>
#else
#if _sys_mnttab
#include <sys/mnttab.h>
#endif
#endif

#endif

#ifndef MOUNTED
#ifdef	MNT_MNTTAB
#define MOUNTED		MNT_MNTTAB
#else
#if _hdr_mnttab || _sys_mnttab
#define MOUNTED		"/etc/mnttab"
#else
#define MOUNTED		"/etc/mtab"
#endif
#endif
#endif

#if _lib_getmntent

typedef struct
#if _mem_mnt_opts_mntent
#define OPTIONS(p)	((p)->mnt_opts)
#else
#define OPTIONS(p)	NiL
#endif

{
	Header_t	hdr;
	FILE*		fp;
} Handle_t;

void*
mntopen(const char* path, const char* mode)
{
	register Handle_t*	mp;

	FIXARGS(path, mode, MOUNTED);
	if (!(mp = newof(0, Handle_t, 1, 0)))
		return 0;
	if (!(mp->fp = setmntent(path, mode)))
	{
		free(mp);
		return 0;
	}
	return (void*)mp;
}

Mnt_t*
mntread(void* handle)
{
	register Handle_t*	mp = (Handle_t*)handle;
	register struct mntent*	mnt;

	if (mnt = getmntent(mp->fp))
	{
		set(&mp->hdr, mnt->mnt_fsname, mnt->mnt_dir, mnt->mnt_type, OPTIONS(mnt));
		return &mp->hdr.mnt;
	}
	return 0;
}

int
mntclose(void* handle)
{
	register Handle_t*	mp = (Handle_t*)handle;

	if (!mp)
		return -1;
	endmntent(mp->fp);
	free(mp);
	return 0;
}

#else

#if _sys_mntent && _lib_w_getmntent

#include <sys/mntent.h>

#define mntent		w_mntent

#define mnt_dir		mnt_mountpoint
#define mnt_type	mnt_fstname

#define MNTBUFSIZE	(sizeof(struct w_mnth)+16*sizeof(struct w_mntent))

#if _mem_mnt_opts_w_mntent
#define OPTIONS(p)	((p)->mnt_opts)
#else
#define OPTIONS(p)	NiL
#endif

#else

#undef _lib_w_getmntent

#define MNTBUFSIZE	sizeof(struct mntent)

#if !_mem_mt_dev_mnttab || !_mem_mt_filsys_mnttab
#undef	_hdr_mnttab
#endif

#if _hdr_mnttab

#define mntent	mnttab

#define mnt_fsname	mt_dev
#define mnt_dir		mt_filsys
#if _mem_mt_fstyp_mnttab
#define mnt_type	mt_fstyp
#endif

#if _mem_mnt_opts_mnttab
#define OPTIONS(p)	((p)->mnt_opts)
#else
#define OPTIONS(p)	NiL
#endif

#else

struct mntent
{
	char	mnt_fsname[256];
	char	mnt_dir[256];
	char	mnt_type[32];
	char	mnt_opts[64];
};

#define OPTIONS(p)	((p)->mnt_opts)

#endif

#endif

typedef struct
{
	Header_t	hdr;
	Sfio_t*		fp;
	struct mntent*	mnt;
#if _lib_w_getmntent
	int		count;
#endif
	char		buf[MNTBUFSIZE];
} Handle_t;

void*
mntopen(const char* path, const char* mode)
{
	register Handle_t*	mp;

	FIXARGS(path, mode, MOUNTED);
	if (!(mp = newof(0, Handle_t, 1, 0)))
		return 0;
#if _lib_w_getmntent
	if ((mp->count = w_getmntent(mp->buf, sizeof(mp->buf))) > 0)
		mp->mnt = (struct mntent*)(((struct w_mnth*)mp->buf) + 1);
	else
#else
	mp->mnt = (struct mntent*)mp->buf;
	if (!(mp->fp = sfopen(NiL, path, mode)))
#endif
	{
		free(mp);
		return 0;
	}
	return (void*)mp;
}

Mnt_t*
mntread(void* handle)
{
	register Handle_t*	mp = (Handle_t*)handle;

#if _lib_w_getmntent

	if (mp->count-- <= 0)
	{
		if ((mp->count = w_getmntent(mp->buf, sizeof(mp->buf))) <= 0)
			return 0;
		mp->count--;
		mp->mnt = (struct mntent*)(((struct w_mnth*)mp->buf) + 1);
	}
	set(&mp->hdr, mp->mnt->mnt_fsname, mp->mnt->mnt_dir, mp->mnt->mnt_type, OPTIONS(mp->mnt));
	mp->mnt++;
	return &mp->hdr.mnt;

#else

#if _hdr_mnttab

	while (sfread(mp->fp, &mp->buf, sizeof(mp->buf)) == sizeof(mp->buf))
		if (*mp->mnt->mnt_fsname && *mp->mnt->mnt_dir)
		{
#ifndef mnt_type
			struct stat	st;

			static char	typ[32];

			set(&mp->hdr, mp->mnt->mnt_fsname, mp->mnt->mnt_dir, stat(mp->mnt->mnt_dir, &st) ? FS_default : strncpy(typ, fmtfs(&st), sizeof(typ) - 1), OPTIONS(mp->mnt));
#else
			set(&mp->hdr, mp->mnt->mnt_fsname, mp->mnt->mnt_dir, mp->mnt->mnt_type, OPTIONS(mp->mnt));
#endif
			return &mp->hdr.mnt;
		}
	return 0;

#else

	register int		c;
	register char*		s;
	register char*		m;
	register char*		b;
	register int		q;

 again:
	q = 0;
	b = s = mp->mnt->mnt_fsname;
	m = s + sizeof(mp->mnt->mnt_fsname) - 1;
	for (;;) switch (c = sfgetc(mp->fp))
	{
	case EOF:
		return 0;
	case ' ':
	case '\t':
		if (s != b) switch (++q)
		{
		case 1:
			*s = 0;
			b = s = mp->mnt->mnt_dir;
			m = s + sizeof(mp->mnt->mnt_dir) - 1;
			break;
		case 2:
			*s = 0;
			b = s = mp->mnt->mnt_type;
			m = s + sizeof(mp->mnt->mnt_type) - 1;
			break;
		case 3:
			*s = 0;
			b = s = mp->mnt->mnt_opts;
			m = s + sizeof(mp->mnt->mnt_opts) - 1;
			break;
		case 4:
			*s = 0;
			b = s = m = 0;
			break;
		}
		break;
	case '\n':
		if (q >= 3)
		{
			set(&mp->hdr, mp->mnt->mnt_fsname, mp->mnt->mnt_dir, mp->mnt->mnt_type, OPTIONS(mp->mnt));
			return &mp->hdr.mnt;
		}
		goto again;
	default:
		if (s < m)
			*s++ = c;
		break;
	}

#endif

#endif

}

int
mntclose(void* handle)
{
	register Handle_t*	mp = (Handle_t*)handle;

	if (!mp)
		return -1;
	sfclose(mp->fp);
	free(mp);
	return 0;
}

#endif

#endif

#endif

/*
 * currently no write
 */

int
mntwrite(void* handle, const Mnt_t* mnt)
{
	NoP(handle);
	NoP(mnt);
	return -1;
}

#endif
