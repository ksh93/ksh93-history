/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1982-2001 AT&T Corp.                *
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
*                David Korn <dgk@research.att.com>                 *
*******************************************************************/
#pragma prototyped

#include	<ast.h>
#include	"ulimit.h"

#define size_resource(a,b) ((a)|((b)<<11))	

/*
 * This is the list of resouce limits controlled by ulimit
 * This command requires getrlimit(), vlimit(), or ulimit()
 */

#ifndef _no_ulimit 
const Shtable_t shtab_limits[] =
{
	{"time(seconds)       ",	size_resource(1,RLIMIT_CPU)},
#   ifdef RLIMIT_FSIZE
	{"file(blocks)        ",	size_resource(512,RLIMIT_FSIZE)},
#   else
	{"file(blocks)        ",	size_resource(1,2)},
#   endif /* RLIMIT_FSIZE */
	{"data(kbytes)        ",	size_resource(1024,RLIMIT_DATA)},
	{"stack(kbytes)       ",	size_resource(1024,RLIMIT_STACK)},
	{"memory(kbytes)      ",	size_resource(1024,RLIMIT_RSS)},
	{"coredump(blocks)    ",	size_resource(512,RLIMIT_CORE)},
	{"nofiles(descriptors)",	size_resource(1,RLIMIT_NOFILE)},
	{"vmemory(kbytes)     ",	size_resource(1024,RLIMIT_VMEM)}
};

const char e_unlimited[] = "unlimited";

#endif
