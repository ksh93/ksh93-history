################################################################
#                                                              #
#           This software is part of the ast package           #
#              Copyright (c) 1982-2000 AT&T Corp.              #
#      and it may only be used by you under license from       #
#                     AT&T Corp. ("AT&T")                      #
#       A copy of the Source Code Agreement is available       #
#              at the AT&T Internet web site URL               #
#                                                              #
#     http://www.research.att.com/sw/license/ast-open.html     #
#                                                              #
#      If you have copied this software without agreeing       #
#      to the terms of the license you are infringing on       #
#         the license and copyright and are violating          #
#             AT&T's intellectual property rights.             #
#                                                              #
#               This software was created by the               #
#               Network Services Research Center               #
#                      AT&T Labs Research                      #
#                       Florham Park NJ                        #
#                                                              #
#              David Korn <dgk@research.att.com>               #
#                                                              #
################################################################
function err_exit
{
	print -u2 -n "\t"
	print -u2 -r $Command: "$@"
	let Errors+=1
}

# test shell builtin commands
Command=$0
integer Errors=0
: ${foo=bar} || err_exit ": failed"
[[ $foo = bar ]] || err_exit ": side effects failed"
set -- - foobar
[[ $# = 2 && $1 = - && $2 = foobar ]] || err_exit "set -- - foobar failed"
set -- -x foobar
[[ $# = 2 && $1 = -x && $2 = foobar ]] || err_exit "set -- -x foobar failed"
getopts :x: foo || err_exit "getopts :x: returns false"
[[ $foo = x && $OPTARG = foobar ]] || err_exit "getopts :x: failed"
OPTIND=1
getopts :r:s var -r
if	[[ $var != : || $OPTARG != r ]]
then	err_exit "'getopts :r:s var -r' not working"
fi
false ${foo=bar} &&  err_exit "false failed"
read <<!
hello world
!
[[ $REPLY = 'hello world' ]] || err_exit "read builtin failed"
print x:y | IFS=: read a b
if	[[ $a != x ]]
then	err_exit "IFS=: read ... not working"
fi
read <<!
hello \
world
!
[[ $REPLY = 'hello world' ]] || err_exit "read continuation failed"
read -d x <<!
hello worldxfoobar
!
[[ $REPLY = 'hello world' ]] || err_exit "read builtin failed"
read <<\!
hello \
	world \

!
[[ $REPLY == 'hello 	world' ]] || err_exit "read continuation2 failed"
print "one\ntwo" | { read line
	print $line | /bin/cat > /dev/null
	read line
}
read <<\!
\
a\
\
\
b
!
if	[[ $REPLY != ab ]]
then	err_exit "read multiple continuation failed"
fi
if	[[ $line != two ]]
then	err_exit "read from pipeline failed"
fi
line=two
read line < /dev/null
if	[[ $line != "" ]]
then	err_exit "read from /dev/null failed"
fi
if	[[ $(print -R -) != - ]]
then	err_exit "print -R not working correctly"
fi
if	[[ $(print -- -) != - ]]
then	err_exit "print -- not working correctly"
fi
print -f "hello%nbar\n" size > /dev/null
if	((	size != 5 ))
then	err_exit "%n format of printf not working"
fi
print -n -u2 2>&1-
[[ -w /dev/fd/1 ]] || err_exit "2<&1- with built-ins has side effects"
x=$0
if	[[ $(eval 'print $0') != $x ]]
then	err_exit '$0 not correct for eval'
fi
unset x
readonly x
set -- $(readonly)
if      [[ " $@ " != *" x "* ]]
then    err_exit 'unset readonly variables are not displayed'
fi
if	[[ $(	for i in foo bar
		do	print $i
			continue 10
		done
	    ) != $'foo\nbar' ]]
then	err_exit 'continue breaks out of loop'
fi
(continue bad 2>/dev/null && err_exit 'continue bad should return an error')
(break bad 2>/dev/null && err_exit 'break bad should return an error')
(continue 0 2>/dev/null && err_exit 'continue 0 should return an error')
(break 0 2>/dev/null && err_exit 'break 0 should return an error')
if	[[ $(print -f "%b" "\a\n\v\b\r\f\E\03\\oo") != $'\a\n\v\b\r\f\E\03\\oo' ]]
then	err_exit 'print -f "%b" not working'
fi
if	[[ $(print -f "%P" "[^x].*b$") != '*[!x]*b' ]]
then	err_exit 'print -f "%P" not working'
fi
if	[[ $(abc: for i in foo bar;do print $i;break abc;done) != foo ]]
then	err_exit 'break labels not working'
fi
if	[[ $(command -v if)	!= if ]]
then	err_exit	'command -v not working'
fi
read -r var <<\!
	
!
if	[[ $var != "" ]]
then	err_exit "read -r of blank line not working"
fi
mkdir -p /tmp/ksh$$/a/b/c 2>/dev/null || err_exit  "mkdir -p failed"
rm -r /tmp/ksh$$ || err_exit "rm -r /tmp/ksh$$ failed"
trap 'print HUP' HUP
if	[[ $(trap) != "trap -- 'print HUP' HUP" ]]
then	err_exit '$(trap) not working'
fi
n=123
typeset -A base
base[o]=8#
base[x]=16#
base[X]=16#
for i in d i o u x X
do	if	(( $(( ${base[$i]}$(printf "%$i" $n) )) != n  ))
	then	err_exit "printf %$i not working"
	fi
done
if	[[ $( trap 'print done' EXIT) != done ]]
then	err_exit 'trap on EXIT not working'
fi
if	[[ $( trap 'print done' EXIT; trap - EXIT) == done ]]
then	err_exit 'trap on EXIT not being cleared'
fi
if	[[ $(type test) != 'test is a shell builtin' ]]
then	err_exit 'whence -v test not a builtin'
fi
builtin -d test
if	[[ $(type test) != 'test is '*/test ]]
then	err_exit 'whence -v test after builtin -d incorrect'
fi
typeset -Z3 percent=$(printf '%o\n' "'%'")
forrmat=\\${percent}s
if      [[ $(printf "$forrmat") != %s ]]
then    err_exit "printf $forrmat not working"
fi
if	(( $(printf 'x\0y' | wc -c) != 3 ))
then	err_exit 'printf \0 not working'
fi
if	[[ $(printf "%bx%s\n" 'f\to\cbar') != $'f\to' ]]
then	err_exit 'printf %bx%s\n  not working'
fi
alpha=abcdefghijklmnop
if	[[ $(printf "%10.*s\n" 5 $alpha) != '     abcde' ]]
then	err_exit 'printf %10.%s\n  not working'
fi
float x2=.0000625
if	[[ $(printf "%10.5E\n" x2) != 6.25000E-05 ]]
then	err_exit 'printf "%10.5E" not normalizing correctly'
fi
($SHELL read -s foobar <<\!
testing
!
) 2> /dev/null || err_exit ksh read -s var fails
if	[[ $(printf +3 2>/dev/null) !=   +3 ]]
then	err_exit 'printf is not processing formats beginning with + correctly'
fi
if	printf "%d %d\n" 123bad 78 >/dev/null 2>/dev/null
then	err_exit "printf not exiting non-zero with conversion errors"
fi
if	[[ $(trap --version 2> /dev/null;print done) != done ]]
then	err_exit 'trap builtin terminating after --version'
fi
if	[[ $(set --version 2> /dev/null;print done) != done ]]
then	err_exit 'set builtin terminating after --veresion'
fi
unset -f foobar
function foobar
{
	print 'hello world'
}
OPTIND=1
if	[[ $(getopts  $'[+?X\ffoobar\fX]' v --man 2>&1) != *'Xhello world'X* ]]
then	err_exit '\f...\f not working in getopts usage strings'
fi
if	[[	$(printf '%H\n' $'<>"& \'\tabc') != '&lt;&gt;&quot;&amp;&nbsp;&apos;&#9;abc' ]]
then	err_exit 'printf %H not working'
fi
if	[[ $(printf '%..:c\n' abc) != a:b:c ]]
then	err_exit	"printf '%..:c' not working"
fi
if	[[ $(printf '%..*c\n' : abc) != a:b:c ]]
then	err_exit	"printf '%..*c' not working"
fi
if	[[ $(printf '%..:s\n' abc def ) != abc:def ]]
then	err_exit	"printf '%..:s' not working"
fi
if	[[ $(printf '%..*s\n' : abc def) != abc:def ]]
then	err_exit	"printf '%..*s' not working"
fi
exit $((Errors))
