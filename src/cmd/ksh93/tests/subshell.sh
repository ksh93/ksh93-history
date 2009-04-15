########################################################################
#                                                                      #
#               This software is part of the ast package               #
#          Copyright (c) 1982-2009 AT&T Intellectual Property          #
#                      and is licensed under the                       #
#                  Common Public License, Version 1.0                  #
#                    by AT&T Intellectual Property                     #
#                                                                      #
#                A copy of the License is available at                 #
#            http://www.opensource.org/licenses/cpl1.0.txt             #
#         (with md5 checksum 059e8cd6165cb4c31e351f2b69388fd9)         #
#                                                                      #
#              Information and Software Systems Research               #
#                            AT&T Research                             #
#                           Florham Park NJ                            #
#                                                                      #
#                  David Korn <dgk@research.att.com>                   #
#                                                                      #
########################################################################
function err_exit
{
	print -u$Error_fd -n "\t"
	print -u$Error_fd -r ${Command}[$1]: "${@:2}"
	(( Errors+=1 ))
}
alias err_exit='err_exit $LINENO'

Command=${0##*/}
integer Errors=0 Error_fd=2

tmp=$(mktemp -dt) || { err_exit mktemp -dt failed; exit 1; }
trap "cd /; rm -rf $tmp" EXIT

bincat=$(PATH=$(getconf PATH) whence -p cat)

z=()
z.foo=( [one]=hello [two]=(x=3 y=4) [three]=hi)
z.bar[0]=hello
z.bar[2]=world
z.bar[1]=(x=4 y=5)
val='(
	typeset -a bar=(
		[0]=hello
		[2]=world
		[1]=(
			x=4
			y=5
		)
	)
	typeset -A foo=(
		[one]=hello
		[three]=hi
		[two]=(
			x=3
			y=4
		)
	)
)'
[[ $z == "$val" ]] || err_exit 'compound variable with mixed arrays not working'
z.bar[1]=yesyes
[[ ${z.bar[1]} == yesyes ]] || err_exit 'reassign of index array compound variable fails'
z.bar[1]=(x=12 y=5)
[[ ${z.bar[1]} == $'(\n\tx=12\n\ty=5\n)' ]] || err_exit 'reassign array simple to compound variable fails'
eval val="$z"
(
	z.foo[three]=good
	[[ ${z.foo[three]} == good ]] || err_exit 'associative array assignment in subshell not working'
)
[[ $z == "$val" ]] || err_exit 'compound variable changes after associative array assignment'
eval val="$z"
(
false
	z.foo[two]=ok
	[[ ${z.foo[two]} == ok ]] || err_exit 'associative array assignment to compound variable in subshell not working'
	z.bar[1]=yes
	[[ ${z.bar[1]} == yes ]] || err_exit 'index array assignment to compound variable in subshell not working'
)
[[ $z == "$val" ]] || err_exit 'compound variable changes after associative array assignment'

x=(
	foo=( qqq=abc rrr=def)
	bar=( zzz=no rst=fed)
)
eval val="$x"
(
	unset x.foo
	[[ ${x.foo.qqq} ]] && err_exit 'x.foo.qqq should be unset'
	x.foo=good
	[[ ${x.foo} == good ]] || err_exit 'x.foo should be good'
)
[[ $x == "$val" ]] || err_exit 'compound variable changes after unset leaves'
unset l
(
	l=( a=1 b="BE" )
)
[[ ${l+foo} != foo ]] || err_exit 'l should be unset'

Error_fd=9
eval "exec $Error_fd>&2 2>/dev/null"

TEST_notfound=notfound
while	whence $TEST_notfound >/dev/null 2>&1
do	TEST_notfound=notfound-$RANDOM
done

