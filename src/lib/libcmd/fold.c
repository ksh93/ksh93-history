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
static const char id[] = "\n@(#)fold (AT&T Bell Laboratories) 04/01/92\0\n";

#include <cmdlib.h>

#define WIDTH	80
#define TABSIZE	8

#define T_EOF	1
#define T_NL	2
#define T_BS	3
#define T_TAB	4
#define T_SP	5
#define T_RET	6

static char cols[1<<CHAR_BIT];

static void fold __PARAM__((Sfio_t *in, Sfio_t *out, int width), (in, out, width)) __OTORP__(Sfio_t *in; Sfio_t *out; int width;){
	char *cp, *first;
	int n, col=0;
	char *last_space=0;
	cols[0] = 0;
	while(cp  = sfgetr(in,'\n',0))
	{
		/* special case -b since no column adjustment is needed */ 
		if(cols['\b']==0 && (n=sfslen())<=width)
		{
			sfwrite(out,cp,n);
			continue;
		}
		first = cp;
		col = 0;
		last_space = 0;
		while(1)
		{
			while((n=cols[*(unsigned char*)cp++])==0);
			while((cp-first) > (width-col))
			{
				if(last_space)
					col = last_space - first;
				else
					col = width-col;
				sfwrite(out,first,col);
				first += col;
				col = 0;
				last_space = 0;
				if(cp>first+1 || (n!=T_NL && n!=T_BS))
					sfputc(out,'\n');
			}
			if(n==T_NL)
				break;
			switch(n)
			{
			    case T_RET:
				col = 0;
				break;
			    case T_BS:
				if((cp+(--col)-first)>0) 
					col--;
				break;
			    case T_TAB:
				n = (TABSIZE-1) - (cp+col-1-first)&(TABSIZE-1);
				col +=n;
				if((cp-first) > (width-col))
				{
					sfwrite(out,first,(--cp)-first);
					sfputc(out,'\n');
					first = cp;
					col =  TABSIZE-1;
					last_space = 0;
					continue;
				}
				if(cols[' '])
					last_space = cp;
				break;
			    case T_SP:
				last_space = cp;
				break;
			}
		}
		sfwrite(out,first,cp-first);
	}
	if(cp = sfgetr(in,'\n',-1))
		sfwrite(out,cp,sfslen());
}

int
b_fold __PARAM__((int argc, char *argv[]), (argc, argv)) __OTORP__(int argc; char *argv[];){
	int n, width=WIDTH;
	Sfio_t *fp;
	char *cp;

	NoP(id[0]);
	cmdinit(argv);
	cols['\t'] = T_TAB;
	cols['\b'] = T_BS;
	cols['\n'] = T_NL;
	cols['\r'] = T_RET;
	while (n = optget(argv, "bsw#[width] [file...]")) switch (n)
	{
	    case 'b':
		cols['\r'] = cols['\b'] = 0;
		cols['\t'] = cols[' '];
		break;
	    case 's':
		cols[' '] = T_SP;
		if(cols['\t']==0)
			cols['\t'] = T_SP;
		break;
	    case 'w':
		if((width =  opt_info.num) <= 0)
			error(2, "%d: width must be positive", opt_info.num);
		break;
	    case ':':
		error(2, opt_info.arg);
		break;
	    case '?':
		error(ERROR_usage(2), opt_info.arg);
		break;
	}
	argv += opt_info.index;
	argc -= opt_info.index;
	if(error_info.errors)
		error(ERROR_usage(2),optusage(NiL));
	if(cp = *argv)
		argv++;
	do
	{
		if(!cp || streq(cp,"-"))
			fp = sfstdin;
		else if(!(fp = sfopen(NiL,cp,"r")))
		{
			error(ERROR_system(0),"%s: cannot open",cp);
			error_info.errors = 1;
			continue;
		}
		fold(fp,sfstdout,width);
		if(fp!=sfstdin)
			sfclose(fp);
	}
	while(cp= *argv++);
	return(error_info.errors);
}
