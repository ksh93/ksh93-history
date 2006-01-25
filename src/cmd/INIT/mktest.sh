########################################################################
#                                                                      #
#               This software is part of the ast package               #
#                  Copyright (c) 1994-2006 AT&T Corp.                  #
#                      and is licensed under the                       #
#                  Common Public License, Version 1.0                  #
#                            by AT&T Corp.                             #
#                                                                      #
#                A copy of the License is available at                 #
#            http://www.opensource.org/licenses/cpl1.0.txt             #
#         (with md5 checksum 059e8cd6165cb4c31e351f2b69388fd9)         #
#                                                                      #
#              Information and Software Systems Research               #
#                            AT&T Research                             #
#                           Florham Park NJ                            #
#                                                                      #
#                 Glenn Fowler <gsf@research.att.com>                  #
#                                                                      #
########################################################################
: mktest - generate regress or shell regression test scripts

command=mktest
stdin=8
stdout=9
STYLE=regress
PREFIX=test

eval "exec $stdout>&1"

case $(getopts '[-][123:xyz]' opt --xyz 2>/dev/null; echo 0$opt) in
0123)	ARGV0="-a $command"
	USAGE=$'
[-?
@(#)$Id: mktest (AT&T Labs Research) 2005-10-09 $
]
'$USAGE_LICENSE$'
[+NAME?mktest - generate a regression test scripts]
[+DESCRIPTION?\bmktest\b generates regression test scripts from test
    template commands in the \aunit\a.\brt\b file. The generated test
    script writes temporary output to '$PREFIX$'\aunit\a.tmp and compares
    it to the expected output in '$PREFIX$'\aunit\a.out. Run the test
    script with the \b--accept\b option to (re)generate the
    '$PREFIX$'\aunit\a.out.]
[s:style?The script style:]:[style:='$STYLE$']
    {
        [+regress?\bregress\b(1) command input.]
        [+shell?Standalone test shell script.]
    }

unit.rt [ unit [ arg ... ] ]

[+INPUT FILES?The regression test command file \aunit\a\b.rt\b is a
    \bksh\b(1) script that makes calls to the following functions:]
    {
        [+DATA \afile\a [ - | [ options ]] data]]?Create input data
            \afile\a that is empty (-) or contains \adata\a subject to
            \bprint\b(1) \aoptions\a or that is a copy of the DATA command
            standard input.]
        [+EXEC [ \aarg\a ... ]]?Run the command under test with
            optional arguments. If the standard input is not specified then
            the standard input of the previous EXEC is used. The standard
            input of the first EXEC in a TEST group is an empty regular
            file.]
        [+EXPORT \aname\a=\avalue\a ...?Export list for subsequent
            commands in the TEST group.]
        [+KEEP \apattern\a ...?File matych patterns of files to retain
            between TEST groups.]
        [+NOTE \acomment\a?\acomment\a is added to the current test
            script.]
        [+PROG \acommand\a [ \aarg\a ... ]]?\acommand\a is run with
            optional arguments.]
        [+TEST [ \anumber\a ]] [ \adescription\a ... ]]?Define a new
            test group with optional \anumber\a and \adescripion\a.]
        [+UNIT \acommand\a [ \aarg\a ... ]]?Define the command and
            optional default arguments to be tested. \bUNIT\b explicitly
            overrides the default command name derived from the test script
            file name.]
        [+WIDTH \awidth\a?Set the output format width to approximately
            \awidth\a.]
    }
[+SEE ALSO?\bregress\b(1), \bksh\b(1)]
'
	;;
*)	ARGV0=""
	USAGE='s: unit.rt [ arg ... ]'
	;;
esac

typeset ARG SCRIPT UNIT TEMP=${TMPDIR:-/tmp}/$command.$$.tmp
typeset IO INPUT OUTPUT ERROR KEEP
typeset SINGLE= WIDTH=80 quote='%${SINGLE}..${WIDTH}q'
typeset -A DATA RESET REMOVE
integer SCRIPT_UNIT=0 TEST=0 CODE=0 EXIT=0 ACCEPT=0 code

