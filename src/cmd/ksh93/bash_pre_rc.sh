####################################################################
#                                                                  #
#             This software is part of the ast package             #
#                Copyright (c) 1982-2003 AT&T Corp.                #
#        and it may only be used by you under license from         #
#                       AT&T Corp. ("AT&T")                        #
#         A copy of the Source Code Agreement is available         #
#                at the AT&T Internet web site URL                 #
#                                                                  #
#       http://www.research.att.com/sw/license/ast-open.html       #
#                                                                  #
#    If you have copied or used this software without agreeing     #
#        to the terms of the license you are infringing on         #
#           the license and copyright and are violating            #
#               AT&T's intellectual property rights.               #
#                                                                  #
#            Information and Software Systems Research             #
#                        AT&T Labs Research                        #
#                         Florham Park NJ                          #
#                                                                  #
#                David Korn <dgk@research.att.com>                 #
#                                                                  #
####################################################################
# bash compability startup script
# Author: Karsten Fleischer <K.Fleischer@omnium.de>

alias declare=typeset
alias local=typeset

nameref FUNCNAME=.sh.fun
integer SHLVL
export SHLVL
SHLVL+=1

if	[[ ! $EUID ]]
then	EUID=$(id -u)
	readonly EUID
fi

if	[[ ! $UID ]]
then	UID=$(id -u)
	readonly UID
fi

readonly SHELLOPTS
if ! shopt -qo restricted; then
	IFS=:
	for i in $SHELLOPTS
	do
		[[ -n "$i" ]] && set -o $i
	done
	unset IFS
fi
function SHELLOPTS.get
{
	.sh.value=$(shopt -so)
	.sh.value=${.sh.value//+([[:space:]])on*([[:space:]])/:}
	.sh.value=${.sh.value%:}
}

set -A GROUPS $(id -G)
function GROUPS.set
{
	return 1
}
function GROUPS.unset
{
	unset -f GROUPS.set
	unset -f GROUPS.unset
}

typeset -A DIRSTACK
function DIRSTACK.get
{
	set -A .sh.value $(dirs)
}
function DIRSTACK.set
{
	integer index
	index=_push_max-.sh.subscript
	(( index == _push_max || index < _push_top )) && return
	_push_stack[index]=${.sh.value}
}
function DIRSTACK.unset
{
	unset -f DIRSTACK.get
	unset -f DIRSTACK.set
	unset -f DIRSTACK.unset
}

function GLOBIGNORE.set
{
	if [[ -z "${.sh.value}" ]]; then
		GLOBIGNORE.unset
	else
		if shopt -qo dotglob; then
			FIGNORE=@(|${.sh.value})
		else
			FIGNORE=${.sh.value}
		fi
	fi
}
function GLOBIGNORE.unset
{
	if shopt -qo dotglob; then
		FIGNORE=
	else
		unset FIGNORE
	fi
}

function PS1.set 
{
	typeset prefix remaining=${.sh.value} var= n= k=
	while	[[ $remaining ]]
	do	prefix=${remaining%%'\'*}
		remaining=${remaining#$prefix}
		var+="$prefix"
		case ${remaining:1:1} in
		t)	var+="\$(printf '%(%H:%M:%S)T')";;
		d)	var+="\$(printf '%(%a %b:%d)T')";;
		n)	var+=$'\n';;
		s)	var+=ksh;;
		w)	var+="\$(pwd)";;
		W)	var+="\$(basename \"\$(pwd)\")";;
		u)	var+=$USER;;
		h)	var+=$(hostname);;
		'#')	var+=!;;
		!)	var+=!;;
		'$')	if	(( $(id -u) == 0 ))
			then	var+='#'
			else	var+='$'
			fi;;
		'\')	var+='\\';;
		'['|']')	;;
		[0-7])	case ${remaining:1:3} in
			[0-7][0-7][0-7])
				k=4;;
			[0-7][0-7])
				k=3;;
			*)	k=2;;
			esac
			eval n="\$'"${remaining:0:k}"'"
			var+=$n
			remaining=${remaining:k}
			continue
			;;
		"")	;;
		*)	var+='\'${remaining:0:2};;
		esac
		remaining=${remaining:2}
	done
	.sh.value=$var
}
function logout
{
	if shopt -q login_shell; then
		exit
	else
		print ${BASH##*/}: $0: not login shell: use 'exit' >&2
		return 1
	fi
}
PS1="bash$ "

function source
{
	if ! shopt -qpo posix; then
		unset	OPATH
		typeset OPATH=$PATH
		typeset PATH=$PATH
		if shopt -q sourcepath; then
			PATH=$OPATH:.
		else
			PATH=.
		fi
	fi
	. "$@"
}
unalias .
alias .=source

alias enable=builtin

function help
{
	man=--man
	[[ $1 == -s ]] && man=--short && shift
	b=$(builtin)
	for i
	do
		for j in $b
		do
			[[ $i == $j ]] && $j $man
		done
	done
}
	
typeset BASH=$0
! shopt -qo posix && HISTFILE=~/.bash_history
HOSTNAME=$(hostname)
