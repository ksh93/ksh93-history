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
 * ast function intercepts
 */

#if !defined(_AST_INTERCEPT) && !_BLD_ast && _API_ast && _API_ast < 20130625 /* <ast_api.h> not in scope yet */
#define _AST_INTERCEPT			0
#endif

#ifndef _AST_INTERCEPT
#define _AST_INTERCEPT			1

#define AST_SERIAL_ENVIRON		1
#define AST_SERIAL_LOCALE		2
#define AST_SERIAL_RESTART		3

#define AST_SERIAL_get			0
#define AST_SERIAL_inc			1
#define AST_SERIAL_always		0xffffffff

#if _BLD_ast && defined(__EXPORT__)
#define extern		__EXPORT__
#endif

extern uint32_t		astserial(int, uint32_t);

#ifndef _AST_INTERCEPT_IMPLEMENT
#define _AST_INTERCEPT_IMPLEMENT	1

/* at*() intercepts */

#undef	faccessat
#define faccessat	ast_faccessat
extern int		ast_faccessat(int, const char*, mode_t, int);

#undef	fchmodat
#define	fchmodat	ast_fchmodat
extern int		ast_fchmodat(int, const char*, mode_t, int);

#undef	fchownat
#define fchownat	ast_fchownat
extern int		ast_fchownat(int, const char*, uid_t, gid_t, int);

#undef	fstatat
#define fstatat		ast_fstatat
extern int		ast_fstatat(int, const char*, struct stat*, int);

#undef	linkat
#define linkat 		ast_linkat
extern int		ast_linkat(int, const char*, int, const char*, int);

#undef	mkdirat
#define mkdirat		ast_mkdirat
extern int		ast_mkdirat(int, const char*, mode_t);

#undef	mkfifoat
#define mkfifoat	ast_mkfifoat
extern int		ast_mkfifoat(int, const char*, mode_t);

#undef	mknodat
#define mknodat		ast_mknodat
extern int		ast_mknodat(int, const char*, mode_t, dev_t);

#undef	openat
#define openat		ast_openat
extern int		ast_openat(int, const char*, int, ...);

#undef	readlinkat
#define readlinkat	ast_readlinkat
extern ssize_t	ast_readlinkat(int, const char*, void*, size_t);

#undef	symlinkat
#define symlinkat	ast_symlinkat
extern int		ast_symlinkat(const char*, int, const char*);

#undef	unlinkat
#define unlinkat	ast_unlinkat
extern int		ast_unlinkat(int, const char*, int);

/* restart intercepts */

#undef	access
#define access		ast_access
extern int		ast_access(const char*, int);

#undef	chdir
#define chdir		ast_chdir
extern int		ast_chdir(const char*);

#undef	chmod
#define chmod		ast_chmod
extern int		ast_chmod(const char*, mode_t);

#undef	chown
#define chown		ast_chown
extern int		ast_chown(const char*, uid_t, gid_t);

#undef	close
#define close		ast_close
extern int		ast_close(int);

#undef	creat
#define creat		ast_creat
extern int		ast_creat(const char*, mode_t);

#undef	dup
#define dup		ast_dup
extern int		ast_dup(int);

#undef	dup2
#define dup2		ast_dup2
extern int		ast_dup2(int, int);

#undef	eaccess
#define eaccess		ast_eaccess
extern int		ast_eccess(const char*, int);

#undef	fchdir
#define fchdir		ast_fchdir
extern int		ast_fchdir(int);

#undef	fchmod
#define fchmod		ast_fchmod
extern int		ast_fchmod(int, mode_t);

#undef	fchown
#define fchown		ast_fchown
extern int		ast_fchown(int, gid_t, uid_t);

/* XXX ast_fcntl() will at least work for all ast usage */

#undef	fcntl
#define fcntl		ast_fcntl
extern int		ast_fcntl(int, int, ...);

#undef	fstat
#define fstat		ast_fstat
extern int		ast_fstat(int, struct stat*);

#undef	ftruncate
#define ftruncate	ast_ftruncate
extern int		ast_ftruncate(int, off_t);

/* ast_ioctl brought in scope by <ast_ioctl.h> */

#undef	lchmod
#define lchmod		ast_lchmod
extern int		ast_lchmod(const char*, mode_t);

#undef	lchown
#define lchown		ast_lchown
extern int		ast_chown(const char*, uid_t, gid_t);

#undef	link
#define link		ast_link
extern int		ast_link(const char*, const char*);

#undef	lstat
#define lstat		ast_lstat
extern int		ast_lstat(const char*, struct stat*);

#undef	mkdir
#define mkdir		ast_mkdir
extern int		ast_mkdir(const char*, mode_t);

#undef	mkfifo
#define mkfifo		ast_mkfifo
extern int		ast_mkfifo(const char*, mode_t);

#undef	mknod
#define mknod		ast_mknod
extern int		ast_mknod(const char*, mode_t, dev_t);

#undef	open
#define open		ast_open
extern int		ast_open(const char*, int, ...);

#undef	pipe
#define pipe		ast_pipe
extern int		ast_pipe(int[2]);

#undef	pipe2
#define pipe2		ast_pipe2
extern int		ast_pipe2(int[2], int);

#undef	readlink
#define readlink	ast_readlink
extern int		ast_readlink(const char*, char*, size_t);

#undef	remove
#define remove		ast_remove
extern int		ast_remove(const char*);

#undef	rename
#define rename		ast_rename
extern int		ast_rename(const char*, const char*);

#undef	rmdir
#define rmdir		ast_rmdir
extern int		ast_rmdir(const char*);

/* stat vs stat64 vs struct stat makes life interesting */
#ifdef stat
#undef	stat64
#define stat64(p,s)	ast_stat(p,s)
#else
#undef	stat
#define stat(p,s)	ast_stat(p,s)
#endif
extern int		ast_stat(const char*, struct stat*);

#undef	symlink
#define symlink		ast_symlink
extern int		ast_symlink(const char*, const char*);

#undef	unlink
#define unlink		ast_unlink
extern int		ast_unlink(const char*);

/* socket intercepts -- prototypes done by <sys/socket.h>! */

#if _sys_socket

#undef	accept
#define accept		ast_accept

#undef	accept4
#define accept4		ast_accept4
#if !_lib_accept4
extern int		ast_accept4(int, void*, void*, int);
#endif

#undef	socket
#define socket		ast_socket

#undef	socketpair
#define socketpair	ast_socketpair

#endif

#endif

#undef	extern

#endif
