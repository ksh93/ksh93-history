########################################################################
#                                                                      #
#               This software is part of the ast package               #
#          Copyright (c) 1982-2008 AT&T Intellectual Property          #
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
	print -u2 -n "\t"
	print -u2 -r ${Command}[$1]: "${@:2}"
	(( Errors+=1 ))
}
alias err_exit='err_exit $LINENO'

Command=${0##*/}
integer Errors=0


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
	[[ ${x.foo} == good ]] ||  err_exit 'x.foo should be good'
)
[[ $x == "$val" ]] || err_exit 'compound variable changes after unset leaves'
exit $Errors
