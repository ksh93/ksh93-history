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
*         THIS IS PROPRIETARY SOURCE CODE LICENSED BY          *
*                          AT&T CORP.                          *
*                                                              *
*                Copyright (c) 1995 AT&T Corp.                 *
*                     All Rights Reserved                      *
*                                                              *
*           This software is licensed by AT&T Corp.            *
*       under the terms and conditions of the license in       *
*       http://www.research.att.com/orgs/ssr/book/reuse        *
*                                                              *
*               This software was created by the               *
*           Software Engineering Research Department           *
*                    AT&T Bell Laboratories                    *
*                                                              *
*               For further information contact                *
*                     gsf@research.att.com                     *
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
#define OPT_ALIAS		(1<<0)
#define OPT_DISABLE		(1<<1)
#define OPT_HEADER		(1<<2)
#define OPT_READONLY		(1<<3)
#define OPT_SET			(1<<4)

#define OPT_environ		(-1)

#define OPT_append		1
#define OPT_atime		2
#define OPT_base		3
#define OPT_blocksize		4
#define OPT_blok		5
#define OPT_charset		6
#define OPT_chksum		7
#define OPT_clobber		8
#define OPT_comment		9
#define OPT_complete		10
#define OPT_crossdevice		11
#define OPT_ctime		12
#define OPT_debug		13
#define OPT_delta		14
#define OPT_descend		15
#define OPT_device		16
#define OPT_devmajor		17
#define OPT_devminor		18
#define OPT_dots		19
#define OPT_edit		20
#define OPT_eom			21
#define OPT_exact		22
#define OPT_extended_ignore	23
#define OPT_extended_path	24
#define OPT_file		25
#define OPT_filter		26
#define OPT_format		27
#define OPT_gname		28
#define OPT_help		29
#define OPT_ignore		30
#define OPT_ino			31
#define OPT_invalid		32
#define OPT_invert		33
#define OPT_keepgoing		34
#define OPT_label		35
#define OPT_label_insert	36
#define OPT_link		37
#define OPT_linkdata		38
#define OPT_linkop		39
#define OPT_linkpath		40
#define OPT_listformat		41
#define OPT_listmacro		42
#define OPT_logical		43
#define OPT_magic		44
#define OPT_mark		45
#define OPT_maxblocks		46
#define OPT_metaphysical	47
#define OPT_mkdir		48
#define OPT_mode		49
#define OPT_mtime		50
#define OPT_name		51
#define OPT_nlink		52
#define OPT_ordered		53
#define OPT_owner		54
#define OPT_path		55
#define OPT_physical		56
#define OPT_preserve		57
#define OPT_read		58
#define OPT_record_charset	59
#define OPT_record_delimiter	60
#define OPT_record_format	61
#define OPT_record_header	62
#define OPT_record_line		63
#define OPT_record_match	64
#define OPT_record_pad		65
#define OPT_record_size		66
#define OPT_record_trailer	67
#define OPT_sequence		68
#define OPT_size		69
#define OPT_summary		70
#define OPT_symlink		71
#define OPT_tape		72
#define OPT_test		73
#define OPT_typeflag		74
#define OPT_uname		75
#define OPT_unblocked		76
#define OPT_update		77
#define OPT_verbose		78
#define OPT_verify		79
#define OPT_version		80
#define OPT_write		81
#define OPT_yes			82

typedef struct
{
	char*		string;
	long		number;
	int		size;
} Value_t;

typedef struct
{
	char*		name;
	short		flag;
	short		index;
	char*		description;
	short		flags;
	short		entry;
	short		level;
	char*		macro;
	Value_t		perm;
	Value_t		temp;
} Option_t;

extern __MANGLE__ Option_t		options[];
