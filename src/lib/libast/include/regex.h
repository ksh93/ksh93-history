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
 * regex library interface
 */

#ifndef _REGEX_H
#define _REGEX_H

#include <ast_common.h>

/* regcomp flags */

#define REG_AUGMENTED	0x0001		/* enable ! & < >		*/
#define REG_EXTENDED	0x0002		/* enable ( | )			*/
#define REG_ICASE	0x0004		/* ignore case in match		*/
#define REG_NEWLINE	0x0008		/* \n is a regular char		*/
#define REG_NOSUB	0x0010		/* don't report subexp matches	*/
#define REG_SHELL	0x0020		/* shell pattern syntax		*/

/* nonstandard regcomp flags */

#define REG_LEFT	0x0100		/* implicit ^...		*/
#define REG_LITERAL	0x0200		/* no operators			*/
#define REG_MINIMAL	0x0400		/* minimal match		*/
#define REG_NULL	0x0800		/* allow null patterns		*/
#define REG_RIGHT	0x1000		/* implicit ...$		*/
#define REG_LENIENT	0x2000		/* look the other way		*/
#define REG_MULTIPLE	0x4000		/* multiple \n sep patterns	*/

#define REG_SHELL_DOT	REG_NOTBOL	/* explicit leading . match	*/
#define REG_SHELL_ESCAPED REG_NOTEOL	/* \ not special		*/
#define REG_SHELL_PATH	REG_NEWLINE	/* explicit / match		*/

#define REG_DELIMITED	REG_STARTEND	/* pattern[0] is delimiter	*/
#define REG_ESCAPE	REG_INVERT	/* \delimiter needed in [...]	*/


/* regexec flags */

#define REG_NOTBOL	0x0040		/* ^ is not a special char	*/
#define REG_NOTEOL	0x0080		/* $ is not a special char	*/

/* nonstandard regexec flags */

#define REG_INVERT	0x4000		/* invert regrexec match sense	*/
#define REG_STARTEND	0x8000		/* subject==match[0].rm_{so,eo} */

/* regalloc flags */

#define REG_NOFREE	0x0001		/* don't free			*/

/* regsub flags */

#define REG_SUB_ALL	0x0001		/* substitute all occurrences	*/
#define REG_SUB_LOWER	0x0002		/* substitute to lower case	*/
#define REG_SUB_UPPER	0x0004		/* substitute to upper case	*/
#define REG_SUB_USER	0x0008		/* first user flag bit		*/

/* regex error codes */

#define REG_ENOSYS	(-1)		/* not supported		*/
#define REG_NOMATCH	1		/* regexec didn't match		*/
#define REG_BADPAT	2		/* invalid regular expression	*/
#define REG_ECOLLATE	3		/* invalid collation element	*/
#define REG_ECTYPE	4		/* invalid character class	*/
#define REG_EESCAPE	5		/* trailing \ in pattern	*/
#define REG_ESUBREG	6		/* invalid \digit backreference	*/
#define REG_EBRACK	7		/* [...] imbalance		*/
#define REG_EPAREN	8		/* \(...\) or (...) imbalance	*/
#define REG_EBRACE	9		/* \{...\} or {...} imbalance	*/
#define REG_BADBR	10		/* invalid {...} digits		*/
#define REG_ERANGE	11		/* invalid [...] range endpoint	*/
#define REG_ESPACE	12		/* out of space			*/
#define REG_BADRPT	13		/* unary op not preceeded by re	*/
#define REG_ENULL	14		/* empty subexpr in pattern	*/
#define REG_ECOUNT	15		/* re component count overflow	*/
#define REG_BADESC	16		/* invalid \char escape		*/
#define REG_VERSION	17		/* version stamp (pseudo error)	*/

typedef int (*regclass_t)(int);
typedef int regoff_t;
typedef void* (*regresize_t)(void*, void*, size_t);
typedef int (*regrecord_t)(void*, const char*, size_t);

typedef struct
{
	regoff_t	rm_so;		/* offset of start		*/
	regoff_t	rm_eo;		/* offset of end		*/
} regmatch_t;

typedef struct
{
	size_t		re_nsub;	/* number of subexpressions	*/
	struct Reginfo*	re_info;	/* library private info		*/
	size_t		re_npat;	/* number of pattern chars used	*/
} regex_t;

#if _BLD_ast && defined(__EXPORT__)
#define extern		__EXPORT__
#endif

extern int	regcomp(regex_t*, const char*, int);
extern size_t	regerror(int, const regex_t*, char*, size_t);
extern int	regexec(const regex_t*, const char*, size_t, regmatch_t*, int);
extern void	regfree(regex_t*);

/* nonstandard hooks */

#ifndef _SFIO_H
struct _sfio_s;
#endif

extern void	regalloc(void*, regresize_t, int);
extern regclass_t regclass(const char*, char**);
extern int	regcollate(const char*, char**);
extern int	regcomb(regex_t*, regex_t*);
extern int	regnexec(const regex_t*, const char*, size_t, size_t, regmatch_t*, int);
extern void	regfatal(regex_t*, int, int);
extern void	regfatalpat(regex_t*, int, int, const char*);
extern int	regrecord(const regex_t*);
extern int	regrexec(const regex_t*, const char*, size_t, size_t, regmatch_t*, int, int, void*, regrecord_t);
extern int	regsub(const regex_t*, struct _sfio_s*, const char*, const char*, size_t, regmatch_t*, int);

#undef	extern

#endif
