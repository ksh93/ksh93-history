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
*        THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF        *
*                    AT&T BELL LABORATORIES                    *
*         AND IS NOT TO BE DISCLOSED OR USED EXCEPT IN         *
*            ACCORDANCE WITH APPLICABLE AGREEMENTS             *
*                                                              *
*          Copyright (c) 1994 AT&T Bell Laboratories           *
*              Unpublished & Not for Publication               *
*                     All Rights Reserved                      *
*                                                              *
*       The copyright notice above does not evidence any       *
*      actual or intended publication of such source code      *
*                                                              *
*               This software was created by the               *
*           Software Engineering Research Department           *
*                    AT&T Bell Laboratories                    *
*                                                              *
*               For further information contact                *
*                   advsoft@research.att.com                   *
*                 Randy Hackbarth 908-582-5245                 *
*                  Dave Belanger 908-582-7427                  *
*                                                              *
***************************************************************/

/* : : generated by proto : : */

#line 1

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

#line 21
#include <ast.h>
#include <error.h>
#include <ls.h>
#include <proc.h>

#ifndef PROBE
#define PROBE		"probe"
#endif

char*
pathprobe __PARAM__((char* path, char* attr, const char* lang, const char* tool, const char* aproc, int op), (path, attr, lang, tool, aproc, op)) __OTORP__(char* path; char* attr; const char* lang; const char* tool; const char* aproc; int op;)
#line 32
{
	char*		proc = (char*)aproc;
	char*	p;
	char*	k;
	char**	ap;
	int		n;
	char*		e;
	char*		probe;
	char		buf[PATH_MAX];
	char		cmd[PATH_MAX];
	char		lib[PATH_MAX];
	char*		arg[6];
	unsigned long	ptime;
	struct stat	st;

	if (*proc != '/')
	{
		if (p = strchr(proc, ' '))
		{
			n = p - proc;
			proc = strncpy(buf, proc, n);
			*(proc + n) = 0;
		}
		if (!(proc = pathpath(cmd, proc, NiL, PATH_ABSOLUTE|PATH_REGULAR|PATH_EXECUTE)))
			proc = (char*)aproc;
		else if (p)
			strcpy(proc + strlen(proc), p);
	}
	if (!path) path = buf;
	probe = PROBE;
	p = strcopy(lib, "lib/");
	p = strcopy(p, probe);
	*p++ = '/';
	p = strcopy(k = p, lang);
	*p++ = '/';
	p = strcopy(p, tool);
	*p++ = '/';
	e = strcopy(p, probe);
	if (!pathpath(path, lib, "", PATH_ABSOLUTE|PATH_EXECUTE) || stat(path, &st)) return(0);
	ptime = st.st_mtime;
	pathkey(p, attr, lang, proc);
	p = path + strlen(path) - (e - k);
	strcpy(p, k);
	if (op >= 0 && !stat(path, &st))
	{
		if (ptime <= (unsigned long)st.st_mtime || ptime <= (unsigned long)st.st_ctime) op = -1;
		else if (st.st_mode & S_IWUSR)
		{
			if (op == 0) error(0, "%s probe information for %s language processor %s must be manually regenerated", tool, lang, proc);
			op = -1;
		}
	}
	if (op >= 0)
	{
		strcpy(p, probe);
		ap = arg;
		*ap++ = path;
		if (op > 0) *ap++ = "-s";
		*ap++ = (char*)lang;
		*ap++ = (char*)tool;
		*ap++ = proc;
		*ap = 0;
		if (procrun(path, arg)) return(0);
		strcpy(p, k);
		if (access(path, 4)) return(0);
	}
	return(path == buf ? strdup(path) : path);
}
