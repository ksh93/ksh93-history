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
f=/tmp/here1$$
g=/tmp/here2$$
trap "rm -f $f $g" EXIT
cat > $f <<!
hello world
!
if	[[ $(<$f) != 'hello world' ]]
then	err_exit "'hello world' here doc not working"
fi
cat > $g <<\!
hello world
!
cmp $f $g 2> /dev/null || err_exit "'hello world' quoted here doc not working"
cat > $g <<- !
	hello world
!
cmp $f $g 2> /dev/null || err_exit "'hello world' tabbed here doc not working"
cat > $g <<- \!
	hello world
!
cmp $f $g 2> /dev/null || err_exit "'hello world' quoted tabbed here doc not working"
x=hello
cat > $g <<!
$x world
!
cmp $f $g 2> /dev/null || err_exit "'$x world' here doc not working"
cat > $g <<!
$(print hello) world
!
cmp $f $g 2> /dev/null || err_exit "'$(print hello) world' here doc not working"
cat > $f <<\!!
!@#$%%^^&*()_+~"::~;'`<>?/.,{}[]
!!
if	[[ $(<$f) != '!@#$%%^^&*()_+~"::~;'\''`<>?/.,{}[]' ]]
then	err_exit "'hello world' here doc not working"
fi
cat > $g <<!!
!@#\$%%^^&*()_+~"::~;'\`<>?/.,{}[]
!!
cmp $f $g 2> /dev/null || err_exit "unquoted here doc not working"
exec 3<<!
	foo
!
if	[[ $(<&3) != '	foo' ]]
then	err_exit "leading tabs stripped with <<!"
fi
$SHELL -c "
eval `echo 'cat <<x'` "|| err_exit "eval `echo 'cat <<x'` core dumps"
cat > /dev/null <<EOF # comments should not cause core dumps
abc
EOF
cat >$g << :
:
:
cmp /dev/null $g 2> /dev/null || err_exit "empty here doc not working"
x=$(print $( cat <<HUP
hello
HUP
)
)
if	[[ $x != hello ]]
then	err_exit "here doc inside command sub not working"
fi
y=$(cat <<!
${x:+${x}}
!
)
if	[[ $y != "${x:+${x}}" ]]
then	err_exit '${x:+${x}} not working in here document'
fi
$SHELL -c '
x=0
while (( x < 100 ))
do	((x = x+1))
	cat << EOF
EOF
done
' 2> /dev/null  || err_exit '100 empty here docs fails'
{
	print 'builtin -d cat
	cat <<- EOF'
	for ((i=0; i < 100; i++))
	do print XXXXXXXXXXXXXXXXXXXX
	done
	print ' XXX$(date)XXXX
	EOF'
} > $f
chmod +x "$f"
$SHELL "$f" > /dev/null  || err_exit "large here-doc with command substitution fails"
x=$(/bin/cat <<!
$0
!
)
[[ "$x" == "$0" ]] || err_exit '$0 not correct inside here documents'
$SHELL -c 'x=$(
cat << EOF
EOF)' 2> /dev/null || err_exit 'here-doc cannot be terminated by )'
if	[[ $( IFS=:;cat <<-!
			$IFS$(print hi)$IFS
		!) != :hi: ]]
then	err_exit '$IFS unset by command substitution in here docs'
fi
exit $((Errors))
