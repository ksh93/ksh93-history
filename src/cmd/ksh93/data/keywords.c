/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1982-2004 AT&T Corp.                  *
*                      and is licensed under the                       *
*          Common Public License, Version 1.0 (the "License")          *
*                        by AT&T Corp. ("AT&T")                        *
*      Any use, downloading, reproduction or distribution of this      *
*      software constitutes acceptance of the License.  A copy of      *
*                     the License is available at                      *
*                                                                      *
*         http://www.research.att.com/sw/license/cpl-1.0.html          *
*         (with md5 checksum 8a5e0081c856944e76c69a1cf29c2e8b)         *
*                                                                      *
*              Information and Software Systems Research               *
*                            AT&T Research                             *
*                           Florham Park NJ                            *
*                                                                      *
*                  David Korn <dgk@research.att.com>                   *
*                                                                      *
***********************************************************************/
#pragma prototyped
#include	"shtable.h"
#include	<shell.h>
#include	"shlex.h"
#include	"FEATURE/options"

/*
 * table of reserved words in shell language
 * This list must be in in ascii sorted order
 */

const Shtable_t shtab_reserved[] =
{
		"!",		NOTSYM,
		"[[",		BTESTSYM,
		"case",		CASESYM,
		"do",		DOSYM,
		"done",		DONESYM,
		"elif",		ELIFSYM,
		"else",		ELSESYM,
		"esac",		ESACSYM,
		"fi",		FISYM,
		"for",		FORSYM,
		"function",	FUNCTSYM,
		"if",		IFSYM,
		"in",		INSYM,
#if SHOPT_NAMESPACE
		"namespace",	NSPACESYM,
#endif /* SHOPT_NAMESPACE */
		"select",	SELECTSYM,
		"then",		THENSYM,
		"time",		TIMESYM,
		"until",	UNTILSYM,
		"while",	WHILESYM,
		"{",		LBRACE,
		"}",		RBRACE,
		"",		0,
};

const char	e_unexpected[]	= "unexpected";
const char	e_unmatched[]	= "unmatched";
const char	e_endoffile[]	= "end of file";
const char	e_newline[]	= "newline";

