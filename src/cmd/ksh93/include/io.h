/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1982-2005 AT&T Corp.                  *
*                      and is licensed under the                       *
*                  Common Public License, Version 1.0                  *
*                            by AT&T Corp.                             *
*                                                                      *
*                A copy of the License is available at                 *
*            http://www.opensource.org/licenses/cpl1.0.txt             *
*         (with md5 checksum 059e8cd6165cb4c31e351f2b69388fd9)         *
*                                                                      *
*              Information and Software Systems Research               *
*                            AT&T Research                             *
*                           Florham Park NJ                            *
*                                                                      *
*                  David Korn <dgk@research.att.com>                   *
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 *	UNIX shell
 *	David Korn
 *
 */

#include	<ast.h>
#include	<sfio.h>

#ifndef IOBSIZE
#   define  IOBSIZE	SF_BUFSIZE
#endif /* IOBSIZE */
#define IOMAXTRY	20

#ifndef SF_CLOSING
#define SF_CLOSING	SF_CLOSE
#endif
#ifndef SF_APPENDWR
#define SF_APPENDWR	SF_APPEND
#endif

/* used for output of shell errors */
#define ERRIO		2

#define IOREAD		001
#define IOWRITE		002
#define IODUP 		004
#define IOSEEK		010
#define IONOSEEK	020
#define IOTTY 		040
#define IOCLEX 		0100
#define IOCLOSE		(IOSEEK|IONOSEEK)

#define IOSUBSHELL	0x8000	/* must be larger than any file descriptor */

/*
 * The remainder of this file is only used when compiled with shell
 */

#if KSHELL

#ifndef ARG_RAW
    struct ionod;
#endif /* !ARG_RAW */

#define sh_inuse(f2)	(sh.fdptrs[f2])

extern int	sh_iocheckfd(int);
extern void 	sh_ioinit(void);
extern int 	sh_iomovefd(int);
extern int	sh_iorenumber(int,int);
extern void 	sh_pclose(int[]);
extern void 	sh_iorestore(int,int);
#if defined(__EXPORT__) && defined(_BLD_DLL) && defined(_BLD_shell) 
   __EXPORT__
#endif
extern Sfio_t 	*sh_iostream(int);
struct ionod;
extern int	sh_redirect(struct ionod*,int);
extern void 	sh_iosave(int,int);
extern void 	sh_iounsave(void);
extern int	sh_chkopen(const char*);
extern int	sh_ioaccess(int,int);
extern int	sh_devtofd(const char*);

/* the following are readonly */
extern const char	e_pexists[];
extern const char	e_query[];
extern const char	e_history[];
extern const char	e_argtype[];
extern const char	e_create[];
extern const char	e_tmpcreate[];
extern const char	e_exists[];
extern const char	e_file[];
extern const char	e_formspec[];
extern const char	e_badregexp[];
extern const char	e_open[];
extern const char	e_toomany[];
extern const char	e_pipe[];
extern const char	e_unknown[];
extern const char	e_devnull[];
extern const char	e_profile[];
extern const char	e_sysprofile[];
extern const char	e_stdprompt[];
extern const char	e_supprompt[];
extern const char	e_ambiguous[];
#endif /* KSHELL */
