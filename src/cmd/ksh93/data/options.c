/*
 * CDE - Common Desktop Environment
 *
 * Copyright (c) 1993-2012, The Open Group. All rights reserved.
 *
 * These libraries and programs are free software; you can
 * redistribute them and/or modify them under the terms of the GNU
 * Lesser General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * These libraries and programs are distributed in the hope that
 * they will be useful, but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with these librararies and programs; if not, write
 * to the Free Software Foundation, Inc., 51 Franklin Street, Fifth
 * Floor, Boston, MA 02110-1301 USA
 */
/***************************************************************
*                                                              *
*                      AT&T - PROPRIETARY                      *
*                                                              *
*        THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF        *
*                    AT&T BELL LABORATORIES                    *
*         AND IS NOT TO BE DISCLOSED OR USED EXCEPT IN         *
*            ACCORDANCE WITH APPLICABLE AGREEMENTS             *
*                                                              *
*                Copyright (c) 1995 AT&T Corp.                 *
*              Unpublished & Not for Publication               *
*                     All Rights Reserved                      *
*                                                              *
*       The copyright notice above does not evidence any       *
*      actual or intended publication of such source code      *
*                                                              *
*               This software was created by the               *
*           Advanced Software Technology Department            *
*                    AT&T Bell Laboratories                    *
*                                                              *
*               For further information contact                *
*                    {research,attmail}!dgk                    *
*                                                              *
***************************************************************/

/* : : generated by proto : : */

#if !defined(__PROTO__)
#if defined(__STDC__) || defined(__cplusplus) || defined(_proto) || defined(c_plusplus)
#if defined(__cplusplus)
#define __MANGLE__	"C"
#else
#define __MANGLE__
#endif
#define __STDARG__
#define __PROTO__(x)	x
#define __OTORP__(x)
#define __PARAM__(n,o)	n
#if !defined(__STDC__) && !defined(__cplusplus)
#if !defined(c_plusplus)
#define const
#endif
#define signed
#define void		int
#define volatile
#define __V_		char
#else
#define __V_		void
#endif
#else
#define __PROTO__(x)	()
#define __OTORP__(x)	x
#define __PARAM__(n,o)	o
#define __MANGLE__
#define __V_		char
#define const
#define signed
#define void		int
#define volatile
#endif
#if defined(__cplusplus) || defined(c_plusplus)
#define __VARARG__	...
#else
#define __VARARG__
#endif
#if defined(__STDARG__)
#define __VA_START__(p,a)	va_start(p,a)
#else
#define __VA_START__(p,a)	va_start(p)
#endif
#endif
#include	<shell.h>
#include	"FEATURE/options"
#include	"name.h"
#include	"shtable.h"

/*
 * This is the list of invocation and set options
 * This list must be in in ascii sorted order
 */

#define bit32(x)	((x)&0xffff?bit16(x):16+bit16((x)>>16))
#define bit16(x)	((x)&0xff?bit8(x):8+bit8((x)>>8))
#define bit8(x)		((x)&0xf?bit4(x):4+bit4((x)>>4))
#define bit4(x)		((x)&0x3?bit2(x):2+bit2((x)>>2))
#define bit2(x)		((x)&1?0:1)

const Shtable_t shtab_options[] =
{
	"allexport",		bit32(SH_ALLEXPORT),
	"bgnice",		bit32(SH_BGNICE),
	"emacs",		bit32(SH_EMACS),
	"errexit",		bit32(SH_ERREXIT),
	"gmacs",		bit32(SH_GMACS),
	"ignoreeof",		bit32(SH_IGNOREEOF),
	"interactive",		bit32(SH_INTERACTIVE),
	"keyword",		bit32(SH_KEYWORD),
	"markdirs",		bit32(SH_MARKDIRS),
	"monitor",		bit32(SH_MONITOR),
	"noexec",		bit32(SH_NOEXEC),
	"noclobber",		bit32(SH_NOCLOBBER),
	"noglob",		bit32(SH_NOGLOB),
	"nolog",		bit32(SH_NOLOG),
	"notify",		bit32(SH_NOTIFY),
	"nounset",		bit32(SH_NOUNSET),
	"privileged",		bit32(SH_PRIVILEGED),
	"restricted",		bit32(SH_RESTRICTED),
	"trackall",		bit32(SH_TRACKALL),
	"verbose",		bit32(SH_VERBOSE),
	"vi",			bit32(SH_VI),
	"viraw",		bit32(SH_VIRAW),
	"xtrace",		bit32(SH_XTRACE),
	"",			0
};

const Shtable_t shtab_attributes[] =
{
	{"-nnameref",	NV_REF},
	{"-xexport",	NV_EXPORT},
	{"-rreadonly",	NV_RDONLY},
	{"-ttagged",	NV_TAGGED},
	{"-Eexponential",(NV_INTEGER|NV_DOUBLE|NV_EXPNOTE)},
	{"-Ffloat",	(NV_INTEGER|NV_DOUBLE)},
	{"++short",	(NV_INTEGER|NV_SHORT)},
	{"++unsigned",	(NV_INTEGER|NV_UNSIGN)},
	{"-iinteger",	NV_INTEGER},
	{"-Hfilename",	NV_HOST},
	{"-llowercase",	NV_UTOL},
	{"-Zzerofill",	NV_ZFILL},
	{"-Lleftjust",	NV_LJUST},
	{"-Rrightjust",	NV_RJUST},
	{"-uuppercase",	NV_LTOU},
	{"-Aarray",	NV_ARRAY},
	{"",		0}
};
