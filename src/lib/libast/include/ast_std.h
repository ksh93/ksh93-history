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
*      If you have copied this software without agreeing       *
*      to the terms of the license you are infringing on       *
*         the license and copyright and are violating          *
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
 * Advanced Software Technology Library
 * AT&T Research
 *
 * a union of the following standard headers that works
 *
 *	<limits.h>
 *	<stdarg.h>
 *	<stddef.h>
 *	<stdlib.h>
 *	<sys/types.h>
 *	<string.h>
 *	<unistd.h>
 *	<fcntl.h>
 *	<locale.h>
 *
 * the following ast implementation specific headers are also included
 * these do not stomp on the std namespace
 *
 *	<ast_botch.h>
 *	<ast_common.h>
 *	<ast_fcntl.h>
 *	<ast_hdr.h>
 *	<ast_lib.h>
 *	<ast_types.h>
 *	<ast_unistd.h>
 */

#ifndef _AST_STD_H
#define _AST_STD_H

#include <ast_common.h>
#include <ast_lib.h>
#include <getopt.h>	/* <stdlib.h> does this */

#if (__hppa || __mips == 2) && !defined(_NO_LARGEFILE64_SOURCE)
#define	_NO_LARGEFILE64_SOURCE	1
#endif
#if !defined(_NO_LARGEFILE64_SOURCE) && _typ_off64_t && _lib_lseek64 && _lib_stat64
#if !defined(_LARGEFILE64_SOURCE)
#define _LARGEFILE64_SOURCE	1
#endif
#else
#undef	_LARGEFILE64_SOURCE
#undef	_typ_off64_t
#undef	_typ_struct_dirent64
#undef	_lib_creat64
#undef	_lib_fstat64
#undef	_lib_fstatvfs64
#undef	_lib_ftruncate64
#undef	_lib_lseek64
#undef	_lib_lstat64
#undef	_lib_mmap64
#undef	_lib_open64
#undef	_lib_readdir64
#undef	_lib_stat64
#undef	_lib_statvfs64
#undef	_lib_truncate64
#endif

#if _BLD_ast
#define _BLD_cdt	1
#define _BLD_sfio	1
#if !_WIN32
#define _BLD_vmalloc	1
#endif
#endif

#include <ast_hdr.h>

#ifdef	_SFSTDIO_H
#define _SKIP_SFSTDIO_H
#else
#define _SFSTDIO_H
#ifndef FILE
#ifndef _SFIO_H
struct _sfio_s;
#endif
#define FILE		struct _sfio_s
#ifndef	__FILE_typedef
#define __FILE_typedef	1
#endif
#endif
#endif

#if defined(__STDPP__directive) && defined(__STDPP__hide)
#if !_std_def_calloc
__STDPP__directive pragma pp:hide calloc
#endif
#if !_std_def_free
__STDPP__directive pragma pp:hide free
#endif
#if !_std_def_malloc
__STDPP__directive pragma pp:hide malloc
#endif
#if !_std_def_realloc
__STDPP__directive pragma pp:hide realloc
#endif
#if !_std_def_valloc
__STDPP__directive pragma pp:hide valloc
#endif
__STDPP__directive pragma pp:hide bcopy bzero execl execle execlp execv
__STDPP__directive pragma pp:hide execve execvp execvpe getcwd
__STDPP__directive pragma pp:hide getopt getsubopt putenv
__STDPP__directive pragma pp:hide realpath setenv setpgrp sleep spawnlp
__STDPP__directive pragma pp:hide spawnve spawnveg spawnvp spawnvpe
__STDPP__directive pragma pp:hide strdup vfprintf vprintf vsprintf
#else
#if !_std_def_calloc
#define calloc		______calloc
#endif
#if !_std_def_free
#define free		______free
#endif
#if !_std_def_malloc
#define malloc		______malloc
#endif
#if !_std_def_realloc
#define realloc		______realloc
#endif
#if !_std_def_valloc
#define valloc		______valloc
#endif
#define bcopy		______bcopy
#define bzero		______bzero
#define execl		______execl
#define execle		______execle
#define execlp		______execlp
#define execv		______execv
#define execve		______execve
#define execvp		______execvp
#define execvpe		______execvpe
#define getcwd		______getcwd
#define getopt		______getopt
#define getsubopt	______getsubopt
#define putenv		______putenv
#define realpath	______realpath
#define setenv		______setenv
#define setpgrp		______setpgrp
#define sleep		______sleep
#define spawnlp		______spawnlp
#define spawnve		______spawnve
#define spawnveg	______spawnveg
#define spawnvp		______spawnvp
#define spawnvpe	______spawnvpe
#define strdup		______strdup
#define vfprintf	______vfprintf
#define vprintf		______vprintf
#define vsprintf	______vsprintf
#endif

