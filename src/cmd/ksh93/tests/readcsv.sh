########################################################################
#                                                                      #
#               This software is part of the ast package               #
#          Copyright (c) 1982-2012 AT&T Intellectual Property          #
#                      and is licensed under the                       #
#                 Eclipse Public License, Version 1.0                  #
#                    by AT&T Intellectual Property                     #
#                                                                      #
#                A copy of the License is available at                 #
#          http://www.eclipse.org/org/documents/epl-v10.html           #
#         (with md5 checksum b35adb5213ca9657e911e9befb180842)         #
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
	let Errors+=1
}
alias err_exit='err_exit $LINENO'

Command=${0##*/}
integer Errors=0

tmp=$(mktemp -dt) || { err_exit mktemp -dt failed; exit 1; }
trap "cd /; rm -rf $tmp" EXIT
	
tmp1=$tmp/tmp1.csv
tmp2=$tmp/tmp2.csv
trap 'rm $tmp1 $tmp2' EXIT
cat > $tmp1 <<- \EOF
	CAT,"CVE CCODE","NECA OCN",ST,LATA,AP,"New InterState
	Orig","New Inter""""State
	Term","New IntraState
	Orig","New IntraState
	Term"
	CLEC,XXXX,AAAA,RB,ABC,comp," 0.2 "," 0.4 "," 0.6 "," 0.8 "
	CLEC,YYYY,QQQQ,SX,123,mmmm," 0.3 "," 0.5 "," 0.7 "," 0.9 "
EOF
integer count=0 nfields
IFS=${2-,}
typeset -a arr
while read -A -S arr
do	((nfields=${#arr[@]}))
	if	((++count==1))
	then	((nfields==10)) || err_exit 'first record should contain 10 fields'
		[[ ${arr[7]} == $'New Inter""State\nTerm' ]] || err_exit $'7th field of record 1 should contain New Inter""State\nTerm'
	fi
	for	((i=0; i < nfields;i++))
	do	delim=$IFS
		if	((i == nfields-1))
		then	delim=$'\r\n'
		fi
		printf "%#q%s"  "${arr[i]}" "$delim"
	done
done < $tmp1 > $tmp2
diff -q "$tmp1" "$tmp2" || err_exit "fles $tmp1 and $tmp2 differ"

exit $((Errors<125?Errors:125))

