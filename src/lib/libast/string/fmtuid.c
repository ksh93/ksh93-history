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
*             Glenn Fowler <gsf@research.att.com>              *
*              David Korn <dgk@research.att.com>               *
*               Phong Vo <kpv@research.att.com>                *
*                                                              *
***************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Bell Laboratories
 *
 * uid number -> name
 */

#if defined(__STDPP__directive) && defined(__STDPP__hide)
__STDPP__directive pragma pp:hide getpwuid
#else
#define getpwuid	______getpwuid
#endif

#include <ast.h>
#include <hash.h>
#include <pwd.h>

#if defined(__STDPP__directive) && defined(__STDPP__hide)
__STDPP__directive pragma pp:nohide getpwuid
#else
#undef	getpwuid
#endif

extern struct passwd*	getpwuid(uid_t);

/*
 * return uid name given uid number
 */

char*
fmtuid(int uid)
{
	register char*		name;
	register struct passwd*	pw;

	static Hash_table_t*	uidtab;
	static char		buf[sizeof(int) * 3 + 1];

	if (!uidtab && !(uidtab = hashalloc(NiL, HASH_set, HASH_ALLOCATE, HASH_namesize, sizeof(uid), HASH_name, "uidnum", 0)))
		sfsprintf(name = buf, sizeof(buf), "%I*d", sizeof(uid), uid);
	else if (!(name = hashget(uidtab, &uid)))
	{
		if (pw = getpwuid(uid))
			name = pw->pw_name;
		else
			sfsprintf(name = buf, sizeof(buf), "%I*d", sizeof(uid), uid);
		hashput(uidtab, NiL, name = strdup(name));
	}
	return name;
}