#include <sys/types.h>
#include <stdarg.h>

#undef	_ast_va_list
#define _ast_va_list	va_list

#if defined(__STDPP__directive) && defined(__STDPP__initial)
__STDPP__directive pragma pp:initial
#endif
#include <limits.h>
#if defined(__STDPP__directive) && defined(__STDPP__initial)
__STDPP__directive pragma pp:noinitial
#endif

#if defined(__STDC__) && !defined(__USE_FIXED_PROTOTYPES__)
#define __USE_FIXED_PROTOTYPES__	1	/* kick gcc out of the past */
#endif

#if defined(__STDC__) || defined(__cplusplus)|| defined(_std_stddef)

#include <stddef.h>

#endif

#ifndef offsetof
#define offsetof(type,member) ((size_t)&(((type*)0)->member))
#endif

#if defined(__STDC__) || !defined(__cplusplus) && defined(_std_stdlib)

#include <stdlib.h>

#else

#define EXIT_FAILURE	1
#define EXIT_SUCCESS	0
#define MB_CUR_MAX	1
#define RAND_MAX	32767

#endif

#ifdef	_SKIP_SFSTDIO_H
#undef	_SKIP_SFSTDIO_H
#else
#undef	_SFSTDIO_H
#undef	FILE
#endif

#if defined(__STDPP__directive) && defined(__STDPP__hide)
#if !_std_def_calloc
__STDPP__directive pragma pp:nohide calloc
#endif
#if !_std_def_free
__STDPP__directive pragma pp:nohide free
#endif
#if !_std_def_malloc
__STDPP__directive pragma pp:nohide malloc
#endif
#if !_std_def_realloc
__STDPP__directive pragma pp:nohide realloc
#endif
#if !_std_def_valloc
__STDPP__directive pragma pp:nohide valloc
#endif
__STDPP__directive pragma pp:nohide bcopy bzero execl execle execlp execv
__STDPP__directive pragma pp:nohide execve execvp execvpe getcwd
__STDPP__directive pragma pp:nohide getopt getsubopt putenv
__STDPP__directive pragma pp:nohide realpath setenv setpgrp sleep spawnlp
__STDPP__directive pragma pp:nohide spawnve spawnveg spawnvp spawnvpe
__STDPP__directive pragma pp:nohide strdup vfprintf vprintf vsprintf
#else
#if !_std_def_calloc
#undef	calloc	
#endif
#if !_std_def_free
#undef	free	
#endif
#if !_std_def_malloc
#undef	malloc	
#endif
#if !_std_def_realloc
#undef	realloc	
#endif
#if !_std_def_valloc
#undef	valloc	
#endif
#undef	bcopy
#undef	bzero
#undef	execl
#undef	execle
#undef	execlp
#undef	execv
#undef	execve
#undef	execvp
#undef	execvpe
#undef	getcwd
#undef	getopt
#undef	getsubopt
#undef	putenv
#undef	realpath
#undef	setenv
#undef	setpgrp
#undef	sleep
#undef	spawnlp
#undef	spawnve
#undef	spawnveg
#undef	spawnvp
#undef	spawnvpe
#undef	strdup
#undef	vfprintf
#undef	vprintf
#undef	vsprintf
#endif

#include <ast_types.h>

