/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*           Copyright (c) 1982-2006 AT&T Knowledge Ventures            *
*                      and is licensed under the                       *
*                  Common Public License, Version 1.0                  *
*                      by AT&T Knowledge Ventures                      *
*                                                                      *
*                A copy of the License is available at                 *
*            http://www.opensource.org/licenses/cpl1.0.txt             *
*         (with md5 checksum 059e8cd6165cb4c31e351f2b69388fd9)         *
*                                                                      *
*              Information and Software Systems Research               *
*                            AT&T Research                             *
*                           Florham Park NJ                            *
*                                                                      *
*                  David Korn <dgk@research.att.com>                   *
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * data for string evaluator library
 */

#include	"FEATURE/options"
#include	"FEATURE/math"
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

	0,	A_NOT,	0,	A_REG,	A_REG,	A_MOD,	A_AND,	A_LIT,
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

#ifdef __NeXT
extern double	hypot(double, double);
#endif

typedef Sfdouble_t (*mathf)(Sfdouble_t,...);
typedef int (*mathif)(Sfdouble_t);

#ifdef _ast_fltmax_double

#   define	fabsl	fabs
#   define	acosl	acos
#   define	asinl	asin
#   define	atanl	atan
#   define	atan2l	atan2
#   define	cosl	cos
#   define	coshl	cosh
#   define	expl	exp
#   define	finitel	finite
#   define	fmodl	fmod
#   define	hypotl	hypot
#   define	isinfl	isinf
#   define	isnanl	isnan
#   define	isnormall	isnormal
#   define	floorl	floor
#   define	logl	log
#   define	powl	pow
#   define	sinl	sin
#   define	sinhl	sinh
#   define	sqrtl	sqrt
#   define	tanl	tan
#   define	tanhl	tanh

#endif

#ifdef __STDARG__
#   define fundef(name)		static _ast_fltmax_t local_##name(_ast_fltmax_t d){ return(name(d));}
#   define protodef(name)	extern _ast_fltmax_t name(_ast_fltmax_t);
#   define macdef(name,x)	((Sfdouble_t)name((double)(x)))
#   define fundef2(name)	static _ast_fltmax_t local_##name(_ast_fltmax_t x, _ast_fltmax_t y){ return(name(x,y));}
#   define protodef2(name)	extern _ast_fltmax_t name(_ast_fltmax_t,_ast_fltmax_t);
#   define macdef2(name,x,y)	((Sfdouble_t)name((double)(x),(double)(y)))
#else
#   define fundef(name)		static _ast_fltmax_t local_##name(d) _ast_fltmax_t d;{ return(name(d));}
#   define protodef(name)	extern _ast_fltmax_t name();
#   define macdef(name,x)	((Sfdouble_t)name((double)(x)))
#   define fundef2(name)	static _ast_fltmax_t local_##name(x,y) _ast_fltmax_t x,y;{ return(name(x,y));}
#   define protodef2(name)	extern _ast_fltmax_t name();
#   define macdef2(name,x,y)	((Sfdouble_t)name((double)(x),(double)(y)))
#endif

