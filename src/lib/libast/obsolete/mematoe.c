/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1985-2004 AT&T Corp.                  *
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
 * OBSOLETE 19970101 -- use ccmapcpy(op,ip,n,CC_ASCII,CC_EBCDIC2)
 */

#include <ast.h>
#include <ccode.h>

/*
 * convert n bytes of CC_ASCII ip to CC_EBCDIC2 in op
 * ip==op is ok
 */

void*
mematoe(void* op, const void* ip, size_t n)
{
	return op == ip ? ccmaps(op, n, CC_ASCII, CC_EBCDIC2) : ccmapcpy(op, ip, n, CC_ASCII, CC_EBCDIC2);
}