#if !defined(__STDC__) && ( defined(__cplusplus) || !defined(_std_stdlib) )

/* <stdlib.h> */

extern double		atof(const char*);
extern int		atoi(const char*);
extern long		atol(const char*);
extern double		strtod(const char*, char**);
extern long		strtol(const char*, char**, int);
extern unsigned long	strtoul(const char*, char**, int);

#ifdef _ast_int8_t
extern _ast_int8_t		strtoll(const char*, char**, int);
extern unsigned _ast_int8_t	strtoull(const char*, char**, int);
#else
extern long			strtoll(const char*, char**, int);
extern unsigned long		strtoull(const char*, char**, int);
#endif

extern int		rand(void);
extern void		srand(unsigned int);

extern void		abort(void);
extern int		atexit(void(*)(void));
extern void		exit(int);
extern char*		getenv(const char*);
extern char*		realpath(const char*, char*);
extern void		swab(const void*, void*, ssize_t);
extern int		system(const char*);

extern void*		bsearch(const void*, const void*, size_t, size_t,
		 		int(*)(const void*, const void*));
extern void		qsort(void*, size_t, size_t,
				int(*)(const void*, const void*));

extern int		abs(int);
extern div_t		div(int, int);
extern long		labs(long);
extern ldiv_t		ldiv(long, long);

extern int		mblen(const char*, size_t);
extern int		mbtowc(wchar_t*, const char*, size_t);
extern int		wctomb(char*, wchar_t);
extern size_t		mbstowcs(wchar_t*, const char*, size_t);
extern size_t		wcstombs(char*, const wchar_t*, size_t);

#else

#if !_lib_strtoll
#ifdef _ast_int8_t
extern _ast_int8_t		strtoll(const char*, char**, int);
#else
extern long			strtoll(const char*, char**, int);
#endif
#endif

#if !_lib_strtoull
#ifdef _ast_int8_t
extern unsigned _ast_int8_t	strtoull(const char*, char**, int);
#else
extern unsigned long		strtoull(const char*, char**, int);
#endif
#endif

#endif

#if !_std_def_calloc
extern void*		calloc(size_t, size_t);
#endif
#if !_std_def_free
extern void		free(void*);
#endif
#if !_std_def_malloc
extern void*		malloc(size_t);
#endif
#if !_std_def_realloc
extern void*		realloc(void*, size_t);
#endif

/* <string.h> */

extern void*		memccpy(void*, const void*, int, size_t);
extern void*		memchr(const void*, int, size_t);
extern int		memcmp(const void*, const void*, size_t);
extern void*		memcpy(void*, const void*, size_t);
extern void*		memmove(void*, const void*, size_t);
extern void*		memset(void*, int, size_t);
extern int		strcasecmp(const char*, const char*);
extern char*		strcat(char*, const char*);
extern char*		strchr(const char*, int);
extern int		strcmp(const char*, const char*);
extern int		strcoll(const char*, const char*);
extern char*		strcpy(char*, const char*);
extern size_t		strcspn(const char*, const char*);
extern size_t		strlen(const char*);
extern int		strncasecmp(const char*, const char*, size_t);
extern char*		strncat(char*, const char*, size_t);
extern int		strncmp(const char*, const char*, size_t);
extern char*		strncpy(char*, const char*, size_t);
extern char*		strpbrk(const char*, const char*);
extern char*		strrchr(const char*, int);
extern size_t		strspn(const char*, const char*);
extern char*		strstr(const char*, const char*);
extern char*		strtok(char*, const char*);
extern size_t		strxfrm(char*, const char*, size_t);

#if defined(__STDPP__directive) && defined(__STDPP__ignore)

__STDPP__directive pragma pp:ignore "libc.h"
__STDPP__directive pragma pp:ignore "memory.h"
__STDPP__directive pragma pp:ignore "stdlib.h"
__STDPP__directive pragma pp:ignore "string.h"
__STDPP__directive pragma pp:ignore "strings.h"

#else

