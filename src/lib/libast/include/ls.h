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
 * License along with these libraries and programs; if not, write
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
                  
/*
 * Glenn Fowler
 * AT&T Bell Laboratories
 *
 * ls formatter interface definitions
 */

#ifndef _LS_H
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

#define _LS_H

#include <ast_fs.h>
#include <ast_mode.h>

/*
 * some systems (could it beee AIX) pollute the std name space
 */

#undef	fileid
#define fileid	fileID

#if _mem_st_blocks_stat
#define iblocks(p)	(((p)->st_blocks+1)/2)
#else
#define iblocks(p)	_iblocks(p)
extern __MANGLE__ off_t		_iblocks __PROTO__((struct stat*));
#endif

#if _mem_st_rdev_stat
#define idevice(p)	((p)->st_rdev)
#define IDEVICE(p,v)	((p)->st_rdev=(v))
#else
#define idevice(p)	0
#define IDEVICE(p,v)
#endif

#define LS_ATIME	(1<<0)		/* list st_atime		*/
#define LS_BLOCKS	(1<<1)		/* list blocks used by file	*/
#define LS_CTIME	(1<<2)		/* list st_ctime		*/
#define LS_EXTERNAL	(1<<3)		/* st_mode is modex canonical	*/
#define LS_INUMBER	(1<<4)		/* list st_ino			*/
#define LS_LONG		(1<<5)		/* long listing			*/
#define LS_MARK		(1<<6)		/* append file name marks	*/
#define LS_NOGROUP	(1<<7)		/* omit group name for LS_LONG	*/
#define LS_NOUSER	(1<<8)		/* omit user name for LS_LONG	*/
#define LS_NUMBER	(1<<9)		/* number instead of name	*/

#define LS_USER		(1<<10)		/* first user flag bit		*/

#define LS_W_BLOCKS	6		/* LS_BLOCKS field width	*/
#define LS_W_INUMBER	7		/* LS_INUMBER field width	*/
#define LS_W_LONG	55		/* LS_LONG width (w/o names)	*/
#define LS_W_LINK	4		/* link text width (w/o names)	*/
#define LS_W_MARK	1		/* LS_MARK field width		*/
#define LS_W_NAME	9		/* group|user name field width	*/

#if defined(_AST_H) || defined(_POSIX_SOURCE)
#define _AST_mode_t	mode_t
#else
#define _AST_mode_t	int
#endif

extern __MANGLE__ char*		fmtls __PROTO__((char*, const char*, struct stat*, const char*, const char*, int));
extern __MANGLE__ int		chmod __PROTO__((const char*, _AST_mode_t));
#if !defined(_ver_fstat)
extern __MANGLE__ int		fstat __PROTO__((int, struct stat*));
#endif
#if !defined(_ver_lstat)
extern __MANGLE__ int		lstat __PROTO__((const char*, struct stat*));
#endif
extern __MANGLE__ int		mkdir __PROTO__((const char*, _AST_mode_t));
extern __MANGLE__ int		mkfifo __PROTO__((const char*, _AST_mode_t));
#if !defined(_ver_mknod)
extern __MANGLE__ int		mknod __PROTO__((const char*, _AST_mode_t, dev_t));
#endif
extern __MANGLE__ int		pathstat __PROTO__((const char*, struct stat*));
#if !defined(_ver_stat)
extern __MANGLE__ int		stat __PROTO__((const char*, struct stat*));
#endif
extern __MANGLE__ _AST_mode_t	umask __PROTO__((_AST_mode_t));

#undef	_AST_mode_t

#endif
