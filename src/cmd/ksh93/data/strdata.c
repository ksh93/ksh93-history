/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1982-2002 AT&T Corp.                *
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
*            Information and Software Systems Research             *
*                        AT&T Labs Research                        *
*                         Florham Park NJ                          *
*                                                                  *
*                David Korn <dgk@research.att.com>                 *
*******************************************************************/
#pragma prototyped
/*
 * data for string evaluator library
 */

#include	"FEATURE/options"
#include	"streval.h"

const unsigned char strval_precedence[35] =
	/* opcode	precedence,assignment  */
{
	/* DEFAULT */		MAXPREC|NOASSIGN,
	/* DONE */		0|NOASSIGN|RASSOC,
	/* NEQ */		10|NOASSIGN,
	/* NOT */		MAXPREC|NOASSIGN,
	/* MOD */		14,
	/* ANDAND */		6|NOASSIGN|SEQPOINT,
	/* AND */		9|NOFLOAT,
	/* LPAREN */		MAXPREC|NOASSIGN|SEQPOINT,
	/* RPAREN */		1|NOASSIGN|RASSOC|SEQPOINT,
	/* POW */		14|NOASSIGN|RASSOC,
	/* TIMES */		14,
	/* PLUSPLUS */		15|NOASSIGN|NOFLOAT|SEQPOINT,
	/* PLUS */		13,	
	/* COMMA */		1|NOASSIGN|SEQPOINT,
	/* MINUSMINUS */	15|NOASSIGN|NOFLOAT|SEQPOINT,
	/* MINUS */		13,
	/* DIV */		14,
	/* LSHIFT */		12|NOFLOAT,
	/* LE */		11|NOASSIGN,
	/* LT */		11|NOASSIGN,	
	/* EQ */		10|NOASSIGN,
	/* ASSIGNMENT */	2|RASSOC,
	/* COLON */		0|NOASSIGN,
	/* RSHIFT */		12|NOFLOAT,	
	/* GE */		11|NOASSIGN,
	/* GT */		11|NOASSIGN,
	/* QCOLON */		3|NOASSIGN|SEQPOINT,
	/* QUEST */		3|NOASSIGN|SEQPOINT|RASSOC,
	/* XOR */		8|NOFLOAT,
	/* OROR */		5|NOASSIGN|SEQPOINT,
	/* OR */		7|NOFLOAT,
	/* DEFAULT */		MAXPREC|NOASSIGN,
	/* DEFAULT */		MAXPREC|NOASSIGN,
	/* DEFAULT */		MAXPREC|NOASSIGN,
	/* DEFAULT */		MAXPREC|NOASSIGN
};

/*
 * This is for arithmetic expressions
 */
const char strval_states[64] =
{
	A_EOF,	A_REG,	A_REG,	A_REG,	A_REG,	A_REG,	A_REG,	A_REG,
	A_REG,	0,	0,	A_REG,	A_REG,	A_REG,	A_REG,	A_REG,
	A_REG,	A_REG,	A_REG,	A_REG,	A_REG,	A_REG,	A_REG,	A_REG,
	A_REG,	A_REG,	A_REG,	A_REG,	A_REG,	A_REG,	A_REG,	A_REG,

	0,	A_NOT,	A_REG,	A_REG,	A_REG,	A_MOD,	A_AND,	A_LIT,
	A_LPAR,	A_RPAR,	A_TIMES,A_PLUS,	A_COMMA,A_MINUS,A_DOT,	A_DIV,
	A_DIG,	A_DIG,	A_DIG,	A_DIG,	A_DIG,	A_DIG,	A_DIG,	A_DIG,
	A_DIG,	A_DIG,	A_COLON,A_REG,	A_LT,	A_ASSIGN,A_GT,	A_QUEST

};


const char e_argcount[]		= "%s: funcion has wrong number of arguments";
const char e_badnum[]		= "%s: bad number";
const char e_moretokens[]	= "%s: more tokens expected";
const char e_paren[]		= "%s: unbalanced parenthesis";
const char e_badcolon[]		= "%s: invalid use of :";
const char e_divzero[]		= "%s: divide by zero";
const char e_synbad[]		= "%s: arithmetic syntax error";
const char e_notlvalue[]	= "%s: assignment requires lvalue";
const char e_recursive[]	= "%s: recursion too deep";
const char e_questcolon[]	= "%s: ':' expected for '?' operator";
const char e_function[]		= "%s: unknown function";
const char e_incompatible[]	= "%s: invalid floating point operation";
const char e_overflow[]		= "%s: overflow exception";
const char e_domain[]		= "%s: domain exception";
const char e_singularity[]	= "%s: singularity exception";
const char e_charconst[]	= "%s: invalid character constant";

typedef double (*mathf)(double,...);

#ifdef __NeXT
extern double	hypot(double, double);
#endif

const struct mathtab shtab_math[] =
{
	"\01abs",		(mathf)fabs,
	"\01acos",		(mathf)acos,
	"\01asin",		(mathf)asin,
	"\01atan",		(mathf)atan,
	"\02atan2",		(mathf)atan2,
	"\01cos",		(mathf)cos,
	"\01cosh",		(mathf)cosh,
	"\01exp",		(mathf)exp,
	"\01floor",		(mathf)floor,
	"\02fmod",		(mathf)fmod,
	"\02hypot",		(mathf)hypot,
	"\01int",		(mathf)floor,
	"\01log",		(mathf)log,
	"\02pow",		(mathf)pow,
	"\01sin",		(mathf)sin,
	"\01sinh",		(mathf)sinh,
	"\01sqrt",		(mathf)sqrt,
	"\01tan",		(mathf)tan,	
	"\01tanh",		(mathf)tanh,
	"",			(mathf)0 
};
