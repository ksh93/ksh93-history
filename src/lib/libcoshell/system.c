/***************************************************************
*                                                              *
*           This software is part of the ast package           *
*              Copyright (c) 1990-2000 AT&T Corp.              *
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
*                                                              *
***************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Research
 *
 * coshell system(3)
 */

#include "colib.h"

int
system(const char* cmd)
{
	Coshell_t*	co;
	Cojob_t*	cj;
	int		status;

	if (!cmd)
		return !access(pathshell(), X_OK);
	if (!(co = coopen(NiL, CO_ANY, NiL)))
		return -1;
	if (cj = coexec(co, cmd, CO_SILENT, NiL, NiL, NiL))
		cj = cowait(co, cj);
	if (!cj)
		return -1;

	/*
	 * synthesize wait() status from shell status
	 * lack of synthesis is the standard's proprietary sellout
	 */

	status = cj->status;
	if (EXITED_TERM(status))
		status &= ((1<<(EXIT_BITS-1))-1);
	else
		status = (status & ((1<<EXIT_BITS)-1)) << EXIT_BITS;
	return status;
}
