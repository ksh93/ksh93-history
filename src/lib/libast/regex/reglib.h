/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1985-2001 AT&T Corp.                *
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
*******************************************************************/
#pragma prototyped

/*
 * posix regex implementation
 *
 * based on Doug McIlroy's C++ implementation
 * Knuth-Morris-Pratt adapted from Corman-Leiserson-Rivest
 * Boyer-Moore from conversations with David Korn, Phong Vo, Andrew Hume
 */

#ifndef _REGLIB_H
#define _REGLIB_H

#define re_info		env

#include <ast.h>
#include <stk.h>
#include <ctype.h>
#include <errno.h>

#define alloc		_reg_alloc
#define drop		_reg_drop
#define fatal		_reg_fatal
#define state		_reg_state

#include "regex.h"

#undef	RE_DUP_MAX			/* posix puts this in limits.h!	*/
#define RE_DUP_MAX	(INT_MAX/2-1)	/* 2*RE_DUP_MAX won't overflow	*/
#define RE_DUP_INF	(RE_DUP_MAX+1)	/* infinity, for *		*/
#define BACK_REF_MAX	9

#define REG_COMP	(REG_EXTENDED|REG_ICASE|REG_NOSUB|REG_NEWLINE|REG_SHELL|REG_AUGMENTED|REG_LEFT|REG_LITERAL|REG_MINIMAL|REG_NULL|REG_RIGHT|REG_LENIENT)
#define REG_EXEC	(REG_INVERT|REG_NOTBOL|REG_NOTEOL)

#define REX_NULL	0		/* null string (internal)	*/
#define REX_ALT		1		/* a|b				*/
#define REX_ALT_CATCH	2		/* REX_ALT catcher		*/
#define REX_BACK	3		/* \1, \2, etc			*/
#define REX_BEG		4		/* initial ^			*/
#define REX_BM		5		/* Boyer-Moore			*/
#define REX_CLASS	6		/* [...]			*/
#define REX_CONJ	7		/* a&b				*/
#define REX_CONJ_LEFT	8		/* REX_CONJ left catcher	*/
#define REX_CONJ_RIGHT	9		/* REX_CONJ right catcher	*/
#define REX_DONE	10		/* completed match (internal)	*/
#define REX_DOT		11		/* .				*/
#define REX_END		12		/* final $			*/
#define REX_GROUP	13		/* \(...\)			*/
#define REX_GROUP_CATCH	14		/* REX_GROUP catcher		*/
#define REX_KMP		15		/* Knuth-Morris-Pratt		*/
#define REX_NEG		16		/* negation			*/
#define REX_NEG_CATCH	17		/* REX_NEG catcher		*/
#define REX_ONECHAR	18		/* a single-character literal	*/
#define REX_REP		19		/* Kleene closure		*/
#define REX_REP_CATCH	20		/* REX_REP catcher		*/
#define REX_STRING	21		/* some chars			*/
#define REX_TRIE	22		/* alternation of strings	*/
#define REX_WBEG	23		/* \<				*/
#define REX_WEND	24		/* \>				*/

#define T_META		(UCHAR_MAX+1)
#define T_STAR		(T_META+0)
#define T_PLUS		(T_META+1)
#define T_QUES		(T_META+2)
#define T_BANG		(T_META+3)
#define T_AT		(T_META+4)
#define T_LEFT		(T_META+5)
#define T_OPEN		(T_META+6)
#define T_CLOSE		(T_OPEN+1)
#define T_RIGHT		(T_OPEN+2)
#define T_CFLX		(T_OPEN+3)
#define T_DOT		(T_OPEN+4)
#define T_DOTSTAR	(T_OPEN+5)
#define T_END		(T_OPEN+6)
#define T_BAD		(T_OPEN+7)
#define T_DOLL		(T_OPEN+8)
#define T_BRA		(T_OPEN+9)
#define T_BAR		(T_OPEN+10)
#define T_AND		(T_OPEN+11)
#define T_LT		(T_OPEN+12)
#define T_GT		(T_OPEN+13)
#define T_SLASHPLUS	(T_OPEN+14)
#define T_BACK		(T_OPEN+15)

#define BRE		0
#define ERE		2
#define ARE		4
#define SRE		6
#define KRE		8

#define HIT		SSIZE_MAX

#define bitclr(p,c)	((p)[(c)>>3]&=(~(1<<((c)&07))))
#define bitset(p,c)	((p)[(c)>>3]|=(1<<((c)&07)))
#define bittst(p,c)	((p)[(c)>>3]&(1<<((c)&07)))

#define setadd(p,c)	bitset((p)->bits,c)
#define setclr(p,c)	bitclr((p)->bits,c)
#define settst(p,c)	bittst((p)->bits,c)

/*
 * private stuff hanging off regex_t
 */

typedef struct
{
	off_t		offset;
	char*		base;
} Stk_pos_t;

typedef struct
{
	Stk_t*		stk;		/* stack pointer		*/
	char*		vec;		/* the data			*/
	int		inc;		/* growth increment		*/
	int		siz;		/* element size			*/
	int		max;		/* max index			*/
	int		cur;		/* current index -- user domain	*/
} Vector_t;

