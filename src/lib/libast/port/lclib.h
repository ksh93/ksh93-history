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
 * locale state private definitions
 */

#ifndef _LCLIB_H
#define _LCLIB_H	1

#define categories	_ast_categories
#define locales		_ast_locales
#define translate	_ast_translate

struct Lc_info_s;

#define _LC_PRIVATE_ \
	struct Lc_info_s	info[AST_LC_COUNT]; \
	struct Lc_s*		next;

#define _LC_TERRITORY_PRIVATE_ \
	unsigned char		indices[LC_territory_language_max];

#include <ast.h>
#include <error.h>
#include <lc.h>

typedef struct Lc_numeric_s
{
	int		decimal;
	int		thousand;
} Lc_numeric_t;

#define LCINFO(c)	(&locales[c]->info[c])

extern	Lc_category_t	categories[];
extern	Lc_t*		locales[];

extern char*		translate(const char*, const char*, const char*, const char*);

#endif