#ifndef _libc_h
#define _libc_h
#endif
#ifndef _libc_h_
#define _libc_h_
#endif
#ifndef __libc_h
#define __libc_h
#endif
#ifndef __libc_h__
#define __libc_h__
#endif
#ifndef _LIBC_H
#define _LIBC_H
#endif
#ifndef _LIBC_H_
#define _LIBC_H_
#endif
#ifndef __LIBC_H
#define __LIBC_H
#endif
#ifndef __LIBC_H__
#define __LIBC_H__
#endif
#ifndef _LIBC_INCLUDED
#define _LIBC_INCLUDED
#endif
#ifndef __LIBC_INCLUDED
#define __LIBC_INCLUDED
#endif
#ifndef _H_LIBC
#define _H_LIBC
#endif
#ifndef __H_LIBC
#define __H_LIBC
#endif

#ifndef _memory_h
#define _memory_h
#endif
#ifndef _memory_h_
#define _memory_h_
#endif
#ifndef __memory_h
#define __memory_h
#endif
#ifndef __memory_h__
#define __memory_h__
#endif
#ifndef _MEMORY_H
#define _MEMORY_H
#endif
#ifndef _MEMORY_H_
#define _MEMORY_H_
#endif
#ifndef __MEMORY_H
#define __MEMORY_H
#endif
#ifndef __MEMORY_H__
#define __MEMORY_H__
#endif
#ifndef _MEMORY_INCLUDED
#define _MEMORY_INCLUDED
#endif
#ifndef __MEMORY_INCLUDED
#define __MEMORY_INCLUDED
#endif
#ifndef _H_MEMORY
#define _H_MEMORY
#endif
#ifndef __H_MEMORY
#define __H_MEMORY
#endif

#ifndef _stdlib_h
#define _stdlib_h
#endif
#ifndef _stdlib_h_
#define _stdlib_h_
#endif
#ifndef __stdlib_h
#define __stdlib_h
#endif
#ifndef __stdlib_h__
#define __stdlib_h__
#endif
#ifndef _STDLIB_H
#define _STDLIB_H
#endif
#ifndef _STDLIB_H_
#define _STDLIB_H_
#endif
#ifndef __STDLIB_H
#define __STDLIB_H
#endif
#ifndef __STDLIB_H__
#define __STDLIB_H__
#endif
#ifndef _STDLIB_INCLUDED
#define _STDLIB_INCLUDED
#endif
#ifndef __STDLIB_INCLUDED
#define __STDLIB_INCLUDED
#endif
#ifndef _H_STDLIB
#define _H_STDLIB
#endif
#ifndef __H_STDLIB
#define __H_STDLIB
#endif

#ifndef _string_h
#define _string_h
#endif
#ifndef _string_h_
#define _string_h_
#endif
#ifndef __string_h
#define __string_h
#endif
#ifndef __string_h__
#define __string_h__
#endif
#ifndef _STRING_H
#define _STRING_H
#endif
#ifndef _STRING_H_
#define _STRING_H_
#endif
#ifndef __STRING_H
#define __STRING_H
#endif
#ifndef __STRING_H__
#define __STRING_H__
#endif
#ifndef _STRING_INCLUDED
#define _STRING_INCLUDED
#endif
#ifndef __STRING_INCLUDED
#define __STRING_INCLUDED
#endif
#ifndef _H_STRING
#define _H_STRING
#endif
#ifndef __H_STRING
#define __H_STRING
#endif

#ifndef _strings_h
#define _strings_h
#endif
#ifndef _strings_h_
#define _strings_h_
#endif
#ifndef __strings_h
#define __strings_h
#endif
#ifndef __strings_h__
#define __strings_h__
#endif
#ifndef _STRINGS_H
#define _STRINGS_H
#endif
#ifndef _STRINGS_H_
#define _STRINGS_H_
#endif
#ifndef __STRINGS_H
#define __STRINGS_H
#endif
#ifndef __STRINGS_H__
#define __STRINGS_H__
#endif
#ifndef _STRINGS_INCLUDED
#define _STRINGS_INCLUDED
#endif
#ifndef __STRINGS_INCLUDED
#define __STRINGS_INCLUDED
#endif
#ifndef _H_STRINGS
#define _H_STRINGS
#endif
#ifndef __H_STRINGS
#define __H_STRINGS
#endif

