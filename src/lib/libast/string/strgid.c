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
 * gid name -> number
 */

#if defined(__STDPP__directive) && defined(__STDPP__hide)
__STDPP__directive pragma pp:hide getgrgid getgrnam getpwnam
#else
#define getgrgid	______getgrgid
#define getgrnam	______getgrnam
#define getpwnam	______getpwnam
#endif

#include <ast.h>
#include <hash.h>
#include <pwd.h>
#include <grp.h>

#if defined(__STDPP__directive) && defined(__STDPP__hide)
__STDPP__directive pragma pp:nohide getgrgid getgrnam getpwnam
#else
#undef	getgrgid
#undef	getgrnam
#undef	getpwnam
#endif

extern struct group*	getgrgid(gid_t);
extern struct group*	getgrnam(const char*);
extern struct passwd*	getpwnam(const char*);

typedef struct
{
	HASH_HEADER;
	int	id;
} bucket;

/*
 * return gid number given gid/uid name
 * gid attempted first, then uid->pw_gid
 * -1 on first error for a given name
 * -2 on subsequent errors for a given name
 */

int
strgid(const char* name)
{
	register struct group*	gr;
	register struct passwd*	pw;
	register bucket*	b;
	char*			e;

	static Hash_table_t*	gidtab;

	if (!gidtab && !(gidtab = hashalloc(NiL, HASH_set, HASH_ALLOCATE, HASH_name, "gidnam", 0)))
		return -1;
	if (b = (bucket*)hashlook(gidtab, name, HASH_LOOKUP|HASH_FIXED, (char*)sizeof(bucket)))
		return b->id;
	if (!(b = (bucket*)hashlook(gidtab, NiL, HASH_CREATE|HASH_FIXED, (char*)sizeof(bucket))))
		return -1;
	if (gr = getgrnam(name))
		return b->id = gr->gr_gid;
	if (pw = getpwnam(name))
		return b->id = pw->pw_gid;
	if (strmatch(name, "+([0-9])"))
		return b->id = strtol(name, NiL, 0);
	b->id = strtol(name, &e, 0);
	if (!*e && getgrgid(b->id))
		return b->id;
	b->id = -2;
	return -1;
}