integer BS=1024 nb=64 ss=60 bs no
for bs in $BS 1
do	$SHELL -c '
		{
			sleep '$ss'
			kill -KILL $$
		} &
		set -- $(printf %.'$(($BS*$nb))'c x | dd bs='$bs')
		print ${#1}
		kill $!
	' > $tmp/sub 2>/dev/null
	no=$(<$tmp/sub)
	(( no == (BS * nb) )) || err_exit "shell hangs on command substitution output size >= $BS*$nb with write size $bs -- expected $((BS*nb)), got ${no:-0}"
done
# this time with redirection on the trailing command
for bs in $BS 1
do	$SHELL -c '
		{
			sleep 2
			sleep '$ss'
			kill -KILL $$
		} &
		set -- $(printf %.'$(($BS*$nb))'c x | dd bs='$bs' 2>/dev/null)
		print ${#1}
		kill $!
	' > $tmp/sub 2>/dev/null
	no=$(<$tmp/sub)
	(( no == (BS * nb) )) || err_exit "shell hangs on command substitution output size >= $BS*$nb with write size $bs and trailing redirection -- expected $((BS*nb)), got ${no:-0}"
done

# exercise command substitutuion trailing newline logic w.r.t. pipe vs. tmp file io

set -- \
	'post-line print'								\
	'$TEST_unset; ($TEST_fork; print 1); print'					\
	1										\
	'pre-line print'								\
	'$TEST_unset; ($TEST_fork; print); print 1'					\
	$'\n1'										\
	'multiple pre-line print'							\
	'$TEST_unset; ($TEST_fork; print); print; ($TEST_fork; print 1); print'		\
	$'\n\n1'									\
	'multiple post-line print'							\
	'$TEST_unset; ($TEST_fork; print 1); print; ($TEST_fork; print); print'		\
	1										\
	'intermediate print'								\
	'$TEST_unset; ($TEST_fork; print 1); print; ($TEST_fork; print 2); print'	\
	$'1\n\n2'									\
	'simple variable'								\
	'$TEST_unset; ($TEST_fork; l=2; print "$l"); print $l'				\
	2										\
	'compound variable'								\
	'$TEST_unset; ($TEST_fork; l=(a=2 b="BE"); print "$l"); print $l'		\
	$'(\n\ta=2\n\tb=BE\n)'								\

export TEST_fork TEST_unset

while	(( $# >= 3 ))
do	txt=$1
	cmd=$2
	exp=$3
	shift 3
	for TEST_unset in '' 'unset var'
	do	for TEST_fork in '' 'ulimit -c 0'
		do	for TEST_shell in "eval" "$SHELL -c"
			do	if	! got=$($TEST_shell "$cmd")
				then	err_exit "${TEST_shell/*-c/\$SHELL -c} ${TEST_unset:+unset }${TEST_fork:+fork }$txt print failed"
				elif	[[ "$got" != "$exp" ]]
				then	EXP=$(printf %q "$exp")
					GOT=$(printf %q "$got")
					err_exit "${TEST_shell/*-c/\$SHELL -c} ${TEST_unset:+unset }${TEST_fork:+fork }$txt command substitution failed -- expected $EXP, got $GOT"
				fi
			done
		done
	done
done

r=$( ($SHELL -c '
	{
		sleep 32
		kill -KILL $$
	} &
	for v in $(set | sed "s/=.*//")
	do	command unset $v
	done
	typeset -Z5 I
	for ((I = 0; I < 1024; I++))
	do	eval A$I=1234567890
	done
	a=$(set 2>&1)
	print ok
	kill -KILL $!
') 2>/dev/null)
[[ $r == ok ]] || err_exit "large subshell command substitution hangs"

for TEST_command in '' $TEST_notfound
do	for TEST_exec in '' 'exec'
	do	for TEST_fork in '' 'ulimit -c 0;'
		do	for TEST_redirect in '' '>/dev/null'
			do	for TEST_substitute in '' ': $'
				do

	TEST_test="$TEST_substitute($TEST_fork $TEST_exec $TEST_command $TEST_redirect)"
	[[ $TEST_test == '('*([[:space:]])')' ]] && continue
	r=$($SHELL -c '
		{
			sleep 2
			kill -KILL $$
		} &
		'"$TEST_test"'
		kill $!
		print ok
		')
	[[ $r == ok ]] || err_exit "shell hangs on $TEST_test"

				done
			done
		done
	done
done

cat=$bincat
[[ ${ print foo|$cat;} == foo ]] || err_exit "\${ print foo|$cat;} not working"
$SHELL -c '( autoload xxxxx);print -n' ||  err_exit 'autoloaded functions in subshells can cause failure'
foo=$($SHELL  <<- ++EOF++
	(trap 'print bar' EXIT;print -n foo)
	++EOF++
)
[[ $foo == foobar ]] || err_exit 'trap on exit when last commands is subshell is not triggered'

err=$(
	$SHELL  2>&1  <<- \EOF
	        date=$(whence -p date)
	        function foo
	        {
	                x=$( $date > /dev/null 2>&1 ;:)
	        }
		integer max=$(ulimit --nofile)
		(( max= max<70?max=64:max-6))
		integer dup
		# consume almost all fds to push the test to the fd limit #
		for ((i=20; i < max; i++))
		do	exec {i}>&1
		done
	        for ((i=0; i < 20; i++))
	        do      y=$(foo)
	        done
	EOF
) || {
	err=${err%%$'\n'*}
	err=${err#*:}
	err=${err##[[:space:]]}
	err_exit "nested command substitution with redirections failed -- $err"
}

exp=0
$SHELL -c $'
	function foobar
	{
		print "hello world"
	}
	[[ $(getopts \'[+?X\ffoobar\fX]\' v --man 2>&1) == *"Xhello worldX"* ]]
	exit '$exp$'
'
got=$?
[[ $got == $exp ]] || err_exit "getopts --man runtime callout with nonzero exit terminates shell -- expected '$exp', got '$got'"
exp=ok
got=$($SHELL -c $'
	function foobar
	{
		print "hello world"
	}
	[[ $(getopts \'[+?X\ffoobar\fX]\' v --man 2>&1) == *"Xhello worldX"* ]]
	print '$exp$'
')
[[ $got == $exp ]] || err_exit "getopts --man runtime callout with nonzero exit terminates shell -- expected '$exp', got '$got'"

# command substitution variations #
set -- \
	'$('			')'		\
	'${ '			'; }'		\
	'$(ulimit -c 0; '	')'		\
	'$( ('			') )'		\
	'${ ('			'); }'		\
	'`'			'`'		\
	'`('			')`'		\
	'`ulimit -c 0; '	'`'		\
	# end of table #
exp=ok
testcase[1]='
	if	%sexpr "NOMATCH" : ".*Z" >/dev/null%s
	then	print error
	else	print ok
	fi
	exit %s
'
testcase[2]='
	function bar
	{
		pipeout=%1$sprintf Ok | tr O o%2$s
		print $pipeout
		return 0
	}
	foo=%1$sbar%2$s || foo="exit status $?"
	print $foo
	exit %3$s
'
while	(( $# >= 2 ))
do	for ((TEST=1; TEST<=${#testcase[@]}; TEST++))
	do	body=${testcase[TEST]}
		for code in 0 2
		do	got=${ printf "$body" "$1" "$2" "$code" | $SHELL 2>&1 }
			status=$?
			if	(( status != code ))
			then	err_exit "test $TEST '$1...$2 exit $code' failed -- exit status $status, expected $code"
			elif	[[ $got != $exp ]]
			then	err_exit "test $TEST '$1...$2 exit $code' failed -- got '$got', expected '$exp'"
			fi
		done
	done
	shift 2
done

# this set of tests loop on all combinations of
#	{ BEG CAT ADD TST } X { file-sizes }
# where the file size starts at 1Ki and doubles up to and including 1Mi
#
# the tests and timeouts are done in async subshells to prevent
# the test harness from hanging

BEG=(  '$( '  '${ '  )
END=(  ' )'   '; }'  )
CAT=(  cat  $bincat  )
ADD=(  ""  "builtin cat; "  "builtin -d cat $bincat; "  ": > /dev/null; "  )
TST=(
	'$cat < $tmp/lin'
	'cat $tmp/lin | $cat'
	'read v < $tmp/buf; print $v'
	'cat $tmp/buf | read v; print $v'
)

# prime the two data files to 512 bytes each
# $tmp/lin has newlines every 16 bytes and $tmp/buf has no newlines

buf=$'1234567890abcdef'
lin=$'\n1234567890abcde'
for ((i=0; i<5; i++))
do	buf=$buf$buf
	lin=$lin$lin
done
print -n "$buf" > $tmp/buf
print -n "$lin" > $tmp/lin

for ((exp=1024; exp<=1024*1024; exp*=2))
do	cat $tmp/buf $tmp/buf > $tmp/tmp
	mv $tmp/tmp $tmp/buf
	cat $tmp/lin $tmp/lin > $tmp/tmp
	mv $tmp/tmp $tmp/lin
	for ((S=0; S<${#BEG[@]}; S++))
	do	for ((C=0; C<${#CAT[@]}; C++))
		do	cat=${CAT[C]}
			for ((A=0; A<${#ADD[@]}; A++))
			do	for ((T=0; T<${#TST[@]}; T++))
				do	if	[[ ! ${ERR[S][C][A][T]} ]]
					then	eval "{ x=${BEG[S]}${ADD[A]}${TST[T]}${END[S]}; print \${#x}; } >\$tmp/out &"
						m=$!
						{ sleep 4; kill -9 $m; } &
						k=$!
						wait $m
						kill -9 $k
						wait $k
						got=$(<$tmp/out)
						if	[[ $got != $exp ]]
						then	# on failure disable similar tests on larger files sizes #
							ERR[S][C][A][T]=1
							# read has per-variable size limitations imposed by sfio #
							if	(( got < 8*1024 )) || [[ ${TST[T]} != *read* ]]
							then	err_exit "${BEG[S]}${ADD[A]}${TST[T]}${END[S]} failed on file size $(printf $'%#i' $exp) -- expected '$exp', got '$got'"
							fi
						fi
					fi
				done
			done
		done
	done
done

exit $Errors
