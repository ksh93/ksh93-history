########################################################################
#                                                                      #
#               This software is part of the ast package               #
#                  Copyright (c) 1994-2005 AT&T Corp.                  #
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
: regress - run regression tests in command.tst

command=regress
case $(getopts '[-][123:xyz]' opt --xyz 2>/dev/null; echo 0$opt) in
0123)	USAGE=$'
[-?
@(#)$Id: regress (AT&T Labs Research) 2004-07-17 $
]
'$USAGE_LICENSE$'
[+NAME?regress - run regression tests]
[+DESCRIPTION?\bregress\b runs the tests in \aunit\a, or \aunit\a\b.tst\b
	if \aunit\a does not exist. If \acommand\a is omitted then it is
	assumed to be the base name of \aunit\a. All testing is done in the
	temporary directory \aunit\a\b.tmp\b.]
[+?Default test output lists the \anumber\a and \adescription\a for each active
	\bTEST\b group and the \anumber\a:\aline\a for each individual
	\bEXEC\b test. Each test that fails results in a diagnostic that
	contains the word \bFAILED\b; no other diagnostics contain this word.]
[k:keep?Do not remove the temporary directory \aunit\a\b.tmp\b on exit.]
[q:quiet?Output information on \bFAILED\b tests only.]
[t:test?Run only tests matching \apattern\a. Tests are numbered and consist of
	at least two digits (0 filled if necessary.) Tests matching \b+(0) are
	always run.]:[pattern]
[v:verbose?List differences between actual (<) and expected (>) output, errors
	and exit codes. Also disable long output line truncation.]
[D:debug?Enable debug tracing.]

unit [ command [ arg ... ] ]

[+INPUT FILES?The regression test file \aunit\a\b.tst\b is a \bksh\b(1) script
	that is executed in an environment with the following functions
	defined:]{
	[+BODY { ... }?Defines the test body; used for complex tests.]
	[+CD \adirectory\a?Create and change to working directory for one test.]
	[+CLEANUP \astatus\a?Called at exit time to remove the temporary
		directory \aunit\a\b.tmp\b, list the tests totals via
		\bTALLY\b, and exit with status \astatus\a.]
	[+COMMAND \aarg\a ...?Runs the current command under test with
		\aarg\a ... appended to the default args.]
	[+COPY \afrom to\a?Copy file \afrom\a to \ato\a. \afrom\a may be
		a regular file or \bINPUT\b, \bOUTPUT\b or \bERROR\b.
		Post test comparisons are still done for \afrom\a.]
	[+DIAGNOSTICS [ \b1\b | \"\" ]]?No argument or an argument of \b1\b
		declares that diagnostics are to expected for the remainder
		of the current \bTEST\b; \"\" reverts to the default state
		that diagnostics are not expected.]
	[+DO \astatement\a?Defines additional statements to be executed
		for the current test. \astatement\a may be a { ... } group.]
	[+EMPTY \bINPUT|OUTPUT|ERROR|SAME?The corresponding file is expected
		to be empty.]
	[+ERROR [ \b-n\b ]] \afile\a | - \adata\a ...?The standard error is
		expected to match either the contents of \afile\a or the line
		\adata\a. \bERROR -n\b does not append a newline to \adata\a.]
	[+EXEC [ \aarg\a ... ]]?Runs the command under test with optional
		arguments. \bINPUT\b, \bOUTPUT\b, \bERROR\b, \bEXIT\b and
		\bSAME\b calls following this \bEXEC\b up until the next
		\bEXEC\b or the end of the script provide details for the
		expected results.]
	[+EXIT \astatus\a?The command exit status is expected to match the
		pattern \astatus\a.]
	[+EXPORT [-]] \aname\a=\avalue\a ...?Export environment variables for
		one test.]
	[+FATAL \amessage\a ...?\amessage\a is printed on the standard error
		and \bregress\b exits with status \b1\b.]
	[+IGNORE \afile\a ...?\afile\a is ignored for subsequent result
		comparisons. \afile\a may be \bOUTPUT\b or \bERROR\b.]
	[+INCLUDE \afile\a ...?One or more \afile\a operands are read
		via the \bksh\b(1) \b.\b(1) command. \bVIEW\b is used to
		locate the files.]
	[+INFO \adescription\a?\adescription\a is printed on the standard
		error.]
	[+INITIALIZE?Called by \bregress\b to initialize a each \bTEST\b
		group.]
	[+INPUT [ \b-n\b ]] \afile\a | - \adata\a ...?The standard input is
		set to either the contents of \afile\a or the line
		\adata\a. \bINPUT -n\b does not append a newline to \adata\a.]
	[+INTRO?Called by \bregress\b to introduce all \bTEST\b groups.]
	[+IO \bINPUT|OUTPUT|ERROR\b [ \b-n\b ]] \afile\a | - \adata\a ...?
		Internal support for the \bINPUT\b, \bOUTPUT\b and \bERROR\b
		functions.]
	[+KEEP \apattern\a ...?The temporary directory is cleared for each
		test. Files matching \apattern\a are retained between tests.]
	[+MOVE \afrom to\a?Rename file \afrom\a to \ato\a. \afrom\a may be
		a regular file or \bINPUT\b, \bOUTPUT\b or \bERROR\b.
		Post test comparisons are ignored for \afrom\a.]
	[+NOTE \acomment\a?\acomment\a is added to the current test trace
		output.]
	[+OUTPUT [ \b-n\b ]] \afile\a | - \adata\a ...?The standard output is
		expected to match either the contents of \afile\a or the line
		\adata\a. \bOUTPUT -n\b does not append a newline to \adata\a.]
	[+PROG \acommand\a [ \aarg\a ... ]]?\acommand\a is run with optional
		arguments.]
	[+REMOVE \afile\a ...?\afile\a ... are removed after the current test
		is done.]
	[+RUN?Called by \bregress\b to run the current test.]
	[+SAME \anew old\a?\anew\a is expected to be the same as \aold\a after
		the current test completes.]
	[+TALLY?Called by \bregress\b display the \bTEST\b results.]
	[+TEST \anumber\a [ \adescription\a ... ]]?Define a new test group
		labelled \anumber\a with option \adescripion\a.]
	[+TWD [ \adir\a ... ]]?Set the temporary test dir to \adir\a. The
		default is \aunit\a\b.tmp\b, where \aunit\a is the test
		input file sans directory and suffix. If \adir\a matches \b/*\b
		then it is the directory name; if \adir\a is non-null then the
		prefix \b${TMPDIR:-/tmp}\b is added; otherwise if \adir\a is
		omitted then \b${TMPDIR:-/tmp}/tst-\b\aunit\a-$$-$RANDOM.\b\aunit\a
		is used.]
	[+UNIT \acommand\a [ \aarg\a ... ]]?Define the command and optional
		default arguments to be tested. \bUNIT\b explicitly overrides
		the default command name derived from the test script
		file name.]
	[+VIEW \avar\a [ \afile\a ]]?\avar\a is set to the full pathname of
		\avar\a [ \afile\a ]] in the current \b$VPATH\b view if
		defined.]
}
[+SEE ALSO?\bnmake\b(1), \bksh\b(1)]
'
	;;
*)	USAGE='ko:[[no]name[=value]]t:[test]v unit [path [arg ...]]'
	;;
esac

function FATAL # message
{
	print -r -u2 "$command: $*"
	GROUP=FINI
	exit 1
}

function EMPTY
{
	typeset i
	typeset -n ARRAY=$1
	for i in ${!ARRAY[@]}
	do	unset ARRAY[$i]
	done
}

function INITIALIZE # void
{
	typeset i j
	cd "$TWD"
	case $KEEP in
	"")	RM *
		;;
	*)	for i in *
		do	case $i in
			!($KEEP))	j="$j $i" ;;
			esac
		done
		case $j in
		?*)	RM $j ;;
		esac
		;;
	esac
	: >INPUT >OUTPUT.ex >ERROR.ex
	BODY=""
	COPY=""
	DIAGNOSTICS=""
	DONE=""
	ERROR=""
	EXIT=0
	IGNORE=""
	INIT=""
	INPUT=""
	MOVE=""
	OUTPUT=""
	EMPTY SAME
}

function INTRO
{
	typeset base command

	case $quiet in
	"")	base=${REGRESS##*/}
		base=${base%.tst}
		command=${COMMAND##*/}
		command=${command%' '*}
		if [[ $command == $base ]]
		then	TITLE=$COMMAND
		else	TITLE="$COMMAND, $base"
		fi
		print -u2 "TEST	$TITLE"
		;;
	esac
}

