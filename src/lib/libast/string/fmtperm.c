/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1985-2006 AT&T Corp.                  *
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
 * Glenn Fowler
 * AT&T Bell Laboratories
 *
 * return strperm() expression for perm
 */

#include <ast.h>
#include <ls.h>

char*
fmtperm(register int perm)
{
	register char*	s;
	char*		buf;

	s = buf = fmtbuf(32);

	/*
	 * u
	 */

	*s++ = 'u';
	*s++ = '=';
	if (perm & S_ISVTX)
		*s++ = 't';
	if (perm & S_ISUID)
		*s++ = 's';
	if (perm & S_IRUSR)
		*s++ = 'r';
	if (perm & S_IWUSR)
		*s++ = 'w';
	if (perm & S_IXUSR)
		*s++ = 'x';
	if ((perm & (S_ISGID|S_IXGRP)) == S_ISGID)
		*s++ = 'l';

	/*
	 * g
	 */

	*s++ = ',';
	*s++ = 'g';
	*s++ = '=';
	if ((perm & (S_ISGID|S_IXGRP)) == (S_ISGID|S_IXGRP))
		*s++ = 's';
	if (perm & S_IRGRP)
		*s++ = 'r';
	if (perm & S_IWGRP)
		*s++ = 'w';
	if (perm & S_IXGRP)
		*s++ = 'x';

	/*
	 * o
	 */

	*s++ = ',';
	*s++ = 'o';
	*s++ = '=';
	if (perm & S_IROTH)
		*s++ = 'r';
	if (perm & S_IWOTH)
		*s++ = 'w';
	if (perm & S_IXOTH)
		*s++ = 'x';
	*s = 0;
	return buf;
}
