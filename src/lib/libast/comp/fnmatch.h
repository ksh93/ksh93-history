/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1985-2004 AT&T Corp.                  *
*                      and is licensed under the                       *
*          Common Public License, Version 1.0 (the "License")          *
*                        by AT&T Corp. ("AT&T")                        *
*      Any use, downloading, reproduction or distribution of this      *
*      software constitutes acceptance of the License.  A copy of      *
*                     the License is available at                      *
*                                                                      *
*         http://www.research.att.com/sw/license/cpl-1.0.html          *
*         (with md5 checksum 8a5e0081c856944e76c69a1cf29c2e8b)         *
*                                                                      *
*              Information and Software Systems Research               *
*                            AT&T Research                             *
*                           Florham Park NJ                            *
*                                                                      *
*                 Glenn Fowler <gsf@research.att.com>                  *
*                  David Korn <dgk@research.att.com>                   *
*                   Phong Vo <kpv@research.att.com>                    *
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * posix fnmatch interface definitions
 */

#ifndef _FNMATCH_H
#define _FNMATCH_H

/* fnmatch flags */

#define FNM_NOESCAPE	0x0001		/* \ is literal			*/
#define FNM_PATHNAME	0x0002		/* explicit match for /		*/
#define FNM_PERIOD	0x0004		/* explicit match for leading .	*/
#define FNM_NOSYS	0x0010		/* not implemented		*/

/* nonstandard fnmatch() flags */

#define FNM_AUGMENTED	0x0008		/* enable ! & ( | )		*/
#define FNM_ICASE	0x0020		/* ignore case in match		*/
#define FNM_LEADING_DIR	0x0040		/* match up to implicit /	*/

#define FNM_CASEFOLD	FNM_ICASE	/* gnu compatibility		*/
#define FNM_FILE_NAME	FNM_PATHNAME	/* gnu compatibility		*/

/* fnmatch error codes -- other non-zero values from <regex.h> */

#define FNM_NOMATCH	1		/* == REG_NOMATCH		*/

extern int	fnmatch(const char*, const char*, int);

#endif
