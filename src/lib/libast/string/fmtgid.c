/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1985-2001 AT&T Corp.                *
*        and it may only be used by you under license from         *
*                       AT&T Corp. ("AT&T")                        *
*         A copy of the Source Code Agreement is available         *
*                at the AT&T Internet web site URL                 *
*                                                                  *
*       http://www.research.att.com/sw/license/ast-open.html       *
*                                                                  *
*        If you have copied this software without agreeing         *
*        to the terms of the license you are infringing on         *
*           the license and copyright and are violating            *
*               AT&T's intellectual property rights.               *
*                                                                  *
*                 This software was created by the                 *
*                 Network Services Research Center                 *
*                        AT&T Labs Research                        *
*                         Florham Park NJ                          *
*                                                                  *
*               Glenn Fowler <gsf@research.att.com>                *
*                David Korn <dgk@research.att.com>                 *
*                 Phong Vo <kpv@research.att.com>                  *
*******************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Bell Laboratories
 *
 * cached gid number -> name
 */

#if defined(__STDPP__directive) && defined(__STDPP__hide)
__STDPP__directive pragma pp:hide getgrgid
#else
#define getgrgid	______getgrgid
#endif

#include <ast.h>
#include <hash.h>
#include <grp.h>

#if defined(__STDPP__directive) && defined(__STDPP__hide)
__STDPP__directive pragma pp:nohide getgrgid
#else
#undef	getgrgid
#endif

extern struct group*	getgrgid(gid_t);

/*
 * return gid name given gid number
 */

char*
fmtgid(int gid)
{
	register char*		name;
	register struct group*	gr;
	int			z;

	static Hash_table_t*	gidtab;

	if (!gidtab && !(gidtab = hashalloc(NiL, HASH_set, HASH_ALLOCATE, HASH_namesize, sizeof(gid), HASH_name, "gidnum", 0)))
	{
		name = fmtbuf(z = sizeof(int) * 3 + 1);
		sfsprintf(name, z, "%I*d", sizeof(gid), gid);
	}
	else if (!(name = hashget(gidtab, &gid)))
	{
		if (gr = getgrgid(gid))
			name = gr->gr_name;
		else
		{
			name = fmtbuf(z = sizeof(int) * 3 + 1);
			sfsprintf(name, z, "%I*d", sizeof(gid), gid);
		}
		hashput(gidtab, NiL, name = strdup(name));
	}
	return name;
}
