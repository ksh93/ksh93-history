/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1982-2001 AT&T Corp.                *
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
*                David Korn <dgk@research.att.com>                 *
*******************************************************************/
#pragma prototyped
#ifndef SEQPOINT
/*
 * D. G. Korn
 *
 * arithmetic expression evaluator
 */

/* The following only is needed for const */
#include	<ast.h>
#include	<math.h>

struct lval
{
	char	*value;
	double	(*fun)(double,...);
	const char *expr;
	short	flag;
	char	isfloat;
	char	nargs;
	short	emode;
	short	level;
};

struct mathtab
{
	char	fname[8];
	double	(*fnptr)(double,...);
};

typedef struct _arith_
{
	unsigned char	*code;
	const char	*expr;
	double		(*fun)(const char**,struct lval*,int,double);
	short		size;
	short		staksize;
	int		emode;
} Arith_t;
#define ARITH_COMP	04	/* set when compile separate from execute */

#define MAXPREC		15	/* maximum precision level */
#define SEQPOINT	0200	/* sequence point */
#define NOASSIGN	0100	/* assignment legal with this operator */
#define RASSOC		040	/* right associative */
#define NOFLOAT		020	/* illegal with floating point */
#define PRECMASK	017	/* precision bit mask */

#define A_EOF		1
#define A_NEQ		2
#define A_NOT		3
#define A_MOD		4
#define A_ANDAND	5
#define A_AND		6
#define A_LPAR		7
#define A_RPAR		8
#define A_TIMES		9
#define A_PLUSPLUS	10
#define A_PLUS		11
#define A_COMMA		12
#define A_MINUSMINUS	13
#define A_MINUS		14
#define A_DIV		15
#define A_LSHIFT	16
#define A_LE		17
#define A_LT		18
#define A_EQ		19
#define A_ASSIGN	20
#define A_COLON		21
#define A_RSHIFT	22
#define A_GE		23
#define A_GT		24
#define A_QCOLON	25
#define A_QUEST		26
#define A_XOR		27
#define A_OROR		28
#define A_OR		29
#define A_TILDE		30
#define A_REG		31
#define A_DIG		32
#define A_INCR          33
#define A_DECR          34
#define A_NOTNOT        35
#define A_PUSHV         36
#define A_PUSHL         37
#define A_PUSHN         38
#define A_PUSHF         39
#define A_STORE         40
#define A_POP           41
#define A_SWAP          42
#define A_UMINUS	43
#define A_JMPZ          44
#define A_JMPNZ         45
#define A_JMP           46
#define A_CALL1         47
#define A_CALL2         48
#define A_CALL3         49
#define A_DOT		50
#define A_LIT		51


/* define error messages */
extern const unsigned char	strval_precedence[34];
extern const char		strval_states[64];
extern const char		e_moretokens[];
extern const char		e_argcount[];
extern const char		e_paren[];
extern const char		e_badnum[];
extern const char		e_badcolon[];
extern const char		e_recursive[];
extern const char		e_divzero[];
extern const char		e_synbad[];
extern const char		e_notlvalue[];
extern const char		e_function[];
extern const char		e_questcolon[];
extern const char		e_incompatible[];
extern const char		e_domain[];
extern const char		e_overflow[];
extern const char		e_singularity[];
extern const char		e_dict[];
extern const char		e_charconst[];
extern const struct 		mathtab shtab_math[];

/* function code for the convert function */

#define LOOKUP	0
#define ASSIGN	1
#define VALUE	2
#define ERRMSG	3

extern double strval(const char*,char**,double(*)(const char**,struct lval*,int,double),int);
extern Arith_t *arith_compile(const char*,char**,double(*)(const char**,struct lval*,int,double),int);
extern double arith_exec(Arith_t*);
#endif /* !SEQPOINT */
