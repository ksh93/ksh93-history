/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1982-2004 AT&T Corp.                  *
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
*                  David Korn <dgk@research.att.com>                   *
*                                                                      *
***********************************************************************/
#pragma prototyped
#ifndef _SHTABLE_H

/*
 * David Korn
 * AT&T Labs
 *
 * Interface definitions read-only data tables for shell
 *
 */

#define _SHTABLE_H	1

typedef struct shtable1
{
	const char	*sh_name;
	unsigned	sh_number;
} Shtable_t;

struct shtable2
{
	const char	*sh_name;
	unsigned	sh_number;
	const char	*sh_value;
};

struct shtable3
{
	const char	*sh_name;
	unsigned	sh_number;
	int		(*sh_value)(int, char*[], void*);
};

#define sh_lookup(name,value)	sh_locate(name,(Shtable_t*)(value),sizeof(*(value)))
extern const Shtable_t		shtab_testops[];
extern const Shtable_t		shtab_options[];
extern const Shtable_t		shtab_attributes[];
extern const struct shtable2	shtab_variables[];
extern const struct shtable2	shtab_aliases[];
extern const struct shtable2	shtab_signals[];
extern const struct shtable3	shtab_builtins[];
extern const Shtable_t		shtab_reserved[];
extern int	sh_locate(const char*, const Shtable_t*, int);

#endif /* SH_TABLE_H */
