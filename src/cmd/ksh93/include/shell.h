/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1982-2002 AT&T Corp.                *
*        and it may only be used by you under license from         *
*                       AT&T Corp. ("AT&T")                        *
*         A copy of the Source Code Agreement is available         *
*                at the AT&T Internet web site URL                 *
*                                                                  *
*       http://www.research.att.com/sw/license/ast-open.html       *
*                                                                  *
*    If you have copied or used this software without agreeing     *
*        to the terms of the license you are infringing on         *
*           the license and copyright and are violating            *
*               AT&T's intellectual property rights.               *
*                                                                  *
*            Information and Software Systems Research             *
*                        AT&T Labs Research                        *
*                         Florham Park NJ                          *
*                                                                  *
*                David Korn <dgk@research.att.com>                 *
*                                                                  *
*******************************************************************/
#pragma prototyped
#ifndef SH_INTERACTIVE
/*
 * David Korn
 * AT&T Labs
 *
 * Interface definitions for shell command language
 *
 */

#include	<cmd.h>
#include	<cdt.h>
#include	<history.h>
#ifdef _SH_PRIVATE
#   include	"name.h"
#else
#   include	<nval.h>
#endif /* _SH_PRIVATE */

#define SH_VERSION	20010901

/*
 *  The following will disappear in a future release so change all sh_fun
 *  calls to use three arguments and sh_waitnotify specify a function
 *  that will be called with three arguments.  The third argument to
 *  the waitnotify function will 0 for input, 1 for output.
 */
#define sh_fun		_sh_fun
#define sh_waitnotify	_sh_waitnotify

#undef NOT_USED
#define NOT_USED(x)	(&x,1)

/* options */
typedef unsigned long	Shopt_t;

#define SH_CFLAG	(1<<0)
#define SH_HISTORY	(1<<1) /* used also as a state */
#define	SH_ERREXIT	(1<<2) /* used also as a state */
#define	SH_VERBOSE	(1<<3) /* used also as a state */
#define SH_MONITOR	(1<<4)/* used also as a state */
#define	SH_INTERACTIVE	(1<<5) /* used also as a state */
#define	SH_RESTRICTED	(1L<<6)
#define	SH_XTRACE	(1L<<7)
#define	SH_KEYWORD	(1L<<8)
#define SH_NOUNSET	(1L<<9)
#define SH_NOGLOB	(1L<<10)
#define SH_ALLEXPORT	(1L<<11)
#define SH_IGNOREEOF	(1L<<13)
#define SH_NOCLOBBER	(1L<<14)
#define SH_MARKDIRS	(1L<<15)
#define SH_BGNICE	(1L<<16)
#define SH_VI		(1L<<17)
#define SH_VIRAW	(1L<<18)
#define	SH_TFLAG	(1L<<19)
#define SH_TRACKALL	(1L<<20)
#define	SH_SFLAG	(1L<<21)
#define	SH_NOEXEC	(1L<<22)
#define SH_GMACS	(1L<<24)
#define SH_EMACS	(1L<<25)
#define SH_PRIVILEGED	(1L<<26)
#define SH_SUBSHARE	(1L<<27)	/* subshell shares state with parent */
#define SH_NOLOG	(1L<<28)
#define SH_NOTIFY	(1L<<29)
#define SH_DICTIONARY	(1L<<30)
#define SH_PIPEFAIL	(1L<<31)

/* The following type is used for error messages */

/* error messages */
extern const char	e_defpath[];
extern const char	e_found[];
extern const char	e_nospace[];
extern const char	e_format[];
extern const char 	e_number[];
extern const char	e_restricted[];
extern const char	e_recursive[];
extern char		e_version[];

typedef struct sh_scope
{
	struct sh_scope	*par_scope;
	int		argc;
	char		**argv;
	char		*cmdname;
	Dt_t	       *var_tree;
} Shscope_t;

/*
 * Saves the state of the shell
 */

typedef struct sh_static
{
	int		inlineno;	/* line number of current input file */
	Shopt_t		options;	/* set -o options */
	Dt_t		*var_tree;	/* for shell variables */
	Dt_t		*fun_tree;	/* for shell functions */
	Dt_t		*alias_tree;	/* for alias names */
	Dt_t		*bltin_tree;    /* for builtin commands */
	int		exitval;	/* most recent exit value */
	Sfio_t		**sftable;	/* pointer to stream pointer table */
	unsigned char	*fdstatus;	/* pointer to file status table */
	const char	*pwd;		/* present working directory */
	History_t	*hist_ptr;	/* history file pointer */
	unsigned char	trapnote;	/* set when trap/signal is pending */
	char		subshell;	/* set for virtual subshell */
	char            universe;
	int             (*waitevent)(int,long,int);
	Shscope_t	*topscope;	/* pointer to top-level scope */
	void		*jmpbuffer;	/* setjmp buffer pointer */
#ifdef _SH_PRIVATE
	_SH_PRIVATE
#endif /* _SH_PRIVATE */
} Shell_t;

