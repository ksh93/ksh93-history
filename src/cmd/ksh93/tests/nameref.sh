####################################################################
#                                                                  #
#             This software is part of the ast package             #
#                Copyright (c) 1982-2001 AT&T Corp.                #
#        and it may only be used by you under license from         #
#                       AT&T Corp. ("AT&T")                        #
#         A copy of the Source Code Agreement is available         #
#                at the AT&T Internet web site URL                 #
#                                                                  #
#       http://www.research.att.com/sw/license/ast-open.html       #
#                                                                  #
#        If you have copied this software without agreeing         #
#        to the terms of the license you are infringing on         #
#           the license and copyright and are violating            #
#               AT&T's intellectual property rights.               #
#                                                                  #
#                 This software was created by the                 #
#                 Network Services Research Center                 #
#                        AT&T Labs Research                        #
#                         Florham Park NJ                          #
#                                                                  #
#                David Korn <dgk@research.att.com>                 #
####################################################################
function err_exit
{
	print -u2 -n "\t"
	print -u2 -r $Command: "$@"
	let Errors+=1
}

Command=$0
integer Errors=0
function checkref
{
	nameref foo=$1 bar=$2
	if	[[ $foo !=  $bar ]]
	then	err_exit "foo=$foo != bar=$bar"
	fi
	foo=hello
	if	[[ $foo !=  $bar ]]
	then	err_exit "foo=$foo != bar=$bar"
	fi
	foo.child=child
	if	[[ ${foo.child} !=  ${bar.child} ]]
	then	err_exit "foo.child=${foo.child} != bar=${bar.child}"
	fi
}

name=first
checkref name name
name.child=second
checkref name name
.foo=top
.foo.bar=next
checkref .foo.bar .foo.bar
if	[[ ${.foo.bar} !=  hello ]]
then	err_exit ".foo.bar=${.foo.bar} != hello"
fi
if	[[ ${.foo.bar.child} !=  child ]]
then	err_exit ".foo.bar.child=${.foo.bar.child} != child"
fi
function func1
{
        nameref color=$1
        func2 color
}

function func2
{
        nameref color=$1
        set -s -- ${!color[@]}
	print -r -- "$@"
}

typeset -A color
color[apple]=red
color[grape]=purple
color[banana]=yellow
if	[[ $(func1 color) != 'apple banana grape' ]]
then	err_exit "nameref or nameref not working"
fi
nameref x=.foo.bar
if	[[ ${!x} != .foo.bar ]]
then	err_exit "${!x} not working"
fi
typeset +n x
unset x
nameref x=.foo.bar
function x.set
{
	[[ ${.sh.value} ]] && print hello
}
if	[[ $(.foo.bar.set) != $(x.set) ]]
then	err_exit "function references  not working"
fi
if	[[ $(typeset +n) != x ]]
then	err_exit "typeset +n doesn't list names of reference variables"
fi
if	[[ $(typeset -n) != x=.foo.bar ]]
then	err_exit "typeset +n doesn't list values of reference variables"
fi
file=/tmp/shtest$$
typeset +n foo bar 2> /dev/null
unset foo bar
export bar=foo
nameref foo=bar
if	[[ $foo != foo ]]
then	err_exit "value of nameref foo !=  $foo"
fi
trap "rm -f $file" EXIT INT
cat > $file <<\!
print -r -- $foo
!
chmod +x "$file"
y=$( $file)
if	[[ $y != '' ]]
then	err_exit "reference variable not cleared"
fi
{ 
	command nameref xx=yy
	command nameref yy=xx
} 2> /dev/null && err_exit "self reference not detected"
typeset +n foo bar
unset foo bar
set foo
nameref bar=$1
foo=hello
if	[[ $bar !=  hello ]]
then	err_exit 'nameref of positional paramters outside of function not working'
fi
unset foo bar
bar=123
function foobar 
{
	typeset -n foo=bar
	typeset -n foo=bar
}
foobar 2> /dev/null || err_exit 'nameref not unsetting previous reference'
(
	nameref short=verylong
	short=( A=a B=b )
	if	[[ ${verylong.A} != a ]]
	then	err_exit 'nameref short to longname compound assignment error'
	fi
) 2> /dev/null|| err_exit 'nameref short to longname compound assignment error'
unset x
if	[[	$(var1=1 var2=2
		for i in var1 var2
		do	nameref x=$i
			print $x
		done) != $'1\n2' ]]
then	err_exit 'for loop nameref optimization error'
fi
exit $((Errors))