#if 0 /* proto bug workaround */
{
#endif

#if defined(acosl) || !_lib_acosl
#   ifndef acosl
#       define acosl(x) macdef(acos,x)
#   endif
    fundef(acosl)
#   undef acosl
#   define acosl local_acosl
#else
#   if _npt_acosl
	protodef(acosl)
#   endif
#endif 

#if defined(asinl) || !_lib_asinl
#   ifndef asinl
#       define asinl(x) macdef(asin,x)
#   endif
    fundef(asinl)
#   undef asinl
#   define asinl local_asinl
#else
#   if _npt_asinl
	protodef(asinl)
#   endif
#endif 

#if defined(atanl) || !_lib_atanl
#   ifndef atanl
#       define atanl(x) macdef(atan,x)
#   endif
    fundef(atanl)
#   undef atanl
#   define atanl local_atanl
#else
#   if _npt_atanl
	protodef(atanl)
#   endif
#endif 

#if defined(atan2l) || !_lib_atan2l
#   ifndef atan2l
#       define atan2l(x,y) macdef2(atan2,x,y)
#   endif
    fundef2(atan2l)
#   undef atan2l
#   define atan2l local_atan2l
#else
#   if _npt_atan2l
	protodef2(atan2l)
#   endif
#endif 

#if defined(expl) || !_lib_expl
#   ifndef expl
#       define expl(x) macdef(exp,x)
#   endif
    fundef(expl)
#   undef expl
#   define expl local_expl
#else
#   if _npt_expl
	protodef(expl)
#   endif
#endif 

#if defined(cosl) || !_lib_cosl
#   ifndef cosl
#       define cosl(x) macdef(cos,x)
#   endif
    fundef(cosl)
#   undef cosl
#   define cosl local_cosl
#else
#   if _npt_cosl
	protodef(cosl)
#   endif
#endif 

#if defined(coshl) || !_lib_coshl
#   ifndef coshl
#       define coshl(x) macdef(cosh,x)
#   endif
    fundef(coshl)
#   undef coshl
#   define coshl local_coshl
#else
#   if _npt_coshl
	protodef(coshl)
#   endif
#endif 

#if defined(hypotl) || !_lib_hypotl
#   ifndef hypotl
#       define hypotl(x,y) macdef2(hypot,x,y)
#   endif
    fundef2(hypotl)
#   undef hypotl
#   define hypotl local_hypotl
#else
#   if _npt_hypotl
	protodef2(hypotl)
#   endif
#endif 

#if defined(finitel) || !_lib_finitel
#   ifndef finitel
#       define finitel(x) macdef(finite,x)
#   endif
#   ifdef _lib_finitel
	fundef(finitel)
#   else
#       ifdef __STDARG__
   	static int local_finitel(_ast_fltmax_t d)
#       else
   	static int local_finitel(d)
	_ast_fltmax_t d;
#       endif
	{
#     if _lib_isfinite
	    return(isfinite(d));
#     else
#       if _lib_isinfl
	    return(!isinfl(d) && !isnan(d));
#       else
#         if _lib_isinf
	    return(!isinf(d) && !isnan(d));
#         else
	    return(!isnan(d));
#         endif
#       endif
#     endif
	}
#   endif
#   undef finitel
#   define finitel local_finitel
#else
#   if _npt_finitel
	protodef(finitel)
#   endif
#endif 

#if defined(floorl) || !_lib_floorl
#   ifndef floorl
#       define floorl(x) macdef(floor,x)
#   endif
    fundef(floorl)
#   undef floorl
#   define floorl local_floorl
#else
#   if _npt_floorl
	protodef(floorl)
#   endif
#endif 

#if defined(fmodl) || !_lib_fmodl
#   ifndef fmodl
#       define fmodl(x,y) macdef2(fmod,x,y)
#   endif
    fundef2(fmodl)
#   undef fmodl
#   define fmodl local_fmodl
#else
#   if _npt_fmodl
	protodef2(fmodl)
#   endif
#endif 

#if defined(isinfl) || !_lib_isinfl
#   ifndef isinfl
#       define isinfl(x) macdef(isinf,x)
#   endif
#   ifdef _lib_isinf
	fundef(isinfl)
#   else
#       ifdef __STDARG__
   	static int local_isinfl(_ast_fltmax_t d)
#       else
   	static int local_isinfl(d)
	_ast_fltmax_t d;
#       endif
	{ return(!finitel(d) && !isnan(d)); }
#   endif
#   undef isinfl
#   define isinfl local_isinfl
#else
#   if _npt_isinfl
	protodef(isinfl)
#   endif
#endif 

#if defined(isnanl) || !_lib_isnanl
#   ifndef isnanl
#       define isnanl(x) macdef(isnan,x)
#   endif
    fundef(isnanl)
#   undef isnanl
#   define isnanl local_isnanl
#else
#   if _npt_isnanl
	protodef(isnanl)
#   endif
#endif 

#if defined(isnormall) || !_lib_isnormall
#   ifndef isnormall
#       define isnormall(x) macdef(isnormal,x)
#   endif
#   ifdef _lib_isnormal
	fundef(isnormall)
#   endif
#   undef isnormall
#   define isnormall local_isnormall
#else
#   if _npt_isnormall
	protodef(isnormall)
#   endif
#endif 

#if defined(logl) || !_lib_logl
#   ifndef logl
#       define logl(x) macdef(log,x)
#   endif
    fundef(logl)
#   undef logl
#   define logl local_logl
#else
#   if _npt_logl
	protodef(logl)
#   endif
#endif 

#if defined(sinl) || !_lib_sinl
#   ifndef sinl
#       define sinl(x) macdef(sin,x)
#   endif
    fundef(sinl)
#   undef sinl
#   define sinl local_sinl
#else
#   if _npt_sinl
	protodef(sinl)
#   endif
#endif 

#if defined(sinhl) || !_lib_sinhl
#   ifndef sinhl
#       define sinhl(x) macdef(sinh,x)
#   endif
    fundef(sinhl)
#   undef sinhl
#   define sinhl local_sinhl
#else
#   if _npt_sinhl
	protodef(sinhl)
#   endif
#endif 

#if defined(sqrtl) || !_lib_sqrtl
#   ifndef sqrtl
#       define sqrtl(x) macdef(sqrt,x)
#   endif
    fundef(sqrtl)
#   undef sqrtl
#   define sqrtl local_sqrtl
#else
#   if _npt_sqrtl
	protodef(sqrtl)
#   endif
#endif 

#if defined(tanl) || !_lib_tanl
#   ifndef tanl
#       define tanl(x) macdef(tan,x)
#   endif
    fundef(tanl)
#   undef tanl
#   define tanl local_tanl
#else
#   if _npt_tanl
	protodef(tanl)
#   endif
#endif 

#if defined(tanhl) || !_lib_tanhl
#   ifndef tanhl
#       define tanhl(x) macdef(tanh,x)
#   endif
    fundef(tanhl)
#   undef tanhl
#   define tanhl local_tanhl
#else
#   if _npt_tanhl
	protodef(tanhl)
#   endif
#endif 

#if defined(powl) || !_lib_powl
#   ifndef powl
#       define powl(x,y) macdef2(pow,x,y)
#   endif
    fundef2(powl)
#   undef powl
#   define powl local_powl
#else
#   if _npt_powl
	protodef2(powl)
#   endif
#endif 

#if defined(fabsl) || !_lib_fabsl
#   ifndef fabsl
#       define fabsl(x) macdef(fabs,x)
#   endif
    fundef(fabsl)
#   undef fabsl
#   define fabsl local_fabsl
#else
#   if _npt_fabsl
	protodef(fabsl)
#   endif
#endif 

#if 0 /* proto bug workaround */
}
#endif

