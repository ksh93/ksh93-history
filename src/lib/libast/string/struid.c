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
 * uid name -> number
 */

#if defined(__STDPP__directive) && defined(__STDPP__hide)
__STDPP__directive pragma pp:hide getpwnam getpwuid
#else
#define getpwnam	______getpwnam
#define getpwuid	______getpwuid
#endif

#include <ast.h>
#include <hash.h>
#include <pwd.h>

#if defined(__STDPP__directive) && defined(__STDPP__hide)
__STDPP__directive pragma pp:nohide getpwnam getpwuid
#else
#undef	getpwnam
#undef	getpwuid
#endif

extern struct passwd*	getpwnam(const char*);
extern struct passwd*	getpwuid(uid_t);

typedef struct
{
	HASH_HEADER;
	int	id;
} bucket;

/*
 * return uid number given uid name
 * -1 on first error for a given name
 * -2 on subsequent errors for a given name
 */

int
struid(const char* name)
{
	register struct passwd*	pw;
	register bucket*	b;
	char*			e;

	static Hash_table_t*	uidtab;

	if (!uidtab && !(uidtab = hashalloc(NiL, HASH_set, HASH_ALLOCATE, HASH_name, "uidnam", 0)))
		return -1;
	if (b = (bucket*)hashlook(uidtab, name, HASH_LOOKUP|HASH_FIXED, (char*)sizeof(bucket)))
		return b->id;
	if (!(b = (bucket*)hashlook(uidtab, NiL, HASH_CREATE|HASH_FIXED, (char*)sizeof(bucket))))
		return -1;
	if (pw = getpwnam(name))
		return b->id = pw->pw_uid;
	b->id = strtol(name, &e, 0);
	if (!*e && getpwuid(b->id))
		return b->id;
	b->id = -2;
	return -1;
}
