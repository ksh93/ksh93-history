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
integer x=1 y=2 z=3
if	(( 2+2 != 4 ))
then	err_exit 2+2!=4
fi
if	((x+y!=z))
then	err_exit x+y!=z
fi
if	(($x+$y!=$z))
then	err_exit $x+$y!=$z
fi
if	(((x|y)!=z))
then	err_exit "(x|y)!=z"
fi
if	((y >= z))
then	err_exit "y>=z"
fi
if	((y+3 != z+2))
then	err_exit "y+3!=z+2"
fi
if	((y<<2 != 1<<3))
then	err_exit "y<<2!=1<<3"
fi
if	((133%10 != 3))
then	err_exit "133%10!=3"
	if	(( 2.5 != 2.5 ))
	then	err_exit 2.5!=2.5
	fi
fi
d=0
((d || 1)) || err_exit 'd=0; ((d||1))'
if	(( d++!=0))
then	err_exit "d++!=0"
fi
if	(( --d!=0))
then	err_exit "--d!=0"
fi
if	(( (d++,6)!=6 && d!=1))
then	err_exit '(d++,6)!=6 && d!=1'
fi
d=0
if	(( (1?2+1:3*4+d++)!=3 || d!=0))
then	err_exit '(1?2+1:3*4+d++) !=3'
fi
for	((i=0; i < 20; i++))
do	:
done
if	(( i != 20))
then	err_exit 'for (( expr)) failed'
fi
for	((i=0; i < 20; i++)); do	: ; done
if	(( i != 20))
then	err_exit 'for (( expr));... failed'
fi
for	((i=0; i < 20; i++)) do	: ; done
if	(( i != 20))
then	err_exit 'for (( expr))... failed'
fi
if	(( (i?0:1) ))
then	err_exit '(( (i?0:1) )) failed'
fi
if	(( (1 || 1 && 0) != 1 ))
then	err_exit '( (1 || 1 && 0) != 1) failed'
fi
if	(( (_=1)+(_x=0)-_ ))
then	err_exit '(_=1)+(_x=0)-_ failed'
fi
if	((  (3^6) != 5))
then	err_exit '((3^6) != 5) failed'
fi
integer x=1
if	(( (x=-x) != -1 ))
then	err_exit '(x=-x) != -1 failed'
fi
i=2
if	(( 1$(($i))3 != 123 ))
then	err_exit ' 1$(($i))3 failed'
fi
((pi=4*atan(1.)))
point=(
	float x
	float y
)
(( point.x = cos(pi/6), point.y = sin(pi/6) ))
if	(( point.x*point.x + point.y*point.y > 1.01 ))
then	err_exit 'cos*cos +sin*sin > 1.01'
fi
if	(( point.x*point.x + point.y*point.y < .99 ))
then	err_exit 'cos*cos +sin*sin < .99'
fi
if [[ $((y=x=1.5)) != 1 ]]
then	err_exit 'typecast not working in arithmetic evaluation'
fi
typeset -E x=1.5
( ((x++))  ) 2>/dev/null
if [[ $? == 0 ]]
then	err_exit 'postincrement of floating point allowed'
fi
( ((++x))  ) 2>/dev/null
if [[ $? == 0 ]]
then	err_exit 'preincrement of floating point allowed'
fi
x=.125
if	[[ $(( 4 * x/2 )) != 0.25 ]] 
then	err_exit '(( 4 * x/2 )) is not 0.25, with x=.125'
fi
if	[[ $(( pow(2,3) )) != 8 ]]
then	err_exit '$(( pow(2,3) )) != 8'
fi
( [[ $(( pow(2,(3)) )) == 8 ]] ) 2> /dev/null
if	(( $? ))
then	err_exit '$(( pow(2,(3)) )) != 8'
fi
unset x
integer x=1; integer x=1
if	[[ $x != 1 ]]
then	err_exit 'two consecutive integer x=1 not working'
fi
unset z
{ z=$(typeset -RZ2 z2; (( z2 = 8 )); print $z2) ;} 2>/dev/null
if [[ $z != "08" ]]
then	err_exit "typeset -RZ2 leading 0 decimal not working [z=$z]"
fi
{ z=$(typeset -RZ3 z3; (( z3 = 8 )); print $z3) ;} 2>/dev/null
if [[ $z != "008" ]]
then	err_exit "typeset -RZ3 leading 0 decimal not working [z=$z]"
fi
unset z
typeset -Z3 z=010
(( z=z+1))
if	[[ $z != 011 ]]
then	err_exit "leading 0's in -Z not treated as decimal"
fi
exit $((Errors))