typedef struct Reginfo			/* library private regex_t info	*/
{
	struct Rex*	rex;		/* compiled expression		*/
	regdisc_t*	disc;		/* REG_DISCIPLINE discipline	*/
	unsigned char*	map;		/* for REG_ICASE folding	*/
	const regex_t*	regex;		/* from regexec			*/
	unsigned char*	beg;		/* beginning of string		*/
	unsigned char*	end;		/* end of string		*/
	Vector_t*	pos;		/* posns of certain subpatterns	*/
	Vector_t*	bestpos;	/* ditto for best match		*/
	regmatch_t*	match;		/* subexrs in current match 	*/
	regmatch_t*	best;		/* ditto in best match yet	*/
	Stk_pos_t	stk;		/* exec stack pos		*/
	size_t		min;		/* minimum match length		*/
	regflags_t	flags;		/* flags from regcomp()		*/
	int		error;		/* last error			*/
	int		explicit;	/* explicit match on this char	*/
	unsigned char	hard;		/* hard comp			*/
	unsigned char	leading;	/* explicit match on leading .	*/
	unsigned char	once;		/* if 1st parse fails, quit	*/
	unsigned char	stack;		/* hard comp or exec		*/
} Env_t;

/*
 * Rex_t subtypes
 */

typedef struct
{
	unsigned char*	beg;		/* beginning of left match	*/
	struct Rex*	right;		/* right pattern		*/
	struct Rex*	cont;		/* right catcher		*/
} Conj_left_t;

typedef struct
{
	unsigned char*	end;		/* end of left match		*/
	struct Rex*	cont;		/* ambient continuation		*/
} Conj_right_t;

typedef unsigned int Bm_mask_t;

typedef struct
{
	Bm_mask_t**	mask;
	size_t*		skip;
	size_t*		fail;
	size_t		size;
	ssize_t		left;
	ssize_t		right;
	size_t		complete;
} Bm_t;

typedef struct
{
	int*		fail;
	unsigned char*	base;
	size_t		size;
} String_t;

typedef struct
{
	unsigned char	bits[(UCHAR_MAX+1)/CHAR_BIT];
} Set_t;

typedef struct
{
	struct Rex*	left;
	struct Rex*	right;
	int		serial;
} Binary_t;

typedef struct
{
	int		number;		/* group number			*/
	int		last;		/* last contained group number	*/
	union
	{
	Binary_t	binary;
	struct Rex*	rex;
	}		expr;
} Group_t;

/*
 * REX_ALT catcher, solely to get control at the end of an
 * alternative to keep records for comparing matches.
 */

typedef struct
{
	struct Rex*	cont;
} Alt_catch_t;

typedef struct
{
	struct Rex*	cont;
	regoff_t*	eo;
} Group_catch_t;

/*
 * REX_NEG catcher determines what string lengths can be matched,
 * then Neg investigates continuations of other lengths.
 * This is inefficient.  For !POSITIONS expressions, we can do better:
 * since matches to rex will be enumerated in decreasing order,
 * we can investigate continuations whenever a length is skipped.
 */

typedef struct
{
	unsigned char*	beg;
	unsigned char*	index;
} Neg_catch_t;

/*
 * REX_REP catcher.  One is created on the stack for
 * each iteration of a complex repetition.
 */

typedef struct
{
	struct Rex*	cont;
	struct Rex*	ref;
	unsigned char*	beg;
	int		n;
} Rep_catch_t;

/*
 * data structure for an alternation of pure strings
 * son points to a subtree of all strings with a common
 * prefix ending in character c.  sib links alternate
 * letters in the same position of a word.  end=1 if
 * some word ends with c.  the order of strings is
 * irrelevant, except long words must be investigated
 * before short ones.
 */

typedef struct Trie
{
	unsigned char	c;
	unsigned char	end;
	struct Trie*	son;
	struct Trie*	sib;
} Trie_node_t;

typedef struct
{
	Trie_node_t**	root;
	int		min;
} Trie_t;

/*
 * Rex_t is a node in a regular expression
 */

typedef struct Rex
{
	short		type;			/* node type		*/
	short		serial;			/* subpattern number	*/
	struct Rex*	next;			/* remaining parts	*/
	int		lo;			/* lo dup count		*/
	int		hi;			/* hi dup count		*/
	union
	{
	Alt_catch_t	alt_catch;		/* alt catcher		*/
	Bm_t		bm;			/* bm			*/
	Set_t*		charclass;		/* char class		*/
	Conj_left_t	conj_left;		/* conj left catcher	*/
	Conj_right_t	conj_right;		/* conj right catcher	*/
	void*		data;			/* data after Rex_t	*/
	Group_t		group;			/* a|b or rep		*/
	Group_catch_t	group_catch;		/* group catcher	*/
	Neg_catch_t	neg_catch;		/* neg catcher		*/
	unsigned char	onechar;		/* single char		*/
	Rep_catch_t	rep_catch;		/* rep catcher		*/
	String_t	string;			/* string/kmp		*/
	Trie_t		trie;			/* trie			*/
	}		re;
} Rex_t;

typedef struct					/* shared state		*/
{
	Rex_t		done;
	regmatch_t	nomatch;
	struct
	{
	unsigned char	key;
	short		val[10];
	}		escape[27];
	unsigned char	ident[UCHAR_MAX+1];
	unsigned char	fold[UCHAR_MAX+1];
	short*		magic[UCHAR_MAX+1];
	regdisc_t	disc;
} State_t;

extern State_t		state;

extern void*		alloc(regdisc_t*, void*, size_t);
extern void		drop(regdisc_t*, Rex_t*);

#endif