#endif

#include <ast_fcntl.h>

#if _typ_off64_t
#undef	off_t
#endif

/* <unistd.h> */

#if _WIN32
#include <unistd.h>
#else
#include <ast_unistd.h>
#endif
#include <ast_botch.h>

#ifndef STDIN_FILENO
#define	STDIN_FILENO	0
#define	STDOUT_FILENO	1
#define	STDERR_FILENO	2
#endif

#ifndef NULL
#define	NULL		0
#endif

#ifndef SEEK_SET
#define	SEEK_SET	0
#define	SEEK_CUR	1
#define	SEEK_END	2
#endif

#ifndef	F_OK
#define	F_OK		0
#define	X_OK		1
#define	W_OK		2
#define	R_OK		4
#endif

extern void		_exit(int);
extern int		access(const char*, int);
extern unsigned		alarm(unsigned);
extern int		chdir(const char*);
extern int		chown(const char*, uid_t, gid_t);
extern int		close(int);
extern size_t		confstr(int, char*, size_t);
extern int		dup(int);
extern int		dup2(int, int);
extern int		execl(const char*, const char*, ...);
extern int		execle(const char*, const char*, ...);
extern int		execlp(const char*, const char*, ...);
extern int		execv(const char*, char* const[]);
extern int		execve(const char*, char* const[], char* const[]);
extern int		execvp(const char*, char* const[]);
extern int		execvpe(const char*, char* const[], char* const[]);
extern pid_t		fork(void);
extern long		fpathconf(int, int);
extern int		ftruncate(int, off_t);
extern char*		getcwd(char*, size_t);
extern gid_t		getegid(void);
extern uid_t		geteuid(void);
extern gid_t		getgid(void);
extern int		getgroups(int, gid_t[]);
extern char*		getlogin(void);
extern pid_t		getpgrp(void);
extern pid_t		getpid(void);
extern pid_t		getppid(void);
extern char*		gettxt(const char*, const char*);
extern uid_t		getuid(void);
extern int		isatty(int);
extern int		link(const char*, const char*);
extern off_t		lseek(int, off_t, int);
extern long		pathconf(const char*, int);
extern int		pause(void);
extern int		pipe(int[]);
extern ssize_t		read(int, void*, size_t);
extern int		rmdir(const char*);
extern int		setgid(gid_t);
extern int		setpgid(pid_t, pid_t);
extern pid_t		setsid(void);
extern int		setuid(uid_t);
extern unsigned		sleep(unsigned int);
extern pid_t		spawnlp(const char*, const char*, ...);
extern pid_t		spawnve(const char*, char* const[], char* const[]);
extern pid_t		spawnveg(const char*, char* const[], char* const[], pid_t);
extern pid_t		spawnvp(const char*, char* const[]);
extern pid_t		spawnvpe(const char*, char* const[], char* const[]);
extern long		sysconf(int);
extern pid_t		tcgetpgrp(int);
extern int		tcsetpgrp(int, pid_t);
extern int		truncate(const char*, off_t);
extern char*		ttyname(int);
extern int		unlink(const char*);
extern ssize_t		write(int, const void*, size_t);

#if _BLD_ast && defined(__EXPORT__)
#define extern		__EXPORT__
#endif

/*
 * yes, we don't trust anyone's interpretation but our own
 */

#undef	strerror
#define strerror	_ast_strerror

extern char*		strerror(int);

#if !_WIN32

#undef	confstr
#define confstr		_ast_confstr
#undef	fpathconf
#define fpathconf	_ast_fpathconf
#undef	pathconf
#define pathconf	_ast_pathconf
#undef	sysconf
#define sysconf		_ast_sysconf

extern size_t		confstr(int, char*, size_t);
extern long		fpathconf(int, int);
extern long		pathconf(const char*, int);
extern long		sysconf(int);

#endif

extern char*		strdup(const char*);

#undef	extern