function TALLY
{
	typeset msg
	case $GROUP in
	INIT)	;;
	*)	msg="TEST	$TITLE, $TESTS test"
		case $TESTS in
		1)	;;
		*)	msg=${msg}s ;;
		esac
		msg="$msg, $ERRORS error"
		case $ERRORS in
		1)	;;
		*)	msg=${msg}s ;;
		esac
		print -u2 "$msg"
		GROUP=INIT
		TESTS=0
		ERRORS=0
		;;
	esac
}

function CLEANUP # status
{
	if	[[ ! $dump && $GROUP!=INIT ]]
	then	cd $SOURCE
		RM "$TWD"
	fi
	TALLY
	exit $1
}

function RUN # [ op ]
{
	typeset failed i j s
	typeset $truncate SHOW
	case $GROUP in
	INIT)	RM "$TWD"
		mkdir "$TWD" || FATAL "$TWD": cannot create directory
		cd "$TWD"
		TWD=$PWD
		: > rmu
		if	rm -u rmu >/dev/null 2>&1
		then	rmu=-u
		else	rm rmu
		fi
		if	[[ $UNIT ]]
		then	set -- "${ARGV[@]}"
			case $1 in
			""|[-+]*)
				UNIT $UNIT "${ARGV[@]}"
				;;
			*)	UNIT "${ARGV[@]}"
				;;
			esac
		fi
		INTRO
		;;
	FINI)	;;
	$select)if	[[ $ITEM == $FLUSHED ]]
		then	return
		fi
		FLUSHED=$ITEM
		((COUNT++))
		if	(( $ITEM <= $LASTITEM ))
		then	LABEL=$TEST#$COUNT
		else	LASTITEM=$ITEM
			LABEL=$TEST:$ITEM
		fi
		case $quiet in
		"")	print -nu2 "$LABEL" ;;
		esac
		file=""
		exec >/dev/null
		#DEBUG#PS4='+$LINENO+ '; set -x
		for i in $INPUT
		do	case " $OUTPUT " in
			*" $i "*)
				if	[[ -f $i.sav ]]
				then	cp $i.sav $i
					compare="$compare $i"
				elif	[[ -f $i ]]
				then	cp $i $i.sav
					compare="$compare $i"
				fi
				;;
			esac
		done
		for i in $OUTPUT
		do	case " $compare " in
			*" $i "*)
				;;
			*)	compare="$compare $i"
				;;
			esac
		done
		for i in $INIT
		do	$i $TEST INIT
		done
		case $BODY in
		"")	COMMAND "${ARGS[@]}" <$TWD/INPUT >$TWD/OUTPUT 2>$TWD/ERROR
			failed=""
			ignore=""
			set -- $COPY
			COPY=""
			while	:
			do	case $# in
				0|1)	break ;;
				*)	cp $1 $2 ;;
				esac
				shift 2
			done
			set -- $MOVE
			MOVE=""
			while	:
			do	case $# in
				0|1)	break ;;
				*)	mv $1 $2; ignore="$ignore $1" ;;
				esac
				shift 2
			done
			for i in $compare $TWD/OUTPUT $TWD/ERROR
			do	case " $IGNORE $ignore " in
				*" $i "*)	continue ;;
				esac
				ignore="$ignore $i"
				j=${SAME[${i##*/}]}
				if	[[ ! $j ]]
				then	if	[[ $i == /* ]]
					then	k=$i
					else	k=$TWD/$i
					fi
					for s in ex sav err
					do	[[ -f $k.$s ]] && break
					done
					j=$k.$s
				fi
				case $DIAGNOSTICS:$i in
				?*:*/ERROR)
					case $STATUS in
					0)	[[ ! -s $TWD/ERROR ]] && failed=$failed${failed:+,}DIAGNOSTICS ;;
					esac
					continue
					;;
				*)	cmp -s $i $j && continue
					;;
				esac
				failed=$failed${failed:+,}${i#$TWD/}
				case $verbose in
				?*)	print -u2 "	=== diff ${i#$TWD/} <actual >expected ==="
					diff $i $j >&2
					;;
				esac
			done
			case $failed in
			"")	case $STATUS in
				$EXIT)	;;
				*)	failed="exit code $EXIT expected -- got $STATUS" ;;
				esac
				;;
			esac
			case $failed in
			"")	SHOW=$NOTE
				case $quiet in
				"")	print -r -u2 "	$SHOW" ;;
				esac
				;;
			?*)	((ERRORS++))
				case $quiet in
				?*)	print -nu2 "$LABEL" ;;
				esac
				SHOW="FAILED [ $failed ] $NOTE"
				print -r -u2 "	$SHOW"
				case $dump in
				?*)	GROUP=FINI; exit ;;
				esac
				;;
			esac
			;;
		*)	SHOW=$NOTE
			case $quiet in
			"")	print -r -u2 "	$SHOW" ;;
			esac
			for i in $BODY
			do	$i $TEST BODY
			done
			;;
		esac
		for i in $DONE
		do	$i $TEST DONE $STATUS
		done
		compare=""
		#DEBUG#set +x
		;;
	esac
	if	[[ $COMMAND_ORIG ]]
	then	COMMAND=$COMMAND_ORIG
		COMMAND_ORIG=
		ARGS=(${ARGS_ORIG[@]})
	fi
}