/* flags for sh_parse */
#define SH_NL		1	/* Treat new-lines as ; */
#define SH_EOF		2	/* EOF causes syntax error */

/* symbolic values for sh_iogetiop */
#define SH_IOCOPROCESS	(-2)
#define SH_IOHISTFILE	(-3)

/* symbolic value for sh_fdnotify */
#define SH_FDCLOSE	(-1)

#if defined(__EXPORT__) && defined(_DLL)
#   ifdef _BLD_shell
#	define extern __EXPORT__
#   endif /* _BLD_shell */
#endif /* _DLL */

extern Dt_t		*sh_bltin_tree(void);
extern void		sh_subfork(void);
extern int		sh_init(int,char*[],void(*)(int));
extern int		sh_reinit(char*[]);
extern int 		sh_eval(Sfio_t*,int);
extern void 		sh_delay(double);
extern void		*sh_parse(Sfio_t*,int);
extern int 		sh_trap(const char*,int);
extern int 		sh_fun(Namval_t*,Namval_t*, char*[]);
extern int 		sh_funscope(int,char*[],int(*)(void*),void*,int);
extern Sfio_t		*sh_iogetiop(int,int);
extern int		sh_main(int, char*[], void(*)(int));
extern void		sh_menu(Sfio_t*, int, char*[]);
extern Namval_t		*sh_addbuiltin(const char*, int(*)(int, char*[],void*), void*);
extern char		*sh_fmtq(const char*);
extern double		sh_strnum(const char*, char**, int);
extern int		sh_access(const char*,int);
extern int 		sh_close(int);
extern int 		sh_dup(int);
extern void 		sh_exit(int);
extern int		sh_fcntl(int, int, ...);
extern Sfio_t		*sh_fd2sfio(int);
extern int		(*sh_fdnotify(int(*)(int,int)))(int,int);
extern Shell_t		*sh_getinterp(void);
extern int		sh_open(const char*, int, ...);
extern int		sh_openmax(void);
extern Sfio_t		*sh_pathopen(const char*);
extern ssize_t 		sh_read(int, void*, size_t);
extern ssize_t 		sh_write(int, const void*, size_t);
extern off_t		sh_seek(int, off_t, int);
extern int 		sh_pipe(int[]);
extern void		*sh_waitnotify(int(*)(int,long,int));
extern Shscope_t	*sh_getscope(int,int);
extern Shscope_t	*sh_setscope(Shscope_t*);
extern void		sh_sigcheck(void);
extern Shopt_t		sh_isoption(Shopt_t);
extern Shopt_t		sh_onoption(Shopt_t);
extern Shopt_t		sh_offoption(Shopt_t);
extern int 		sh_waitsafe(void);



#ifdef SHOPT_DYNAMIC
    extern void		**sh_getliblist(void);
#endif /* SHOPT_DYNAMIC */

/*
 * direct access to sh is obsolete, use sh_getinterp() instead
 */
#if !defined(_SH_PRIVATE) && defined(__IMPORT__) && !defined(_BLD_shell)
	__IMPORT__  Shell_t sh;
#else
	extern Shell_t sh;
#endif

#ifdef _DLL
#   undef extern
#endif /* _DLL */

#ifndef _SH_PRIVATE
#   define access(a,b)	sh_access(a,b)
#   define close(a)	sh_close(a)
#   define exit(a)	sh_exit(a)
#   define fcntl(a,b,c)	sh_fcntl(a,b,c)
#   define pipe(a)	sh_pipe(a)
#   define read(a,b,c)	sh_read(a,b,c)
#   define write(a,b,c)	sh_write(a,b,c)
#   define dup		sh_dup
#   if _lib_lseek64
#	define open64	sh_open
#	define lseek64	sh_seek
#   else
#	define open	sh_open
#	define lseek	sh_seek
#   endif
#endif /* !_SH_PRIVATE */

#define SH_SIGSET	4
#define SH_EXITSIG	0400	/* signal exit bit */
#define SH_EXITMASK	(SH_EXITSIG-1)	/* normal exit status bits */
#define SH_RUNPROG	-1022	/* needs to be negative and < 256 */

#endif /* SH_INTERACTIVE */