/*
 * now activate the guards for headers already covered
 */

#if defined(__STDPP__directive) && defined(__STDPP__ignore)

__STDPP__directive pragma pp:ignore "fcntl.h"
__STDPP__directive pragma pp:ignore "unistd.h"
__STDPP__directive pragma pp:ignore "sys/unistd.h"

#else

#ifndef _fcntl_h
#define _fcntl_h
#endif
#ifndef _fcntl_h_
#define _fcntl_h_
#endif
#ifndef __fcntl_h
#define __fcntl_h
#endif
#ifndef __fcntl_h__
#define __fcntl_h__
#endif
#ifndef _FCNTL_H
#define _FCNTL_H
#endif
#ifndef _FCNTL_H_
#define _FCNTL_H_
#endif
#ifndef __FCNTL_H
#define __FCNTL_H
#endif
#ifndef __FCNTL_H__
#define __FCNTL_H__
#endif
#ifndef _FCNTL_INCLUDED
#define _FCNTL_INCLUDED
#endif
#ifndef __FCNTL_INCLUDED
#define __FCNTL_INCLUDED
#endif
#ifndef _H_FCNTL
#define _H_FCNTL
#endif
#ifndef __H_FCNTL
#define __H_FCNTL
#endif

#ifndef _unistd_h
#define _unistd_h
#endif
#ifndef _unistd_h_
#define _unistd_h_
#endif
#ifndef __unistd_h
#define __unistd_h
#endif
#ifndef __unistd_h__
#define __unistd_h__
#endif
#ifndef _UNISTD_H
#define _UNISTD_H
#endif
#ifndef _UNISTD_H_
#define _UNISTD_H_
#endif
#ifndef __UNISTD_H
#define __UNISTD_H
#endif
#ifndef __UNISTD_H__
#define __UNISTD_H__
#endif
#ifndef _UNISTD_INCLUDED
#define _UNISTD_INCLUDED
#endif
#ifndef __UNISTD_INCLUDED
#define __UNISTD_INCLUDED
#endif
#ifndef _H_UNISTD
#define _H_UNISTD
#endif
#ifndef __H_UNISTD
#define __H_UNISTD
#endif
#ifndef _SYS_UNISTD_H
#define _SYS_UNISTD_H
#endif

#endif

#if defined(__cplusplus)

#if defined(__STDPP__directive) && defined(__STDPP__ignore)

__STDPP__directive pragma pp:ignore "sysent.h"

#else

#ifndef _sysent_h
#define _sysent_h
#endif
#ifndef _sysent_h_
#define _sysent_h_
#endif
#ifndef __sysent_h
#define __sysent_h
#endif
#ifndef __sysent_h__
#define __sysent_h__
#endif
#ifndef _SYSENT_H
#define _SYSENT_H
#endif
#ifndef _SYSENT_H_
#define _SYSENT_H_
#endif
#ifndef __SYSENT_H
#define __SYSENT_H
#endif
#ifndef __SYSENT_H__
#define __SYSENT_H__
#endif
#ifndef _SYSENT_INCLUDED
#define _SYSENT_INCLUDED
#endif
#ifndef __SYSENT_INCLUDED
#define __SYSENT_INCLUDED
#endif
#ifndef _H_SYSENT
#define _H_SYSENT
#endif
#ifndef __H_SYSENT
#define __H_SYSENT
#endif

#endif

#endif

/* locale stuff */

#if _hdr_locale

#include <locale.h>

#if _sys_localedef

#include <sys/localedef.h>

#endif

#endif

#if _BLD_ast && defined(__EXPORT__)
#define extern		__EXPORT__
#endif

#undef	setlocale
#define setlocale	_ast_setlocale

extern char*		setlocale(int, const char*);

#define AST_LC_COLLATE	0
#define AST_LC_CTYPE	1
#define AST_LC_MESSAGES	2
#define AST_LC_MONETARY	3
#define AST_LC_NUMERIC	4
#define AST_LC_TIME	5
#define AST_LC_ALL	6

