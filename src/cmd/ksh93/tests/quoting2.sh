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
#     If you received this software without first entering     #
#       into a license with AT&T, you have an infringing       #
#           copy and cannot use it without violating           #
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

Command=$0
integer Errors=0
set -o noglob
if	[[ 'hi there' != "hi there" ]]
then	err_exit "single quotes not the same as double quotes"
fi
x='hi there'
if	[[ $x != 'hi there' ]]
then	err_exit "$x not the same as 'hi there'"
fi
if	[[ $x != "hi there" ]]
then	err_exit "$x not the same as \"hi there \""
fi
if	[[ \a\b\c\*\|\"\ \\ != 'abc*|" \' ]]
then	err_exit " \\ differs from '' "
fi
if	[[ "ab\'\"\$(" != 'ab\'\''"$(' ]]
then	err_exit " \"\" differs from '' "
fi
if	[[ $(print -r - 'abc*|" \') !=  'abc*|" \' ]]
then	err_exit "\$(print -r - '') differs from ''"
fi
if	[[ $(print -r - "abc*|\" \\") !=  'abc*|" \' ]]
then	err_exit "\$(print -r - '') differs from ''"
fi
if	[[ "$(print -r - 'abc*|" \')" !=  'abc*|" \' ]]
then	err_exit "\"\$(print -r - '')\" differs from ''"
fi
if	[[ "$(print -r - "abc*|\" \\")" !=  'abc*|" \' ]]
then	err_exit "\"\$(print -r - "")\" differs from ''"
fi
if	[[ $(print -r - "$(print -r - 'abc*|" \')") !=  'abc*|" \' ]]
then	err_exit "nested \$(print -r - '') differs from ''"
fi
if	[[ "$(print -r - $(print -r - 'abc*|" \'))" !=  'abc*|" \' ]]
then	err_exit "\"nested \$(print -r - '')\" differs from ''"
fi
if	[[ $(print -r - "$(print -r - 'abc*|" \')") !=  'abc*|" \' ]]
then	err_exit "nested \"\$(print -r - '')\" differs from ''"
fi
unset x
if	[[ ${x-$(print -r - "abc*|\" \\")} !=  'abc*|" \' ]]
then	err_exit "\${x-\$(print -r - '')} differs from ''"
fi
if	[[ ${x-$(print -r - "a}c*|\" \\")} !=  'a}c*|" \' ]]
then	err_exit "\${x-\$(print -r - '}')} differs from ''"
fi
x=$((echo foo)|(cat))
if	[[ $x != foo  ]]
then	err_exit "((cmd)|(cmd)) failed"
fi
x=$(print -r -- "\"$HOME\"")
if	[[ $x != '"'$HOME'"' ]]
then	err_exit "nested double quotes failed"
fi
: ${z="a{b}c"}
if	[[ $z != 'a{b}c' ]]
then	err_exit '${z="a{b}c"} not correct'
fi
unset z
: "${z="a{b}c"}"
if	[[ $z != 'a{b}c' ]]
then	err_exit '"${z="a{b}c"}" not correct'
fi
if	[[ $(print -r -- "a\*b") !=  'a\*b' ]]
then	err_exit '$(print -r -- "a\*b") differs from  a\*b'
fi
unset x
if	[[ $(print -r -- "a\*b$x") !=  'a\*b' ]]
then	err_exit '$(print -r -- "a\*b$x") differs from  a\*b'
fi
x=hello
set -- ${x+foo bar bam}
if	(( $# !=3 ))
then	err_exit '${x+foo bar bam} does not yield three arguments'
fi
set -- ${x+foo "bar bam"}
if	(( $# !=2 ))
then	err_exit '${x+foo "bar bam"} does not yield two arguments'
fi
set -- ${x+foo 'bar bam'}
if	(( $# !=2 ))
then	err_exit '${x+foo '\''bar bam'\''} does not yield two arguments'
fi
set -- ${x+foo $x bam}
if	(( $# !=3 ))
then	err_exit '${x+foo $x bam} does not yield three arguments'
fi
set -- ${x+foo "$x" bam}
if	(( $# !=3 ))
then	err_exit '${x+foo "$x" bam} does not yield three arguments'
fi
set -- ${x+"foo $x bam"}
if	(( $# !=1 ))
then	err_exit '${x+"foo $x bam"} does not yield one argument'
fi
set -- "${x+foo $x bam}"
if	(( $# !=1 ))
then	err_exit '"${x+foo $x bam}" does not yield one argument'
fi
set -- ${x+foo "$x "bam}
if	(( $# !=2 ))
then	err_exit '${x+foo "$x "bam} does not yield two arguments'
fi
x="ab$'cd"
if	[[ $x != 'ab$'"'cd" ]]
then	err_exit '$'"' inside double quotes not working"
fi
x=`print 'ab$'`
if	[[ $x != 'ab$' ]]
then	err_exit '$'"' inside `` quotes not working"
fi
unset a
x=$(print -r -- "'\
\
")
if	[[ $x != "'" ]]
then	err_exit 'line continuation in double strings not working'
fi
x=$(print -r -- "'\
$a\
")
if	[[ $x != "'" ]]
then	err_exit 'line continuation in expanded double strings not working'
fi
x='\*'
if	[[ $(print -r -- $x) != '\*' ]]
then	err_exit 'x="\\*";$x != \*'
fi
if	[[ $(print -r -- "\}" ) != '\}' ]]
then	err_exit '(print -r -- "\}"' not working
fi
if	[[ $(print -r -- "\{" ) != '\{' ]]
then	err_exit 'print -r -- "\{"' not working
fi
# The following caused a syntax error on earlier versions
foo=foo x=-
if	[[  `eval print \\${foo$x}` != foo* ]]
then	err_exit '`eval  print \\${foo$x}`' not working
fi
if	[[  "`eval print \\${foo$x}`" != foo* ]]
then	err_exit '"`eval  print \\${foo$x}`"' not working
fi
if	( [[ $() != '' ]] )
then	err_exit '$() not working'
fi
x=a:b:c
set -- $( IFS=:; print $x)
if	(( $# != 3))
then	err_exit 'IFS not working correctly with command substitution'
fi
$SHELL -n 2> /dev/null << \! || err_exit '$(...) bug with ( in comment'
y=$(
	# ( this line is a bug fix
	print hi
)
!
exit $((Errors))
