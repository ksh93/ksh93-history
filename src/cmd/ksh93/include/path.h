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
#ifndef PATH_OFFSET

/*
 *	UNIX shell path handling interface
 *	Written by David Korn
 *	These are the definitions for the lexical analyzer
 */

#include	"FEATURE/options"
#include	<nval.h>

#define PATH_OFFSET	2		/* path offset for path_join */
#define MAXDEPTH	(sizeof(char*)==2?64:4096) /* maximum recursion depth*/

#ifndef ARG_RAW
    struct argnod;
#endif /* !ARG_RAW */

/* pathname handling routines */
extern void 		path_alias(Namval_t*,char*);
extern char 		*path_absolute(const char*,const char*);
extern char 		*path_basename(const char*);
extern int 		path_expand(const char*, struct argnod**);
extern void 		path_exec(const char*,char*[],struct argnod*);
#if defined(_DLL_BLD) && defined(_BLD_shell) 
#   define extern __EXPORT__
#endif
extern int		path_open(const char*,char*);
extern char 		*path_get(const char*);
#undef extern
extern char		*path_join(char*,const char*);
extern char 		*path_pwd(int);
extern int		path_search(const char*,const char*,int);
extern char		*path_relative(const char*);
extern int		path_complete(const char*, const char*,struct argnod**);
#ifdef SHOPT_BRACEPAT
    extern int 		path_generate(struct argnod*,struct argnod**);
#endif /* SHOPT_BRACEPAT */

/* constant strings needed for whence */
extern const char e_real[];
extern const char e_user[];
extern const char e_sys[];
extern const char e_dot[];
extern const char e_pwd[];
extern const char e_logout[];
extern const char e_alphanum[];
extern const char e_mailmsg[];
extern const char e_suidprofile[];
extern const char e_sysprofile[];
extern const char e_traceprompt[];
extern const char e_crondir[];
#ifdef SHOPT_SUID_EXEC
    extern const char	e_suidexec[];
#endif /* SHOPT_SUID_EXEC */
extern const char is_alias[];
extern const char is_builtin[];
extern const char is_builtver[];
extern const char is_reserved[];
extern const char is_talias[];
extern const char is_xalias[];
extern const char is_function[];
extern const char is_xfunction[];
extern const char is_ufunction[];
#ifdef SHELLMAGIC
    extern const char e_prohibited[];
#endif /* SHELLMAGIC */

#ifdef SHOPT_ACCT
#   include	"FEATURE/acct"
#   ifdef	_sys_acct
	extern void sh_accinit(void);
	extern void sh_accbegin(const char*);
	extern void sh_accend(void);
	extern void sh_accsusp(void);
#   else
#	undef	SHOPT_ACCT
#   endif	/* _sys_acct */
#endif /* SHOPT_ACCT */

#endif /*! PATH_OFFSET */
