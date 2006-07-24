/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1985-2005 AT&T Corp.                  *
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
*                 Glenn Fowler <gsf@research.att.com>                  *
*                  David Korn <dgk@research.att.com>                   *
*                   Phong Vo <kpv@research.att.com>                    *
*                                                                      *
***********************************************************************/
#pragma prototyped

/*
 * generic binary magic id definitions
 */

#ifndef _MAGICID_H
#define _MAGICID_H		1

#include <ast_common.h>

#define MAGICID		0x00010203

typedef unsigned _ast_int4_t Magicid_data_t;

typedef struct Magicid_s
{
	Magicid_data_t	magic;		/* magic number			*/
	char		name[8];	/* generic data/application name*/
	char		type[12];	/* specific data type		*/
	Magicid_data_t	version;	/* YYYYMMDD or 0xWWXXYYZZ	*/
	Magicid_data_t	size;
} Magicid_t;

#endif
