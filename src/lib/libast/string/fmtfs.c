/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1985-2002 AT&T Corp.                *
*        and it may only be used by you under license from         *
*                       AT&T Corp. ("AT&T")                        *
*         A copy of the Source Code Agreement is available         *
*                at the AT&T Internet web site URL                 *
*                                                                  *
*       http://www.research.att.com/sw/license/ast-open.html       *
*                                                                  *
*    If you have copied or used this software without agreeing     *
*        to the terms of the license you are infringing on         *
*           the license and copyright and are violating            *
*               AT&T's intellectual property rights.               *
*                                                                  *
*            Information and Software Systems Research             *
*                        AT&T Labs Research                        *
*                         Florham Park NJ                          *
*                                                                  *
*               Glenn Fowler <gsf@research.att.com>                *
*                David Korn <dgk@research.att.com>                 *
*                 Phong Vo <kpv@research.att.com>                  *
*                                                                  *
*******************************************************************/
#pragma prototyped

/*
 * Glenn Fowler
 * AT&T Bell Laboratories
 *
 * return fs type string given stat
 */

#include <ast.h>
#include <ls.h>
#include <mnt.h>

#include "FEATURE/fs"

#if _str_st_fstype

char*
fmtfs(struct stat* st)
{
	return st->st_fstype;
}

#else

#include <hash.h>

char*
fmtfs(struct stat* st)
{
	register void*		mp;
	register Mnt_t*		mnt;
	register char*		s;
	struct stat		rt;
	char*			buf;

	static Hash_table_t*	tab;

	if ((tab || (tab = hashalloc(NiL, HASH_set, HASH_ALLOCATE, HASH_namesize, sizeof(dev_t), HASH_name, "fstype", 0))) && (s = (char*)hashget(tab, &st->st_dev)))
		return(s);
	s = FS_default;
	if (mp = mntopen(NiL, "r"))
	{
		while ((mnt = mntread(mp)) && (stat(mnt->dir, &rt) || rt.st_dev != st->st_dev));
		if (mnt && mnt->type && (!tab || !(s = strdup(mnt->type))))
		{
			buf = fmtbuf(strlen(mnt->type) + 1);
			strcpy(buf, mnt->type);
			mntclose(mp);
			return buf;
		}
		mntclose(mp);
	}
	if (tab)
		hashput(tab, NiL, s);
	return s;
}

#endif
