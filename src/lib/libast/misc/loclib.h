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
 * locale and translation private definitions
 */

#ifndef _LOCLIB_H
#define _LOCLIB_H	1

#define locales		_ast_locales
#define translate	_ast_translate

#include <ast.h>
#include <error.h>

typedef struct
{
	char*		name;
	int		category;
	int		set;
	char*		usr;
	char*		sys;
	char*		old_usr;
	char*		old_sys;
} Locale_t;

#ifndef LC_COLLATE
#define LC_COLLATE	AST_LC_COLLATE
#endif
#ifndef LC_CTYPE
#define LC_CTYPE	AST_LC_CTYPE
#endif
#ifndef LC_MESSAGES
#define LC_MESSAGES	AST_LC_MESSAGES
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

extern Locale_t		locales[];

extern char*		translate(const char*, const char*, const char*, const char*);

#endif