function UNIT # cmd arg ...
{
	typeset cmd=$1
	case $cmd in
	-)	shift
		#BUG# ARGV=("${ARGV[@]}" "$@")
		set -- "${ARGV[@]}" "$@"
		ARGV=("$@")
		return
		;;
	esac
	if	[[ $UNIT ]]
	then	set -- "${ARGV[@]}"
		case $1 in
		"")	set -- "$cmd" ;;
		[-+]*)	set -- "$cmd" "${ARGV[@]}" ;;
		*)	set -- "${ARGV[@]}" ;;
		esac
		UNIT=
	fi
	COMMAND=$1
	shift
	typeset cmd=$(PATH=$SOURCE:$PATH:/usr/5bin:/bin:/usr/bin whence $COMMAND)
	if	[[ ! $cmd ]]
	then	FATAL $COMMAND: not found
	elif	[[ ! $cmd ]]
	then	FATAL $cmd: not found
	fi
	COMMAND=$cmd
	case $# in
	0)	;;
	*)	COMMAND="$COMMAND $*" ;;
	esac
}

function TWD # [ dir ]
{
	case $1 in
	'')	TWD=${TMPDIR:-/tmp}/tst-${TWD%.*}-$$-$RANDOM ;;
	/*)	TWD=$1 ;;
	*)	TWD=${TMPDIR:-/tmp}/$1 ;;
	esac
}

function TEST # number description arg ...
{
	RUN
	COUNT=0
	LASTITEM=0
	case $1 in
	-)		((LAST++)); TEST=$LAST ;;
	+([0123456789]))	LAST=$1 TEST=$1 ;;
	*)		LAST=0${1/[!0123456789]/} TEST=$1 ;;
	esac
	NOTE=
	case $TEST in
	$select)
		case $quiet in
		"")	print -r -u2 "$TEST	$2" ;;
		esac
		;;
	esac
	unset ARGS
	unset EXPORT
	EXPORTS=0
	file=""
	case $TEST in
	${GROUP}*)
		;;
	*)	GROUP=${TEST%%+([abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ])}
		case $GROUP in
		$select)INITIALIZE ;;
		esac
		;;
	esac
	((SUBTESTS=0))
	[[ $TEST == $select ]]
}

function EXEC # arg ...
{
	case $GROUP in
	!($select))	return ;;
	esac
	if	((SUBTESTS++))
	then	RUN
	fi
	case $# in
	0)	set -- "${ARGS[@]}" ;;
	esac
	ITEM=$LINE
	NOTE="$(print -r -f '%q ' -- $COMMAND_ORIG "$@")"
	ARGS=("$@")
}

function CD
{
	RUN
	case $GROUP in
	$select)	mkdir -p "$@" && cd "$@" || FATAL cannot initialize working directory "$@" ;;
	esac
}

function EXPORT
{
	typeset x
	RUN
	case $GROUP in
	!($select))	return ;;
	esac
	for x
	do	EXPORT[EXPORTS++]=$x
	done
}

function FLUSH
{
	case $GROUP in
	!($select))	return ;;
	esac
	if	((SUBTESTS++))
	then	RUN
	fi
}

function PROG # cmd arg ...
{
	typeset command args
	case $GROUP in
	!($select))	return ;;
	esac
	ITEM=$LINE
	NOTE="$(print -r -f '%q ' -- "$@")"
	COMMAND_ORIG=$COMMAND
	COMMAND=$1
	shift
	ARGS_ORIG=(${ARGS[@]})
	ARGS=("$@")
}

function NOTE # description
{
	NOTE=$*
}

function IO # INPUT|OUTPUT|ERROR [-f|-n] file|- data ...
{
	typeset op i v f file x
	case $GROUP in
	!($select))	return ;;
	esac
	op=$1
	shift
	file=$TWD/$op
	case $1 in
	-x)	x=1; shift ;;
	esac
	case $1 in
	-[fn])	f=$1; shift ;;
	esac
	case $# in
	0)	;;
	*)	case $1 in
		-)	;;
		*)	file=$1
			eval i='$'$op
			case " $i " in
			*" $file "*)
				;;
			*)	eval $op='"$'$op' $file"'
				;;
			esac
			;;
		esac
		shift
		;;
	esac
	case " $IGNORE " in
	*" $file "*)
		for i in $IGNORE
		do	case $i in
			$file)	;;
			*)	v="$v $i" ;;
			esac
		done
		IGNORE=$v
		;;
	esac
	case $op in
	OUTPUT|ERROR)
		file=$file.ex
		if [[ $file != /* ]]
		then	file=$TWD/$file
		fi
		;;
	esac
	#unset SAME[$op]
	SAME[$op]=
	RM $TWD/$file.sav
	if	[[ $file == */* ]]
	then	mkdir -p ${file%/*}
	fi
	if	[[ $file != */ ]]
	then	case $#:$f in
		0:)	: > $file ;;
		*:-f)	printf -- "$@" > $file ;;
		*)	print $f -r -- "$@" > $file ;;
		esac
		if [[ $x ]]
		then	chmod +x $file
		fi
	fi
}

