/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1990-2004 AT&T Corp.                  *
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
*                                                                      *
***********************************************************************/
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
		return !eaccess(pathshell(), X_OK);
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
