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
/*
 *  national.h -  definitions for international character sets
 *
 *   David Korn
 *   AT&T Labs
 *   Room 2B-102
 *   Murray Hill, N. J. 07974
 *   Tel. x7975
 *
 */

#ifdef SHOPT_MULTIBYTE
/*
 * This data must be defined for each country in defs.c
 */

#ifndef HIGHBIT
#   define HIGHBIT		(1<<(CHAR_BIT-1))
#endif /* HIGHBIT */

#ifndef ESS_MAXCHAR	/* allow multiple includes */

/*
 *  This section may change from country to country
 */

#define ESS_MAXCHAR	2	/* Maximum number of non-escape bytes
				   for any and all character sets */
#define	CCS1_IN_SIZE	2
#define	CCS1_OUT_SIZE	2
#define	CCS2_IN_SIZE	1
#define	CCS2_OUT_SIZE	1
#define	CCS3_IN_SIZE	2
#define	CCS3_OUT_SIZE	2

/*
 * This part is generic
 */

#define MARKER		0x100	/* Must be invalid character */
#define ESS2		0x8e	/* Escape to char set 2 */
#define ESS3		0x8f	/* Escape to char set 3 */
#define ESS_SETMASK	(3<<(7*ESS_MAXCHAR))	/* character set bits */

#define	echarset(c)	((c)==ESS3?3:((c)==ESS2)?2:((c)>>7)&1)
#define icharset(i)	((i)>>(7*ESS_MAXCHAR)&3)

#define in_csize(s)	int_charsize[s]
#define out_csize(s)	int_charsize[s+4]

extern char int_charsize[8];
extern int sh_strchr(const char*,const char*);

#endif /* ESS_MAXCHAR */
#endif /* SHOPT_MULTIBYTE */
