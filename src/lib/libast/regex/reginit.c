/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1985-2000 AT&T Corp.                *
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
*               Glenn Fowler <gsf@research.att.com>                *
*                David Korn <dgk@research.att.com>                 *
*                 Phong Vo <kpv@research.att.com>                  *
*                                                                  *
*******************************************************************/
#pragma prototyped

/*
 * posix regex state and alloc
 */

#include "reglib.h"

/*
 * state shared by all threads
 */

State_t		state =
{
	{ REX_DONE },
	{ -1, -1 },

	/*
	 * escape code table;  the "funny" things get special
	 * treatment at ends of BRE
	 *
	 *	0:BRE unescaped 1:BRE escaped
	 *	2:ERE unescaped 3:ERE escaped
	 *	4:ARE unescaped 5:ARE escaped
	 *	6:SRE unescaped 7:SRE escaped
	 *	8:KRE unescaped 9:KRE escaped
	 */

	'\\',
		'\\', 	'\\', 
		'\\',	'\\',
		'\\',	'\\',
		'\\',	'\\',
		'\\',	'\\',
	'^',			/* funny */
		'^',	'^',
		T_CFLX, '^',
		T_CFLX, '^',
		'^',	'^',
		'^',	'^',
	'.',
		T_DOT,	'.',
		T_DOT, 	'.',
		T_DOT, 	'.',
		'.',	'.',
		'.',	'.',
	'$',			/* funny */
		'$',	'$',
		T_DOLL, '$',
		T_DOLL, '$',
		'$',	'$',
		'$',	'$',
	'*',
		T_STAR,	'*',
		T_STAR, '*',
		T_STAR, '*',
		T_STAR, '*',
		T_STAR, '*',
	'[',
		T_BRA,	'[',
		T_BRA,	'[',
		T_BRA,	'[',
		T_BRA,	'[',
		T_BRA,	'[',
	'|',
		'|',	T_BAD,
		T_BAR,	'|',
		T_BAR,	'|',
		'|',	'|',
		T_BAR,	'|',
	'+',
		'+',	T_BAD,
		T_PLUS,	'+',
		T_PLUS,	'+',
		'+',	'+',
		T_PLUS,	'+',
	'?',
		'?',	T_BAD,
		T_QUES, '?',
		T_QUES, '?',
		T_QUES,	'?',
		T_QUES,	'?',
	'(',
		'(',	T_OPEN,
		T_OPEN, '(',
		T_OPEN, '(',
		'(',	'(',
		T_OPEN,	'(',
	')',
		')',	T_CLOSE,
		T_CLOSE,')',
		T_CLOSE,')',
		')',	')',
		T_CLOSE,')',
	'{',
		'{',	T_LEFT,
		T_LEFT,	'{',
		T_LEFT,	'{',
		'{',	'{',
		T_LEFT,	'{',
	'}',
		'}',	T_RIGHT, 
		'}',	T_BAD,
		'}',	T_BAD,
		'}',	'}',
		'}',	'}',
	'&',
		'&',	T_BAD,
		'&',	T_AND,
		T_AND,	'&',
		'&',	'&',
		T_AND,	'&',
	'!',
		'!',	T_BAD,
		'!',	T_BANG,
		T_BANG, '!',
		'!',	'!',
		T_BANG, '!',
	'@',
		'@',	T_BAD,
		'@',	T_BAD,
		'@',	T_BAD,
		'@',	'@',
		T_AT,	'@',
	'<',
		'<',	T_LT,
		'<',	T_LT,
		T_LT,   '<',
		'<',	'<',
		'<',	'<',
	'>',
		'>',	T_GT,
		'>',	T_GT,
		T_GT,   '>',
		'>',	'>',
		'>',	'>',
	'1',
		'1',	T_BACK+1,
		'1',	T_BACK+1,
		'1',	T_BACK+1,
		'1',	'1',
		'1',	T_BACK+1,
	'2',
		'2',	T_BACK+2,
		'2',	T_BACK+2,
		'2',	T_BACK+2,
		'2',	'2',
		'2',	T_BACK+2,
	'3',
		'3',	T_BACK+3,
		'3',	T_BACK+3,
		'3',	T_BACK+3,
		'3',	'3',
		'3',	T_BACK+3,
	'4',
		'4',	T_BACK+4,
		'4',	T_BACK+4,
		'4',	T_BACK+4,
		'4',	'4',
		'4',	T_BACK+4,
	'5',
		'5',	T_BACK+5,
		'5',	T_BACK+5,
		'5',	T_BACK+5,
		'5',	'5',
		'5',	T_BACK+5,
	'6',
		'6',	T_BACK+6,
		'6',	T_BACK+6,
		'6',	T_BACK+6,
		'6',	'6',
		'6',	T_BACK+6,
	'7',
		'7',	T_BACK+7,
		'7',	T_BACK+7,
		'7',	T_BACK+7,
		'7',	'7',
		'7',	T_BACK+7,
	'8',
		'8',	T_BACK+8,
		'8',	T_BACK+8,
		'8',	T_BACK+8,
		'8',	'8',
		'8',	T_BACK+8,
	'9',
		'9',	T_BACK+9,
		'9',	T_BACK+9,
		'9',	T_BACK+9,
		'9',	'9',
		'9',	T_BACK+9,
};

/*
 * all allocation/free done here
 * interface compatible with vmresize()
 *
 *	malloc(n)	alloc(0,n)
 *	realloc(p,n)	alloc(p,n)
 *	free(p)		alloc(p,0)
 */

void*
alloc(register regdisc_t* disc, void* p, size_t n)
{
	if (disc->re_resizef)
	{
		if (!n && (disc->re_flags & REG_NOFREE))
			return 0;
		return (*disc->re_resizef)(disc->re_resizehandle, p, n);
	}
	else if (!n)
	{
		if (!(disc->re_flags & REG_NOFREE))
			free(p);
		return 0;
	}
	else if (p)
		return realloc(p, n);
	else
		return malloc(n);
}