while	getopts $ARGV0 "$USAGE" OPT
do	case $OPT in
	s)	case $OPTARG in
		regress|shell)
			STYLE=$OPTARG
			;;
		*)	print -u2 -r -- $command: --style=$OPTARG: regress or shell expected
			exit 1
			;;
		esac
		;;
	*)	OPTIND=0
		getopts $ARGV0 "$USAGE" OPT '-?'
		exit 2
		;;
	esac
done
shift $OPTIND-1

if	[[ $1 == - ]]
then	shift
fi
if	(( ! $# ))
then
	print -u2 -r -- $command: test command script path expected
	exit 1
fi
SCRIPT=$1
shift
if	[[ ! -r $SCRIPT ]]
then	print -u2 -r -- $command: $SCRIPT: cannot read
	exit 1
fi
if	(( $# ))
then	set -A UNIT -- "$@"
else	ARG=${SCRIPT##*/}
	set -A UNIT -- "${ARG%.*}"
fi

function LINE
{
	if	[[ $STYLE == regress ]]
	then	print -u$stdout
	fi
}

function NOTE
{
	case $STYLE in
	regress)LINE
		print -u$stdout -r -- '#' "$@"
		;;
	shell)	print -u$stdout -r -f ": $QUOTE"$'\n' -- "$*"
		;;
	esac
}

function UNIT
{
	set -A UNIT -- "$@"
	case $STYLE in
	regress)LINE
		print -u$stdout -r -f $'UNIT'
		for ARG in "$@"
		do	print -u$stdout -r -f " $QUOTE" -- "$ARG"
		done
		print -u$stdout
		;;
	shell)	print -u$stdout -r -f $'set x'
		for ARG in "$@"
		do	print -u$stdout -r -f " $QUOTE" -- "$ARG"
		done
		print -u$stdout
		print -u$stdout shift
		;;
	esac
}

function TEST
{
	typeset i
	typeset -A REM
	if	(( ${#RESET[@]} ))
	then	unset ${!RESET[@]}
		case $STYLE in
		shell)	print -u$stdout -r -- unset ${!RESET[@]} ;;
		esac
		for i in ${!RESET[@]}
		do	unset RESET[$i]
		done
	fi
	if	(( ${#REMOVE[@]} ))
	then	rm -f -- "${!REMOVE[@]}"
		case $STYLE in
		shell)	print -u$stdout -r -f $'rm -f'
			for i in ${!REMOVE[@]}
			do	print -u$stdout -r -f " $QUOTE" "$i"
			done
			print -u$stdout
			;;
		esac
		for i in ${!REMOVE[@]}
		do	unset REMOVE[$i]
		done
	fi
	if	[[ $1 == +([0-9]) ]]
	then	TEST=${1##0}
		shift
	else	((TEST++))
	fi
	LINE
	case $STYLE in
	regress)print -u$stdout -r -f "TEST %02d $QUOTE"$'\n' -- $TEST "$*"
		;;
	shell)	print -u$stdout -r -f ": TEST %02d $QUOTE"$'\n' -- $TEST "$*"
		;;
	esac
	: > $TEMP.INPUT > $TEMP.in
	INPUT=
	OUTPUT=
	ERROR=
}

function EXEC
{
	typeset i n output=1 error=1 exitcode=1
	integer z
	while	:
	do	case $1 in
		++NOOUTPUT)	output= ;;
		++NOERROR)	error= ;;
		++NOEXIT)	exitcode= ;;
		++*)		print -u2 -r -- $command: $0: $1: unknown option; exit 1 ;;
		*)		break ;;
		esac
		shift
	done
	if	(( ! ${#UNIT[@]} ))
	then	print -u2 -r -- $command: $SCRIPT: UNIT statement or operand expected
		exit 1
	fi
	LINE
	case $STYLE in
	regress)print -u$stdout -r -f $'\tEXEC\t'
		for ARG in "$@"
		do	print -u$stdout -r -f " $QUOTE" -- "$ARG"
		done
		print -u$stdout
		[[ /dev/fd/0 -ef /dev/fd/$stdin ]] || cat > $TEMP.in
		IO=$(<$TEMP.in)
		(( z = $(wc -c < $TEMP.in) ))
		if	(( z && z == ${#IO} ))
		then	n=-n
		else	n=
		fi
		"${UNIT[@]}" "$@" < $TEMP.in > $TEMP.out 2> $TEMP.err
		code=$?
		if	[[ $IO != "$INPUT" ]]
		then	INPUT=$IO
			print -u$stdout -n -r -- $'\t\tINPUT' $n -
			[[ $IO ]] && print -u$stdout -r -f " $QUOTE" -- "$IO"
			print -u$stdout
		fi
		for i in ${!DATA[@]}
		do	IO=$(<$i)
			(( z = $(wc -c < $i) ))
			if	(( z && z == ${#IO} ))
			then	n=-n
			else	n=
			fi
			print -u$stdout -n -r -- $'\t\tINPUT' $n
			print -u$stdout -r -f " $QUOTE" -- "$i"
			[[ $IO ]] && print -u$stdout -r -f " $QUOTE" -- "$IO"
			print -u$stdout
			unset DATA[$i]
		done
		IO=$(<$TEMP.out)
		(( z = $(wc -c < $TEMP.out) ))
		if	(( z && z == ${#IO} ))
		then	n=-n
		else	n=
		fi
		if	[[ $IO != "$OUTPUT" ]]
		then	OUTPUT=$IO
			if	[[ $output ]]
			then	print -u$stdout -n -r -- $'\t\tOUTPUT' $n -
				[[ $IO ]] && print -u$stdout -r -f " $QUOTE" -- "$IO"
				print -u$stdout
			fi
		fi
		IO=$(<$TEMP.err)
		(( z = $(wc -c < $TEMP.err) ))
		if	(( z && z == ${#IO} ))
		then	n=-n
		else	n=
		fi
		if	[[ $IO != "$ERROR" ]]
		then	ERROR=$IO
			if	[[ $error ]]
			then	print -u$stdout -n -r -- $'\t\tERROR' $n -
				[[ $IO ]] && print -u$stdout -r -f " $QUOTE" -- "$IO"
				print -u$stdout
			fi
		fi
		case $output:$error in
		:)	OUTPUT=
			ERROR=
			print -u$stdout -r -- $'\t\tIGNORE OUTPUT ERROR'
			;;
		:1)	OUTPUT=
			print -u$stdout -r -- $'\t\tIGNORE OUTPUT'
			;;
		1:)	ERROR=
			print -u$stdout -r -- $'\t\tIGNORE ERROR'
			;;
		esac
		if	(( code != CODE ))
		then	(( CODE=code ))
			if	[[ $exitcode ]]
			then	print -u$stdout -r -f $'\t\tEXIT %d\n' $CODE
			fi
		fi
		;;
	shell)	print -u$stdout -r -f $'"$@"'
		for ARG in "$@"
		do	print -u$stdout -r -f " $QUOTE" -- "$ARG"
		done
		if	[[ ! $output ]]
		then	print -u$stdout -r -f " >/dev/null"
		fi
		if	[[ ! $error ]]
		then	if	[[ ! $output ]]
			then	print -u$stdout -r -f " 2>&1"
			else	print -u$stdout -r -f " 2>/dev/null"
			fi
		fi
		IO=$(cat)
		if	[[ $IO ]]
		then	print -u$stdout -r -- "<<'!TEST-INPUT!'"
			print -u$stdout -r -- "$IO"
			print -u$stdout -r -- !TEST-INPUT!
		else	print -u$stdout
		fi
		if	[[ $exitcode ]]
		then	print -u$stdout -r -- $'CODE=$?\ncase $CODE in\n0) ;;\n*) echo exit status $CODE ;;\nesac'
		fi
		;;
	esac
}

function DATA
{
	typeset f=$1 o=
	shift
	case $1 in
	'')	cat ;;
	-)	;;
	*)	print -r "$@" ;;
	esac > $f
	DATA[$f]=1
	if	[[ $f != $KEEP ]]
	then	REMOVE[$f]=1
	fi
	if	[[ $STYLE == shell ]]
	then	{
		print -r -f "cat > $QUOTE <<'!TEST-INPUT!'"$'\n' -- "$f"
		cat "$f"
		print -r -- !TEST-INPUT!
		} >&$stdout
	fi
}

function KEEP
{
	typeset p
	for p
	do	if	[[ $KEEP ]]
		then	KEEP=$KEEP'|'
		fi
		KEEP=$KEEP$p
	done
}

function EXPORT
{
	typeset x n v
	LINE
	case $STYLE in
	regress)	print -u$stdout -r -f $'EXPORT' ;;
	shell)		print -u$stdout -r -f $'export' ;;
	esac
	for x
	do	n=${x%%=*}
		v=${x#*=}
		export "$x"
		print -u$stdout -r -f " %s=$QUOTE" "$n" "$v"
		RESET[$n]=1
	done
	print -u$stdout
}

function PROG
{
	print -u2 -r -- $command: $0: not implemented
}

function WIDTH
{
	WIDTH=${1:-80}
	eval QUOTE='"'$quote'"'
}

trap 'CODE=$?; rm -f $TEMP.*; exit $CODE' 0 1 2 3 15

IFS=$IFS$'\n'

print -u$stdout -r "# : : generated from $SCRIPT by $command : : #"
case $STYLE in
shell)	cat <<!
ACCEPT=0
while	:
do	case \$1 in
	-a|--accept)
		ACCEPT=1
		;;
	--help|--man)
		cat 1>&2 <<!!
Usage: \\\$SHELL $PREFIX${UNIT[0]}.sh [ --accept ] [ unit ... ]

${UNIT[0]} regression test script.  Run this script to generate new
results in $PREFIX${UNIT[0]}.tmp and compare with expected results in
$PREFIX${UNIT[0]}.out.  The --accept option generates $PREFIX${UNIT[0]}.tmp
and moves it to $PREFIX${UNIT[0]}.out.
!!
		exit 2
		;;
	-*)	echo \$0: \$1: invalid option >&2
		exit 1
		;;
	*)	break
		;;
	esac
	shift
