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
 * OBSOLETE 19970101 -- use ccmapcpy(op,ip,n,CC_EBCDIC2,CC_ASCII)
 */

#include <ast.h>
#include <ccode.h>

/*
 * convert n bytes of CC_EBCDIC2 ip to CC_ASCII in op
 * ip==op is ok
 */

void*
memetoa(void* op, const void* ip, size_t n)
{
	return op == ip ? ccmaps(op, n, CC_EBCDIC2, CC_ASCII) : ccmapcpy(op, ip, n, CC_EBCDIC2, CC_ASCII);
}
