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
 * AT&T Research
 *
 * <dirent.h> for [fl]stat64 and off64_t
 *
 * NOTE: this file assumes the local <dirent.h>
 *	 can be reached by <../include/dirent.h>
 */

#ifndef _DIR64_H
#define _DIR64_H

#include <ast_std.h>

#if _typ_off64_t
#undef	off_t
#endif

#include <../include/dirent.h>

#if _typ_off64_t
#define	off_t		off64_t
#endif

#if _lib_readdir64 && _typ_struct_dirent64
#ifndef	dirent
#define dirent		dirent64
#endif
#ifndef	readdir
#define readdir		readdir64
#endif
#endif

#endif
