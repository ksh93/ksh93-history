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
 * Glenn Fowler
 * Landon Kurt Knoll
 * Phong Vo
 *
 * FNV-1 linear congruent checksum/hash/PRNG
 * see http://www.isthe.com/chongo/tech/comp/fnv/
 */

#ifndef _FNV_H
#define _FNV_H

#include <ast_common.h>

#define FNV_INIT	0x811c9dc5L
#define FNV_MULT	0x01000193L

#define FNVINIT(h)	(h = FNV_INIT)
#define FNVPART(h,c)	(h = h * FNV_MULT ^ (c))
#define FNVSUM(h,s,n)	do { \
			register size_t _i_ = 0; \
			while (_i_ < n) \
				FNVPART(h, ((unsigned char*)s)[_i_++]); \
			} while (0)

#ifdef _ast_int8_t

#ifdef _ast_LL

#define FNV_INIT64	0xcbf29ce484222325LL
#define FNV_MULT64	0x00000100000001b3LL

#else

#define FNV_INIT64	((_ast_int8_t)0xcbf29ce484222325)
#define FNV_MULT64	((_ast_int8_t)0x00000100000001b3)

#endif

#define FNVINIT64(h)	(h = FNV_INIT64)
#define FNVPART64(h,c)	(h = h * FNV_MULT64 ^ (c))
#define FNVSUM64(h,s,n)	do { \
			register int _i_ = 0; \
			while (_i_ < n) \
				FNVPART64(h, ((unsigned char*)s)[_i_++]); \
			} while (0)

#endif

#endif
