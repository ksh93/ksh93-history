/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1992-2004 AT&T Corp.                  *
*                      and is licensed under the                       *
*          Common Public License, Version 1.0 (the "License")          *
*                        by AT&T Corp. ("AT&T")                        *
*      Any use, downloading, reproduction or distribution of this      *
*      software constitutes acceptance of the License.  A copy of      *
*                     the License is available at                      *
*                                                                      *
*         http://www.research.att.com/sw/license/cpl-1.0.html          *
*         (with md5 checksum 8a5e0081c856944e76c69a1cf29c2e8b)         *
*                                                                      *
*              Information and Software Systems Research               *
*                            AT&T Research                             *
*                           Florham Park NJ                            *
*                                                                      *
*                 Glenn Fowler <gsf@research.att.com>                  *
*                  David Korn <dgk@research.att.com>                   *
*                                                                      *
***********************************************************************/
#pragma prototyped

/*
 * common ast cmd library definitions
 */

#ifndef _CMDLIB_H
#define _CMDLIB_H

#include <ast.h>
#include <cmd.h>
#include <error.h>
#include <stak.h>

#define rev_line	_cmd_revline

extern int		rev_line(Sfio_t*, Sfio_t*, off_t);

#endif