done
export COLUMNS=80
{
!
	;;
esac

export COLUMNS=80

case $STYLE in
shell)	SINGLE='#'
	eval QUOTE='"'$quote'"'
	. $SCRIPT < /dev/null | sed -e $'s,\\\\n,\n,g' -e $'s,\\\\t,\t,g' -e $'s,\\$\',\',g'
	;;
*)	eval QUOTE='"'$quote'"'
	: > $TEMP.INPUT > $TEMP.in
	eval "exec $stdin<$TEMP.INPUT"
	. $SCRIPT <&$stdin
	;;
esac

case $STYLE in
shell)	cat <<!
} > $PREFIX${UNIT[0]}.tmp 2>&1 < /dev/null
case \$ACCEPT in
0)	if	grep -q '$' $PREFIX${UNIT[0]}.tmp
	then	mv $PREFIX${UNIT[0]}.tmp $PREFIX${UNIT[0]}.junk
		sed 's/$//' < $PREFIX${UNIT[0]}.junk > $PREFIX${UNIT[0]}.tmp
		rm -f $PREFIX${UNIT[0]}.junk
	fi
	if	cmp -s $PREFIX${UNIT[0]}.tmp $PREFIX${UNIT[0]}.out
	then	echo ${UNIT[0]} tests PASSED
		rm -f $PREFIX${UNIT[0]}.tmp
	else	echo ${UNIT[0]} tests FAILED
		diff $PREFIX${UNIT[0]}.tmp $PREFIX${UNIT[0]}.out
	fi
	;;

*)	mv $PREFIX${UNIT[0]}.tmp $PREFIX${UNIT[0]}.out
	;;
esac
!
	;;
esac
