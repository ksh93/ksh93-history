/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1985-2004 AT&T Corp.                  *
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
*                   Phong Vo <kpv@research.att.com>                    *
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Research
 *
 * sfio tmp string buffer support
 */

#include <sfio_t.h>
#include <ast.h>
#include <sfstr.h>

/*
 * replace buffer in string stream f for either SF_READ or SF_WRITE
 */

int
sfstrtmp(register Sfio_t* f, int mode, void* buf, size_t siz)
{
	if (!(f->_flags & SF_STRING))
		return -1;
	if (f->_flags & SF_MALLOC)
		free(f->_data);
	f->_flags &= ~(SF_ERROR|SF_MALLOC);
	f->mode = mode;
	f->_next = f->_data = (unsigned char*)buf;
	f->_endw = f->_endr = f->_endb = f->_data + siz;
	f->_size = siz;
	return 0;
}
