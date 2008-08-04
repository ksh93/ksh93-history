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
integer n=2

typeset -T Type_t=(
	typeset name=foobar
	typeset x=(hi=ok bar=yes)
	typeset y=(xa=xx xq=89)
	typeset -A aa=([one]=abc [two]=def)
	typeset -a ia=(abc def)
	typeset -i z=5
)
for ((i=0; i < 10; i++))
do
	Type_t r s
	[[ $r == "$s" ]] || err_exit 'r is not equal to s'
	x=${r.x}
	y=(xa=bb xq=cc)
	y2=xyz
	z2=xyz
	z="$y"
	[[ $y == "$z" ]] || err_exit 'y is not equal to z'
	s.y=${z}
	[[ $y == "${s.y}" ]] || err_exit 'y is not equal to s.y'
	.sh.q=$y
	www="${.sh.q}"
	[[ $www == "$z" ]] ||  err_exit 'www is not equal to z'
	s.x=${r.x}
	[[ ${s.x} == "${r.x}" ]] ||  err_exit 's.x is not equal to r.x'

	function foo
	{
		nameref x=$1 y=$2
		typeset z=$x
		y=$x
		[[ $x == "$y" ]] || err_exit "x is not equal to y with ${!x}"
	}
	foo  r.y y  
	[[ $y == "${r.y}" ]] || err_exit 'y is not equal to r.y'
	y=$z
	foo y r.y
	[[ $y == "${r.y}" ]] || err_exit 'y is not equal to r.y again'
	y=$z
	(
		q=${z}
		[[ $q == "$z" ]] ||  err_exit 'q is not equal to z'
		z=abc
	)
	[[ $z == "$y" ]] || err_exit 'value of z not preserved after subshell'
	unset z y r s x  z2 y2 www .sh.q
done
typeset -T Frame_t=( typeset file lineno )
Frame_t frame
[[ $(typeset -p frame) == 'Frame_t frame=(typeset file;typeset lineno;)' ]] || err_exit  'empty fields in type not displayed'
exit $Errors