/*
 * first byte is two-digit octal number.  Last digit is number of args
 * first digit is 0 if return value is double, 1 for integer
 */
const struct mathtab shtab_math[] =
{
	"\01abs",		(mathf)fabsl,
	"\01acos",		(mathf)acosl,
	"\01asin",		(mathf)asinl,
	"\01atan",		(mathf)atanl,
	"\02atan2",		(mathf)atan2l,
	"\01cos",		(mathf)cosl,
	"\01cosh",		(mathf)coshl,
	"\01exp",		(mathf)expl,
	"\011finite",		(mathf)finitel,
	"\01floor",		(mathf)floorl,
	"\02fmod",		(mathf)fmodl,
	"\02hypot",		(mathf)hypotl,
	"\01int",		(mathf)floorl,
	"\011isinf",		(mathf)isinfl,
	"\011isnan",		(mathf)isnanl,
#ifdef _lib_isnormal
	"\011isnormal",		(mathf)isnormall,
#endif
	"\01log",		(mathf)logl,
	"\02pow",		(mathf)powl,
	"\01sin",		(mathf)sinl,
	"\01sinh",		(mathf)sinhl,
	"\01sqrt",		(mathf)sqrtl,
	"\01tan",		(mathf)tanl,	
	"\01tanh",		(mathf)tanhl,
	"",			(mathf)0 
};
