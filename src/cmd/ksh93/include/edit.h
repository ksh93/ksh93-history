/***************************************************************
*                                                              *
*           This software is part of the ast package           *
*              Copyright (c) 1982-2000 AT&T Corp.              *
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
*              David Korn <dgk@research.att.com>               *
*                                                              *
***************************************************************/
#pragma prototyped
#ifndef SEARCHSIZE
/*
 *  edit.h -  common data structure for vi and emacs edit options
 *
 *   David Korn
 *   AT&T Labs
 *
 */

#define SEARCHSIZE	80

#include	"FEATURE/options"
#if !defined(SHOPT_VSH) && !defined (SHOPT_ESH)
#   define ed_winsize()	(SEARCHSIZE)
#else

#ifndef KSHELL
#   include	<setjmp.h>
#   include	<sig.h>
#   include	<ctype.h>
#endif /* KSHELL */

#include	"FEATURE/setjmp"
#include	"terminal.h"

#ifdef SHOPT_SEVENBIT
#   define STRIP	0177
#else
#   define STRIP	0377
#endif /* SHOPT_SEVENBIT */
#define LOOKAHEAD	80

#ifdef SHOPT_MULTIBYTE
#   ifndef ESS_MAXCHAR
#   include	"national.h"
#   endif /* ESS_MAXCHAR */
#   if ESS_MAXCHAR<=2
	typedef unsigned short genchar;
#   else
	typedef long genchar;
#   endif
#   define CHARSIZE	2
#else
    typedef char genchar;
#   define CHARSIZE	1
#endif /* SHOPT_MULTIBYTE */

#define TABSIZE	8
#define PRSIZE	80
#define MAXLINE	502		/* longest edit line permitted */

typedef struct edit
{
	int	e_kill;
	int	e_erase;
	int	e_werase;
	int	e_eof;
	int	e_lnext;
	int	e_fchar;
	char	e_plen;		/* length of prompt string */
	char	e_crlf;		/* zero if cannot return to beginning of line */
	sigjmp_buf e_env;
	int	e_llimit;	/* line length limit */
	int	e_hline;	/* current history line number */
	int	e_hloff;	/* line number offset for command */
	int	e_hismin;	/* minimum history line number */
	int	e_hismax;	/* maximum history line number */
	int	e_raw;		/* set when in raw mode or alt mode */
	int	e_cur;		/* current line position */
	int	e_eol;		/* end-of-line position */
	int	e_pcur;		/* current physical line position */
	int	e_peol;		/* end of physical line position */
	int	e_mode;		/* edit mode */
	int	e_lookahead;	/* index in look-ahead buffer */
	int	e_repeat;
	int	e_saved;
	int	e_fcol;		/* first column */
	int	e_ucol;		/* column for undo */
	int	e_wsize;	/* width of display window */
	char	*e_outbase;	/* pointer to start of output buffer */
	char	*e_outptr;	/* pointer to position in output buffer */
	char	*e_outlast;	/* pointer to end of output buffer */
	genchar	*e_inbuf;	/* pointer to input buffer */
	char	*e_prompt;	/* pointer to buffer containing the prompt */
	genchar	*e_ubuf;	/* pointer to the undo buffer */
	genchar	*e_killbuf;	/* pointer to delete buffer */
	char	e_search[SEARCHSIZE];	/* search string */
	genchar	*e_Ubuf;	/* temporary workspace buffer */
	genchar	*e_physbuf;	/* temporary workspace buffer */
	int	e_lbuf[LOOKAHEAD];/* pointer to look-ahead buffer */
	int	e_fd;		/* file descriptor */
	int	e_ttyspeed;	/* line speed, also indicates tty parms are valid */
#ifdef _hdr_utime
	ino_t	e_tty_ino;
	dev_t	e_tty_dev;
	char	*e_tty;
#endif
#ifdef SHOPT_OLDTERMIO
	char	e_echoctl;
	char	e_tcgeta;
	struct termio e_ott;
#endif
#ifdef SHOPT_MULTIBYTE
	int	e_curchar;
	int	e_cursize;
#endif
	int	*e_globals;	/* global variables */
	genchar	*e_window;	/* display window  image */
	char	e_inmacro;	/* processing macro expansion */
#ifdef KSHELL
	char	e_vi_insert[2];	/* for sh_keytrap */
	long	e_col;		/* for sh_keytrap */
#else
	char	e_prbuff[PRSIZE]; /* prompt buffer */
#endif /* KSHELL */
	struct termios	e_ttyparm;      /* initial tty parameters */
	struct termios	e_nttyparm;     /* raw tty parameters */
	struct termios e_savetty;	/* saved terminal state */
	int	e_savefd;	/* file descriptor for saved terminal state */
	char	e_macro[4];	/* macro buffer */
	void	*e_vi;		/* vi specific data */
	void	*e_emacs;	/* emacs specific data */
	Shell_t	*sh;		/* interpreter pointer */ 
} Edit_t;