function INPUT # file|- data ...
{
	IO $0 "$@"
}

function COPY # from to
{
	case $GROUP in
	!($select))	return ;;
	esac
	COPY="$COPY $@"
}

function MOVE # from to
{
	typeset f
	case $GROUP in
	!($select))	return ;;
	esac
	for f
	do	case $f in
		INPUT|OUTPUT|ERROR)
			f=$TWD/$f
			;;
		/*)	;;
		*)	f=$PWD/$f
			;;
		esac
		MOVE="$MOVE $f"
	done
}

function SAME # new old
{
	typeset i file v
	case $GROUP in
	!($select))	return ;;
	esac
	case $# in
	2)	case $1 in
		INPUT)	cat $2 > $1; return ;;
		esac
		SAME[$1]=$2
		file=$1
		compare="$compare $1"
		;;
	3)	SAME[$2]=$3
		file=$2
		eval i='$'$1
		case " $i " in
		*" $2 "*)
			;;
		*)	eval $1='"$'$1' $2"'
			;;
		esac
		compare="$compare $2"
		;;
	esac
	case " $IGNORE " in
	*" $file "*)
		for i in $IGNORE
		do	case $i in
			$file)	;;
			*)	v="$v $i" ;;
			esac
		done
		IGNORE=$v
		;;
	esac
}

function OUTPUT # file|- data ...
{
	IO $0 "$@"
}

function ERROR # file|- data ...
{
	IO $0 "$@"
}

function RM # rm(1) args
{
	if	[[ ! $rmu ]]
	then	chmod -R u+rwx "$@" >/dev/null 2>&1
	fi
	rm $rmu $rmflags "$@"
}

function REMOVE # file ...
{
	typeset i
	for i
	do	RM $i $i.sav
	done
}

function IGNORE # file ...
{
	typeset i
	for i
	do	case $i in
		INPUT|OUTPUT|ERROR)
			i=$TWD/$i
			;;
		esac
		case " $IGNORE " in
		*" $i "*)
			;;
		*)	IGNORE="$IGNORE $i"
			;;
		esac
	done
}

function KEEP # pattern ...
{
	typeset i
	for i
	do	case $KEEP in
		"")	KEEP="$i" ;;
		*)	KEEP="$KEEP|$i" ;;
		esac
	done
}

function DIAGNOSTICS # [ 1 | "" ]
{
	DIAGNOSTICS=${1:-1}
	EXIT='*'
}

function EXIT # status
{
	EXIT=$1
}

function INFO # info description
{
	typeset -R15 info=$1
	case $1 in
	"")	info=no ;;
	esac
	shift
	case $quiet in
	"")	print -r -u2 "$info " "$@" ;;
	esac
}

function COMMAND # arg ...
{
	((TESTS++))
	case $dump in
	?*)	(
		PS4=''
		set -x
		print -r -- "${EXPORT[@]}" $COMMAND "$@"
		) 2>&1 >/dev/null |
		sed 's,^print -r -- ,,' >$TWD/COMMAND
		chmod +x $TWD/COMMAND
		;; 
	esac
	eval "${EXPORT[@]}" '$'COMMAND '"$@"'
	STATUS=$?
	return $STATUS
}

function SET # [no]name[=value]
{
	typeset i
	for i
	do	case $i in
		no?*)	eval ${i#no}='""' ;;
		*=0)	eval ${i%0}='""' ;;
		*=*)	eval $i ;;
		*)	eval $i=1 ;;
		esac
	done
}

function VIEW # var [ file ]
{
	nameref var=$1
	typeset i bwd file pwd view root offset
	if	[[ $var ]]
	then	return 0
	fi
	case $# in
	1)	file=$1 ;;
	*)	file=$2 ;;
	esac
	pwd=${TWD%/*}
	bwd=${PMP%/*}
	if	[[ -r $file ]]
	then	if	[[ ! -d $file ]]
		then	var=$PWD/$file
			return 0
		fi
		for i in $file/*
		do	if	[[ -r $i ]]
			then	var=$PWD/$file
				return 0
			fi
			break
		done
	fi
	for view in ${VIEWS[@]}
	do	case $view in
		/*)	;;
		*)	view=$pwd/$view ;;
		esac
		case $offset in
		'')	case $pwd in
			$view/*)	offset=${pwd#$view} ;;
			*)		offset=${bwd#$view} ;;
			esac
			;;
		esac
		if	[[ -r $view$offset/$file ]]
		then	if	[[ ! -d $view$offset/$file ]]
			then	var=$view$offset/$file
				return 0
			fi
			for i in $view$offset/$file/*
			do	if	[[ -f $i ]]
				then	var=$view$offset/$file
					return 0
				fi
				break
			done
		fi
	done
	var=
	return 1
}

function INCLUDE # file ...
{
	typeset f v
	for f
	do	if	VIEW v $f || [[ $PREFIX && $f != /* ]] && VIEW v $PREFIX$f
		then	. $v
		else	FATAL $f: not found
		fi
	done
}

# main

integer ERRORS=0 EXPORTS=0 TESTS=0 SUBTESTS=0 LINE=0 ITEM=0 LASTITEM=0 COUNT
typeset ARGS COMMAND COPY DIAGNOSTICS ERROR EXEC FLUSHED=0 GROUP=INIT
typeset IGNORE INPUT KEEP OUTPUT TEST SOURCE MOVE NOTE
typeset ARGS_ORIG COMMAND_ORIG TITLE UNIT ARGV PREFIX OFFSET
typeset dump file quiet rmflags='-rf --' rmu select trace verbose truncate=-L70
typeset -A EXPORT SAME VIEWS
typeset -Z LAST=00

unset FIGNORE

while	getopts -a $command "$USAGE" OPT
do	case $OPT in
	k)	SET dump=1
		;;
	q)	SET quiet=1
		;;
	t)	case $select in
		"")	select="${OPTARG//,/\|}" ;;
		*)	select="$select|${OPTARG//,/\|}" ;;
		esac
		;;
	v)	SET verbose=1
		truncate=
		;;
	D)	SET trace=1
		;;
	*)	GROUP=FINI
		exit 2
		;;
	esac
done
shift $OPTIND-1
case $# in
0)	FATAL test unit name omitted ;;
esac
export COLUMNS=80
SOURCE=$PWD
PATH=$SOURCE:${PATH#:}
UNIT=$1
shift
if	[[ -f $UNIT && ! -x $UNIT ]]
then	REGRESS=$UNIT
else	REGRESS=${UNIT%.tst}
	REGRESS=$REGRESS.tst
	[[ -f $REGRESS ]] || FATAL $REGRESS: regression tests not found
fi
UNIT=${UNIT##*/}
UNIT=${UNIT%.tst}
if	[[ $VPATH ]]
then	set -A VIEWS ${VPATH//:/' '}
	OFFSET=${SOURCE#${VIEWS[0]}}
	if	[[ $OFFSET ]]
	then	OFFSET=${OFFSET#/}/
	fi
fi
if	[[ $REGRESS == */* ]]
then	PREFIX=${REGRESS%/*}
	if	[[ ${#VIEWS[@]} ]]
	then	for i in ${VIEWS[@]}
		do	PREFIX=${PREFIX#$i/}
		done
	fi
	PREFIX=${PREFIX#$OFFSET}
	if	[[ $PREFIX ]]
	then	PREFIX=$PREFIX/
	fi
fi
TWD=$PWD/$UNIT.tmp
PMP=$(/bin/pwd)/$UNIT.tmp
ARGV=("$@")
trap 'RUN; CLEANUP 0' EXIT
trap 'CLEANUP $?' HUP INT PIPE TERM
case $select in
"")	select="[0123456789]*" ;;
esac
select="@($select|+(0))"
case $trace in
?*)	PS4='+$LINENO+ '
	set -x
	;;
esac

# some last minute shenanigans

alias BODY='BODY=BODY; function BODY'
alias DO='(( $ITEM != $FLUSHED )) && RUN DO; [[ $GROUP == $select ]] &&'
alias DONE='DONE=DONE; function DONE'
alias EXEC='LINE=$LINENO; EXEC'
alias INIT='INIT=INIT; function INIT'
alias PROG='LINE=$LINENO; FLUSH; PROG'

# do the tests

. $REGRESS
