/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1990-2004 AT&T Corp.                *
*        and it may only be used by you under license from         *
*                       AT&T Corp. ("AT&T")                        *
*         A copy of the Source Code Agreement is available         *
*                at the AT&T Internet web site URL                 *
*                                                                  *
*       http://www.research.att.com/sw/license/ast-open.html       *
*                                                                  *
*    If you have copied or used this software without agreeing     *
*        to the terms of the license you are infringing on         *
*           the license and copyright and are violating            *
*               AT&T's intellectual property rights.               *
*                                                                  *
*            Information and Software Systems Research             *
*                          AT&T Research                           *
*                         Florham Park NJ                          *
*                                                                  *
*               Glenn Fowler <gsf@research.att.com>                *
*                                                                  *
*******************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Research
 *
 * coshell readonly data
 */

#include "colib.h"

char	coident[] = "\
# @(#)$Id: libcoshell (AT&T Research) 2004-09-01 $\n\
{ { (eval 'function fun { trap \":\" 0; return 1; }; trap \"exit 0\" 0; fun; exit 1') && PATH= print -u%d ksh; } || { times && echo bsh >&%d; } || { echo osh >&%d; }; } >/dev/null 2>&1\n\
";

char	cobinit[] = "\
if	(eval 'f() echo') >/dev/null 2>&1\n\
then	eval 'ignore() {\n\
		case $- in\n\
		*x*)	set -\n\
			_coshell_silent=\n\
			;;\n\
		*)	_coshell_silent=1\n\
			;;\n\
		esac\n\
		_coshell_state=exp\n\
		_coshell_stop=\"<< -- StoP -- >>\"\n\
		_coshell_quote='\\\\\\''\n\
		set \"$@\" \"$_coshell_stop\"\n\
		while	:\n\
		do	case $1 in\n\
			$_coshell_stop)\n\
				shift\n\
				break\n\
				;;\n\
			*=*)	;;\n\
			*)	_coshell_state=arg ;;\n\
			esac\n\
			case $_coshell_state in\n\
			exp)	_coshell_arg=`echo $1 | sed \"s/\\\\([^=]*\\\\)=\\\\(.*\\\\)/\\\\1=$_coshell_quote\\\\2$_coshell_quote/\"`\n\
				set \"\" \"$@\" \"$_coshell_arg\"\n\
				shift\n\
				;;\n\
			arg)	set \"\" \"$@\" \"$_coshell_quote$1$_coshell_quote\"\n\
				shift\n\
				;;\n\
			esac\n\
			shift\n\
		done\n\
		case $_coshell_silent in\n\
		\"\")	set \"set -x;\" \"$@\" ;;\n\
		esac\n\
		eval \"$@\"\n\
		return 0\n\
	}'\n\
	eval 'silent() {\n\
		case $- in\n\
		*x*)	set -\n\
			_coshell_silent=\n\
			;;\n\
		*)	_coshell_silent=1\n\
			;;\n\
		esac\n\
		_coshell_state=exp\n\
		_coshell_stop=\"<< -- StoP -- >>\"\n\
		_coshell_quote='\\\\\\''\n\
		set \"$@\" \"$_coshell_stop\"\n\
		while	:\n\
		do	case $1 in\n\
			$_coshell_stop)\n\
				shift\n\
				break\n\
				;;\n\
			*=*)	;;\n\
			*)	_coshell_state=arg ;;\n\
			esac\n\
			case $_coshell_state in\n\
			exp)	_coshell_arg=`echo $1 | sed \"s/\\\\([^=]*\\\\)=\\\\(.*\\\\)/\\\\1=$_coshell_quote\\\\2$_coshell_quote/\"`\n\
				set \"\" \"$@\" \"$_coshell_arg\"\n\
				shift\n\
				;;\n\
			arg)	set \"\" \"$@\" \"$_coshell_quote$1$_coshell_quote\"\n\
				shift\n\
				;;\n\
			esac\n\
			shift\n\
		done\n\
		eval \"$@\"\n\
		_coshell_state=$?\n\
		case $_coshell_silent in\n\
		\"\")	set -x ;;\n\
		esac\n\
		return $_coshell_state\n\
	}'\n\
else	:\n\
fi\n\
";

char	cokinit[] = "\
set +o bgnice -o monitor\n\
exec %d>&%d\n\
(wait $$; exit 0) 2>/dev/null || alias wait=:\n\
alias ignore='ignore '\n\
function ignore\n\
{\n\
	integer argc=0\n\
	typeset argv state=exp\n\
	while	:\n\
	do	case $# in\n\
		0)	break ;;\n\
		esac\n\
		case $1 in\n\
		*=*)	;;\n\
		*)	state=arg ;;\n\
		esac\n\
		case $state in\n\
		exp)	argv[argc]=${1%%=*}=\"'${1#*=}'\" ;;\n\
		arg)	argv[argc]=\"'\"$1\"'\" ;;\n\
		esac\n\
		((argc=argc+1))\n\
		shift\n\
	done\n\
	eval \"${argv[@]}\"\n\
	return 0\n\
}\n\
alias silent='set +x X$- \"$@\";_coshell_flags_=$1;shift;silent '\n\
function silent\n\
{\n\
	case $_coshell_flags_ in\n\
	*x*)	trap '	_coshell_status_=$?\n\
		if ((_coshell_status_==0))\n\
		then	set -x\n\
		else	set -x;(set +x;exit $_coshell_status_)\n\
		fi' 0\n\
		;;\n\
	esac\n\
	\"$@\"\n\
}\n\
typeset -xf ignore silent\n\
";

char*	coexport[] =		/* default export var list		*/
{
	CO_ENV_EXPORT,		/* first				*/
	CO_ENV_ATTRIBUTES,
	CO_ENV_PROC,
	"FPATH",
	"VPATH",
	0			/* last					*/
};