#undef MAXWINDOW
#define MAXWINDOW	160	/* maximum width window */
#define FAST	2
#define SLOW	1
#define ESC	cntl('[')
#define	UEOF	-2			/* user eof char synonym */
#define	UINTR	-3			/* user intr char synonym */
#define	UERASE	-4			/* user erase char synonym */
#define	UKILL	-5			/* user kill char synonym */
#define	UWERASE	-6			/* user word erase char synonym */
#define	ULNEXT	-7			/* user next literal char synonym */

#if ( 'a' == 97) /* ASCII? */
#   define	cntl(x)		(x&037)
#else
#   define cntl(c) (c=='D'?55:(c=='E'?45:(c=='F'?46:(c=='G'?'\a':(c=='H'?'\b': \
		(c=='I'?'\t':(c=='J'?'\n':(c=='T'?60:(c=='U'?61:(c=='V'?50: \
		(c=='W'?38:(c=='Z'?63:(c=='['?39:(c==']'?29: \
		(c<'J'?c+1-'A':(c+10-'J'))))))))))))))))
#endif

#ifndef KSHELL
#   define STRIP	0377
#   define GMACS	1
#   define EMACS	2
#   define VIRAW	4
#   define EDITVI	8
#   define NOHIST	16
#   define EDITMASK	15
#   define is_option(m)	(opt_flag&(m))
    extern char opt_flag;
#   ifdef SYSCALL
#	define read(fd,buff,n)	syscall(3,fd,buff,n)
#   else
#	define read(fd,buff,n)	rEAd(fd,buff,n)
#   endif /* SYSCALL */
#endif	/* KSHELL */

extern void	ed_crlf(Edit_t*);
extern void	ed_putchar(Edit_t*, int);
extern void	ed_ringbell(void);
extern void	ed_setup(Edit_t*,int);
extern void	ed_flush(Edit_t*);
extern int	ed_getchar(Edit_t*,int);
extern int	ed_virt_to_phys(Edit_t*,genchar*,genchar*,int,int,int);
extern int	ed_window(void);
extern void	ed_ungetchar(Edit_t*,int);
extern int	ed_viread(int, char*, int);
extern int	ed_read(int, char*, int);
extern int	ed_emacsread(int, char*, int);
#ifdef KSHELL
	extern int	ed_macro(Edit_t*,int);
	extern int	ed_expand(Edit_t*, char[],int*,int*,int);
	extern int	ed_fulledit(Edit_t*);
	extern void	*ed_open(Shell_t*);
#endif /* KSHELL */
#   ifdef SHOPT_MULTIBYTE
	extern int ed_internal(const char*, genchar*);
	extern int ed_external(const genchar*, char*);
	extern void ed_gencpy(genchar*,const genchar*);
	extern void ed_genncpy(genchar*,const genchar*,int);
	extern int ed_genlen(const genchar*);
	extern int ed_setwidth(const char*);
#  endif /* SHOPT_MULTIBYTE */

extern const char	e_runvi[];
#ifndef KSHELL
   extern const char	e_version[];
#endif /* KSHELL */

#endif
#endif
