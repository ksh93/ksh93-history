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
 * AT&T Bell Laboratories
 *
 * <dirent.h> for systems with opendir() and <ndir.h>
 */

#ifndef _DIRENT_H
#define _DIRENT_H

#if defined(__STDPP__directive) && defined(__STDPP__hide)
__STDPP__directive pragma pp:hide closedir opendir readdir seekdir telldir
#else
#define closedir	______closedir
#define opendir		______opendir
#define readdir		______readdir
#define seekdir		______seekdir
#define telldir		______telldir
#endif

#include <ndir.h>

#if defined(__STDPP__directive) && defined(__STDPP__hide)
__STDPP__directive pragma pp:nohide closedir opendir readdir seekdir telldir
#else
#undef	closedir
#undef	opendir
#undef	readdir
#undef	seekdir
#undef	telldir
#endif

#ifndef dirent
#define dirent	direct
#endif

#if !defined(d_fileno) && !defined(d_ino)
#define d_fileno	d_ino
#endif

#ifdef	rewinddir
#undef	rewinddir
#define rewinddir(p)	seekdir(p,0L)
#endif

extern DIR*		opendir(const char*);
extern void		closedir(DIR*);
extern struct dirent*	readdir(DIR*);
extern void		seekdir(DIR*, long);
extern long		telldir(DIR*);

#endif
