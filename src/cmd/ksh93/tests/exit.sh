####################################################################
#                                                                  #
#             This software is part of the ast package             #
#                Copyright (c) 1982-2000 AT&T Corp.                #
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
#                                                                  #
####################################################################
function err_exit
{
	print -u2 -n "\t"
	print -u2 -r $Command: "$@"
	let Errors+=1
}
function abspath
{
        base=$(basename $SHELL)
        cd ${SHELL%/$base}
        newdir=$(pwd)
        cd ~-
        print $newdir/$base
}
#test for proper exit of shell
Command=$0
integer Errors=0
ABSHELL=$(abspath)
mkdir /tmp/ksh$$ || err_exit "mkdir /tmp/ksh$$ failed"
cd /tmp/ksh$$ || err_exit "cd /tmp/ksh$$ failed"
print exit >.profile
${ABSHELL}  <<!
LD_LIBRARY_PATH=$LD_LIBRARY_PATH \
LD_LIBRARYN32_PATH=$LD_LIBRARYN32_PATH \
LD_LIBRARY64_PATH=$LD_LIBRARY64_PATH \
LIBPATH=$LIBPATH \
SHLIBPATH=$SHLIBPATH \
exec -c -a -ksh ${ABSHELL} -c "exit 1" 1>/dev/null 2>&1
!
if [[ $(echo $?) != 0 ]]
then err_exit 'exit in .profile is ignored'
fi
cd ~- || err_exit "cd back failed"
rm -r /tmp/ksh$$ || err_exit "rm -r /tmp/ksh$$ failed"
exit $((Errors))