#define AST_LC_debug		(1<<13)
#define AST_LC_setlocale	(1<<14)
#define AST_LC_translate	(1<<15)

#ifndef LC_COLLATE
#define LC_COLLATE	AST_LC_COLLATE
#endif
#ifndef LC_CTYPE
#define LC_CTYPE	AST_LC_CTYPE
#endif
#ifndef LC_MESSAGES
#define LC_MESSAGES	LC_ALL
#endif
#ifndef LC_MONETARY
#define LC_MONETARY	AST_LC_MONETARY
#endif
#ifndef LC_NUMERIC
#define LC_NUMERIC	AST_LC_NUMERIC
#endif
#ifndef LC_TIME
#define LC_TIME		AST_LC_TIME
#endif
#ifndef LC_ALL
#define LC_ALL		AST_LC_ALL
#endif

#undef	extern

#undef	strcoll
#if _std_strcoll
#define strcoll		_ast_info.collate
#else
#define strcoll		strcmp
#endif

typedef struct
{

	char*		id;

	struct
	{
	unsigned int	serial;
	unsigned int	set;
	}		locale;

	long		tmp_long;
	size_t		tmp_size;
	short		tmp_short;
	char		tmp_char;
	wchar_t		tmp_wchar;

	int		(*collate)(const char*, const char*);

	int		tmp_int;
	void*		tmp_pointer;

} _Ast_info_t;

#ifndef	_ast_info
#define _ast_info	_ast_state
#endif

#if _BLD_ast && defined(__EXPORT__)
#define __PUBLIC_DATA__		__EXPORT__
#else
#if !_BLD_ast && defined(__IMPORT__)
#define __PUBLIC_DATA__		__IMPORT__
#else
#define __PUBLIC_DATA__
#endif
#endif

extern __PUBLIC_DATA__ _Ast_info_t	_ast_info;

#undef	__PUBLIC_DATA__

/* stuff from std headers not used by ast, e.g., <stdio.h> */

extern void*		memzero(void*, size_t);
extern int		remove(const char*);
extern int		rename(const char*, const char*);

/* largefile hackery -- ast uses the large versions by default */

#if _typ_off64_t
#define off_t		off64_t
#endif
#if _lib_ftruncate64
#define ftruncate	ftruncate64
extern int		ftruncate64(int, off64_t);
#endif
#if _lib_lseek64
#define lseek		lseek64
extern off64_t		lseek64(int, off64_t, int);
#endif
#if _lib_truncate64
#define truncate	truncate64
extern int		truncate64(const char*, off64_t);
#endif

/* direct macro access for bsd crossover */

#if !defined(__cplusplus)

#if !defined(memcpy) && !defined(_lib_memcpy) && defined(_lib_bcopy)
extern void		bcopy(void*, void*, size_t);
#define memcpy(t,f,n)	(bcopy(f,t,n),(t))
#endif

#if !defined(memzero)
#if defined(_lib_bzero)
extern void		bzero(void*, size_t);
#if defined(FD_ZERO)
#undef	FD_ZERO
#define FD_ZERO(p)	memzero(p,sizeof(*p))
#endif
#define memzero(b,n)	(bzero(b,n),(b))
#else
#define memzero(b,n)	memset(b,0,n)
#endif
#endif

#endif

#if !defined(remove) && !defined(_lib_remove)
extern int		unlink(const char*);
#define remove(p)	unlink(p)
#endif

#if !defined(strchr) && !defined(_lib_strchr) && defined(_lib_index)
extern char*		index(const char*, int);
#define strchr(s,c)	index(s,c)
#endif

#if !defined(strrchr) && !defined(_lib_strrchr) && defined(_lib_rindex)
extern char*		rindex(const char*, int);
#define strrchr(s,c)	rindex(s,c)
#endif

/* and now introducing prototypes botched by the standard(s) */

#if _BLD_ast && defined(__EXPORT__)
#define extern		__EXPORT__
#endif

#undef	getpgrp
#define	getpgrp()	_ast_getpgrp()
extern int		_ast_getpgrp(void);

#undef	extern

#endif
