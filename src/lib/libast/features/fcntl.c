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
 * generate POSIX fcntl.h
 */

#include "FEATURE/standards"	/* iffe --include-first */
#include "FEATURE/lib"

#include <sys/types.h>

#define getdtablesize	______getdtablesize
#define getpagesize	______getpagesize
#define ioctl		______ioctl

#if _typ_off64_t
#undef	off_t
#ifdef __STDC__
#define	off_t		off_t
#endif
#endif

#if _hdr_fcntl
#include <fcntl.h>
#endif
#if _hdr_unistd
#include <unistd.h>
#endif
#if _sys_socket
#include <sys/socket.h>
#endif

#include <sys/stat.h>

#include "FEATURE/fs"

#undef	getdtablesize   
#undef	getpagesize
#undef	ioctl

#include "FEATURE/tty"

#if _typ_off64_t
#undef	off_t
#define	off_t	off64_t
#endif

/*
 * some bit macros may in fact be bits
 */

static unsigned int
hibit(unsigned int m)
{
	unsigned int	b;

	while (b = m & (m - 1))
		m = b;
	return m;
}

int
main()
{
	unsigned int	f_local = 0;
	unsigned int	f_lck = 0;
	unsigned int	o_local = 2;

	printf("#pragma prototyped\n");
	printf("\n");
	printf("#if _typ_off64_t\n");
	printf("#undef	off_t\n");
	printf("#ifdef __STDC__\n");
	printf("#define	off_t		off_t\n");
	printf("#endif\n");
	printf("#endif\n");
	printf("\n");
	printf("#include <ast_fs.h>\n");
	printf("\n");
	printf("#if _typ_off64_t\n");
	printf("#undef	off_t\n");
	printf("#ifdef __STDC__\n");
	printf("#define	off_t		off_t\n");
	printf("#endif\n");
	printf("#endif\n");
	printf("\n");
	printf("#include <fcntl.h>\n");
#if _hdr_mman
	printf("#include <mman.h>\n");
#else
#if _sys_mman
	printf("#include <sys/mman.h>\n");
#endif
#endif
	printf("\n");
#ifndef	FD_CLOEXEC
	printf("#define FD_CLOEXEC	1\n");
	printf("\n");
#endif

#ifdef	F_DUPFD
	if (F_DUPFD > f_local) f_local = F_DUPFD;
#endif
#ifdef	F_DUPFD_CLOEXEC
	if (F_DUPFD_CLOEXEC > f_local) f_local = F_DUPFD_CLOEXEC;
#else
#define NEED_F	1
#endif
#ifdef	F_GETFD
	if (F_GETFD > f_local) f_local = F_GETFD;
#endif
#ifdef	F_GETFL
	if (F_GETFL > f_local) f_local = F_GETFL;
#endif
#ifdef	F_GETLK
	if (F_GETLK > f_local) f_local = F_GETLK;
#endif
#ifdef	F_RDLCK
	if (F_RDLCK > f_lck) f_lck = F_RDLCK;
#endif
#ifdef	F_SETFD
	if (F_SETFD > f_local) f_local = F_SETFD;
#endif
#ifdef	F_SETFL
	if (F_SETFL > f_local) f_local = F_SETFL;
#endif
#ifdef	F_SETLK
	if (F_SETLK > f_local) f_local = F_SETLK;
#endif
#ifdef	F_SETLKW
	if (F_SETLKW > f_local) f_local = F_SETLKW;
#endif
#ifdef	F_UNLCK
	if (F_UNLCK > f_lck) f_lck = F_UNLCK;
#endif
#ifdef	F_WRLCK
	if (F_WRLCK > f_lck) f_lck = F_WRLCK;
#endif
#ifdef	F_EXLCK
	if (F_EXLCK > f_local) f_local = F_EXLCK;
#endif
#ifdef	F_GETLEASE
	if (F_GETLEASE > f_local) f_local = F_GETLEASE;
#endif
#ifdef	F_GETLK64
	if (F_GETLK64 > f_local) f_local = F_GETLK64;
#endif
#ifdef	F_GETOWN
	if (F_GETOWN > f_local) f_local = F_GETOWN;
#endif
#ifdef	F_GETOWN_EX
	if (F_GETOWN_EX > f_local) f_local = F_GETOWN_EX;
#endif
#ifdef	F_GETSIG
	if (F_GETSIG > f_local) f_local = F_GETSIG;
#endif
#ifdef	F_NOTIFY
	if (F_NOTIFY > f_local) f_local = F_NOTIFY;
#endif
#ifdef	F_SETLEASE
	if (F_SETLEASE > f_local) f_local = F_SETLEASE;
#endif
#ifdef	F_SETLK64
	if (F_SETLK64 > f_local) f_local = F_SETLK64;
#endif
#ifdef	F_SETLKW64
	if (F_SETLKW64 > f_local) f_local = F_SETLKW64;
#endif
#ifdef	F_SETOWN
	if (F_SETOWN > f_local) f_local = F_SETOWN;
#endif
#ifdef	F_SETOWN_EX
	if (F_SETOWN_EX > f_local) f_local = F_SETOWN_EX;
#endif
#ifdef	F_SETSIG
	if (F_SETSIG > f_local) f_local = F_SETSIG;
#endif
#ifdef	F_SHLCK
	if (F_SHLCK > f_local) f_local = F_SHLCK;
#endif
#ifdef	F_SHARE
	if (F_SHARE > f_local) f_local = F_SHARE;
#endif
#ifdef	F_UNSHARE
	if (F_UNSHARE > f_local) f_local = F_UNSHARE;
#endif
#ifdef	F_BADFD /* Create Poison FD */
	if (F_BADFD > f_local) f_local = F_BADFD;
#endif

#if	NEED_F
#if	_lib_fcntl
	printf("#define _lib_fcntl	1\n");
#endif
	printf("#define _ast_F_LOCAL	%d\n", f_local + 1);
#ifndef	F_DUPFD_CLOEXEC
	printf("#define F_DUPFD_CLOEXEC	%d	/* ast extension */\n", ++f_local);
#endif
	printf("\n");
#endif

#ifndef	O_APPEND
#define NEED_O	1
#else
	if (O_APPEND > o_local) o_local = O_APPEND;
#endif
#ifndef	O_CREAT
#define NEED_O	1
#else
	if (O_CREAT > o_local) o_local = O_CREAT;
#endif
#ifndef	O_EXCL
#define NEED_O	1
#else
	if (O_EXCL > o_local) o_local = O_EXCL;
#endif
#ifndef	O_NOCTTY
#ifdef	TIOCNOTTY
#define NEED_O	1
#endif
#else
	if (O_NOCTTY > o_local) o_local = O_NOCTTY;
#endif
#ifndef	O_NONBLOCK
#define NEED_O	1
#else
	if (O_NONBLOCK > o_local) o_local = O_NONBLOCK;
#endif
#ifndef	O_RDONLY
#define NEED_O	1
#endif
#ifndef	O_RDWR
#define NEED_O	1
#endif
#ifndef	O_TRUNC
#define NEED_O	1
#else
	if (O_TRUNC > o_local) o_local = O_TRUNC;
#endif
#ifndef	O_WRONLY
#define NEED_O	1
#endif

#ifdef O_BLKSEEK
	if (O_BLKSEEK > o_local) o_local = O_BLKSEEK;
#endif
#ifdef O_LARGEFILE
	if (O_LARGEFILE > o_local) o_local = O_LARGEFILE;
#endif
#ifdef O_LARGEFILE128
	if (O_LARGEFILE128 > o_local) o_local = O_LARGEFILE128;
#endif
#ifdef O_NOATIME
	if (O_NOATIME > o_local) o_local = O_NOATIME;
#endif
#ifdef O_NOFOLLOW
	if (O_NOFOLLOW > o_local) o_local = O_NOFOLLOW;
#endif
#ifdef O_NOLINKS
	if (O_NOLINKS > o_local) o_local = O_NOLINKS;
#endif
#ifdef O_PRIV
	if (O_PRIV > o_local) o_local = O_PRIV;
#endif
#ifdef O_DSYNC
	if (O_DSYNC > o_local) o_local = O_DSYNC;
#endif
#ifdef O_RSYNC
	if (O_RSYNC > o_local) o_local = O_RSYNC;
#endif
#ifdef O_SYNC
	if (O_SYNC > o_local) o_local = O_SYNC;
#endif
#ifdef O_XATTR
	if (O_XATTR > o_local) o_local = O_XATTR;
#endif
#ifdef O_DIRECT
	if (O_DIRECT > o_local) o_local = O_DIRECT;
#endif
#ifndef O_DIRECTORY
#define NEED_O	1
#else
	if (O_DIRECTORY > o_local) o_local = O_DIRECTORY;
#endif
#ifdef O_OPENDIR
	if (O_OPENDIR > o_local) o_local = O_OPENDIR;
#endif
#ifndef O_SEARCH
#define NEED_O	1
#else
	if (O_SEARCH > o_local) o_local = O_SEARCH;
#endif
#ifdef O_PATH
	if (O_PATH > o_local) o_local = O_PATH;
#endif
#ifdef O_EXEC
	if (O_EXEC > o_local) o_local = O_EXEC;
#endif
#ifndef O_CLOEXEC
#define NEED_O	1
#else
	if (O_CLOEXEC > o_local) o_local = O_CLOEXEC;
#endif
#ifndef O_NDELAY
#define NEED_O	1
#else
	if (O_NDELAY > o_local) o_local = O_NDELAY;
#endif
#ifdef O_TTY_INIT
	if (O_TTY_INIT > o_local) o_local = O_TTY_INIT;
#endif

	o_local = hibit(o_local);
	printf("#define _ast_O_LOCAL		0%o\n", o_local<<1);
#if	NEED_O
#ifndef	O_RDONLY
	printf("#define O_RDONLY		0\n");
#endif
#ifndef	O_WRONLY
	printf("#define O_WRONLY		1\n");
#endif
#ifndef	O_RDWR
	printf("#define O_RDWR			2\n");
#endif
#ifndef	O_APPEND
	printf("#define O_APPEND		0%o	/* ast extension */\n", o_local <<= 1);
#endif
#ifndef	O_CREAT
	printf("#define O_CREAT			0%o	/* ast extension */\n", o_local <<= 1);
#endif
#ifndef	O_EXCL
	printf("#define O_EXCL			0%o	/* ast extension */\n", o_local <<= 1);
#endif
#ifndef	O_NOCTTY
#ifdef	TIOCNOTTY
	printf("#define O_NOCTTY		0%o	/* ast extension */\n", o_local <<= 1);
#endif
#endif
#ifndef	O_NDELAY
	printf("#define O_NDELAY		0%o	/* ast extension */\n", o_local <<= 1);
#endif
#ifndef	O_NONBLOCK
	printf("#define O_NONBLOCK		0%o	/* ast extension */\n", o_local <<= 1);
#endif
#ifndef	O_TRUNC
	printf("#define O_TRUNC			0%o	/* ast extension */\n", o_local <<= 1);
#endif
#endif
#ifndef	O_ACCMODE
	printf("#define O_ACCMODE		(O_RDONLY|O_WRONLY|O_RDWR)\n");
#endif
#ifndef	O_NOCTTY
#ifndef	TIOCNOTTY
	printf("#define O_NOCTTY		0\n");
#endif
#endif
#ifndef	O_BINARY
	printf("#define O_BINARY		0\n");
#endif
#ifndef	O_CLOEXEC
	printf("#define O_CLOEXEC		0%o	/* ast extension */\n", o_local <<= 1);
#endif
#ifndef	O_SEARCH
#ifdef O_PATH
	/*
	 * O_PATH is a Linux's variation of O_SEARCH. Since this is treated
	 * as extension it may not be available without _GNU_SOURCE.
	 * Even even with _GNU_SOURCE some Linux platforms have bugs in their
	 * headers which prevents the definition of O_PATH in some hideous
	 * cases. To prevent all this mess we just grab the octal value and
	 * define it as O_SEARCH.
	 * Do not change this. You'll end-up in hell, together with the Linux
	 * headers. Quickly. - rm
	 * why not play nice and just provide O_SEARCH - gsf
	 */
	printf("#ifdef O_PATH\n");
	printf("#define O_SEARCH		O_PATH\n");
	printf("#else\n");
	printf("#define O_SEARCH		0%o /* O_PATH */\n", (unsigned int)O_PATH);
	printf("#endif\n");
#else
	printf("#define O_SEARCH		0\n");
#endif
#endif
#ifdef O_DIRECTORY
	/* O_DIRECTORY -- see O_PATH caveats above */
	printf("#ifndef O_DIRECTORY\n");
	printf("#define O_DIRECTORY		0%o\n", (int)O_DIRECTORY);
	printf("#endif\n");
#else
#ifdef O_OPENDIR
	printf("#define O_DIRECTORY		0%o /* O_OPENDIR */\n", O_OPENDIR);
#else
	printf("#define O_DIRECTORY		0%o	/* ast extension */\n", o_local <<= 1);
#endif
#endif
#ifdef O_CLOEXEC
	printf("#ifndef O_CLOEXEC\n");
	printf("#define O_CLOEXEC		0%o	/* just in case _foo_SOURCE or _USE_foo omitted */\n", (int)O_CLOEXEC);
	printf("#endif\n");
#endif
#ifdef O_DIRECT
	printf("#ifndef O_DIRECT\n");
	printf("#define O_DIRECT		0%o	/* just in case _foo_SOURCE or _USE_foo omitted */\n", (int)O_DIRECT);
	printf("#endif\n");
#endif
#ifdef O_NOFOLLOW
	printf("#ifndef O_NOFOLLOW\n");
	printf("#define O_NOFOLLOW		0%o	/* just in case _foo_SOURCE or _USE_foo omitted */\n", (int)O_NOFOLLOW);
	printf("#endif\n");
#endif
#ifdef O_NOATIME
	printf("#ifndef O_NOATIME\n");
	printf("#define O_NOATIME		0%o	/* just in case _foo_SOURCE or _USE_foo omitted */\n", (int)O_NOATIME);
	printf("#endif\n");
#endif
#ifndef	O_INTERCEPT
	printf("#define O_INTERCEPT		0%o	/* ast extension */\n", o_local <<= 1);
#endif
#ifndef	O_TEMPORARY
	printf("#define O_TEMPORARY		0\n");
#endif
#ifndef	O_TEXT
	printf("#define O_TEXT			0\n");
#endif
#if !defined(SOCK_CLOEXEC) || !defined(SOCK_NONBLOCK)
	printf("\n");
#ifndef SOCK_CLOEXEC
	printf("#define _ast_SOCK_CLOEXEC	1\n");
	printf("#define SOCK_CLOEXEC		02000000 /* ast extension */\n");
#endif
#ifndef SOCK_NONBLOCK
	printf("#define _ast_SOCK_NONBLOCK	1\n");
	printf("#define SOCK_NONBLOCK		04000	/* ast extension */\n");
#endif
#endif
#ifdef F_DUPFD_CLOEXEC
	printf("#ifndef F_DUPFD_CLOEXEC\n");
	printf("#define F_DUPFD_CLOEXEC		0%o	/* just in case _foo_SOURCE or _USE_foo omitted */\n", (int)F_DUPFD_CLOEXEC);
	printf("#endif\n");
#endif
	printf("#define F_dupfd_cloexec		F_DUPFD_CLOEXEC /* OBSOLETE */\n");
	printf("#define O_cloexec		O_CLOEXEC /* OBSOLETE*/\n");
#if !defined(AT_FDCWD) || !defined(AT_SYMLINK_NOFOLLOW) || !defined(AT_REMOVEDIR) || !defined(AT_SYMLINK_FOLLOW) || !defined(AT_EACCESS)
	printf("\n");
#ifndef AT_FDCWD
	/* AT_FDCWD must be < -256 for portability reasons */
	printf("#define AT_FDCWD		-666	/* ast extension */\n");
#endif
#ifndef AT_SYMLINK_NOFOLLOW
	printf("#define AT_SYMLINK_NOFOLLOW	0x100	/* ast extension */\n");
#endif
#ifndef AT_REMOVEDIR
	printf("#define AT_REMOVEDIR		0x200	/* ast extension*/\n");
#endif
#ifndef AT_SYMLINK_FOLLOW
	printf("#define AT_SYMLINK_FOLLOW	0x400	/* ast extension*/\n");
#endif
#ifndef AT_EACCESS
	printf("#define AT_EACCESS		0x800	/* ast extension*/\n");
#endif
#endif
	printf("\n");
	printf("#include <ast_fs.h>\n");
	printf("#if _typ_off64_t\n");
	printf("#undef	off_t\n");
	printf("#define	off_t		off64_t\n");
	printf("#endif\n");
	printf("#if _lib_fstat64\n");
	printf("#define fstat		fstat64\n");
	printf("#endif\n");
	printf("#if _lib_fstatat64\n");
	printf("#define fstatat		fstatat64\n");
	printf("#endif\n");
	printf("#if _lib_lstat64\n");
	printf("#define lstat		lstat64\n");
	printf("#endif\n");
	printf("#if _lib_stat64\n");
	printf("#define stat		stat64\n");
	printf("#endif\n");
	printf("#if _lib_creat64\n");
	printf("#define creat		creat64\n");
	printf("#endif\n");
	printf("#if _lib_mmap64\n");
	printf("#define mmap		mmap64\n");
	printf("#endif\n");
	printf("#if _lib_open64\n");
	printf("#undef	open\n");
	printf("#define open		open64\n");
	printf("#endif\n");

	printf("\n");
	printf("#if _BLD_ast && defined(__EXPORT__)\n");
	printf("\n");
	printf("#endif\n");
	printf("#if !_lib_faccessat\n");
	printf("extern int	faccessat(int, const char*, int, int);\n");
	printf("#endif\n");
	printf("#if !_lib_fchmodat\n");
	printf("extern int	fchmodat(int, const char*, mode_t, int);\n");
	printf("#endif\n");
	printf("#if !_lib_fchownat\n");
	printf("extern int	fchownat(int, const char*, uid_t, gid_t, int);\n");
	printf("#endif\n");
	printf("#if !_lib_fstatat\n");
	printf("struct stat;\n");
	printf("extern int	fstatat(int, const char*, struct stat*, int);\n");
	printf("#endif\n");
	printf("#if !_lib_linkat\n");
	printf("extern int	linkat(int, const char*, int, const char*, int);\n");
	printf("#endif\n");
	printf("#if !_lib_mkdirat\n");
	printf("extern int	mkdirat(int, const char*, mode_t);\n");
	printf("#endif\n");
	printf("#if !_lib_mkfifoat\n");
	printf("extern int	mkfifoat(int, const char*, mode_t);\n");
	printf("#endif\n");
	printf("#if !_lib_mknodat\n");
	printf("extern int	mknodat(int, const char*, mode_t, dev_t);\n");
	printf("#endif\n");
	printf("#if !_lib_openat\n");
	printf("extern int	openat(int, const char*, int, ...);\n");
	printf("#endif\n");
	printf("#if !_lib_readlinkat\n");
	printf("extern int	readlinkat(int, const char*, char*, size_t);\n");
	printf("#endif\n");
	printf("#if !_lib_renameat\n");
	printf("extern int	renameat(int, const char*, int, const char*);\n");
	printf("#endif\n");
	printf("#if !_lib_symlinkat\n");
	printf("extern int	symlinkat(const char*, int, const char*);\n");
	printf("#endif\n");
	printf("#if !_lib_unlinkat\n");
	printf("extern int	unlinkat(int, const char*, int);\n");
	printf("#endif\n");
	printf("\n");
	printf("#undef	extern\n");

	return 0;
}
