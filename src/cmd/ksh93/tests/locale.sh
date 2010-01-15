########################################################################
#                                                                      #
#               This software is part of the ast package               #
#          Copyright (c) 1982-2010 AT&T Intellectual Property          #
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
	let Errors+=1
}
alias err_exit='err_exit $LINENO'

Command=${0##*/}
integer Errors=0

tmp=$(mktemp -dt) || { err_exit mktemp -dt failed; exit 1; }
trap "cd /; rm -rf $tmp" EXIT
cd $tmp || exit

unset LANG LC_ALL LC_CTYPE LC_NUMERIC

# LC_ALL=debug is an ast specific debug/test locale

if	[[ "$(LC_ALL=debug $SHELL <<- \+EOF+
		x=a<1z>b<2yx>c
		print ${#x}
		+EOF+)" != 5
	]]
then	err_exit '${#x} not working with multibyte locales'
fi

a=$($SHELL -c '/' 2>&1 | sed -e "s,.*: *,," -e "s, *\[.*,,")
b=$($SHELL -c '(LC_ALL=debug / 2>/dev/null); /' 2>&1 | sed -e "s,.*: *,," -e "s, *\[.*,,")
[[ "$b" == "$a" ]] || err_exit "locale not restored after subshell -- expected '$a', got '$b'"
b=$($SHELL -c '(LC_ALL=debug; / 2>/dev/null); /' 2>&1 | sed -e "s,.*: *,," -e "s, *\[.*,,")
[[ "$b" == "$a" ]] || err_exit "locale not restored after subshell -- expected '$a', got '$b'"

# test shift-jis \x81\x40 ... \x81\x7E encodings
# (shift char followed by 7 bit ascii)

typeset -i16 chr
for locale in $(PATH=/bin:/usr/bin locale -a 2>/dev/null | grep -i jis)
do	export LC_ALL=$locale
	for ((chr=0x40; chr<=0x7E; chr++))
	do	c=${chr#16#}
		for s in \\x81\\x$c \\x$c
		do	b="$(printf "$s")"
			eval n=\$\'$s\'
			[[ $b == "$n" ]] || err_exit "LC_ALL=$locale printf difference for \"$s\" -- expected '$n', got '$b'"
			u=$(print -- $b)
			q=$(print -- "$b")
			[[ $u == "$q" ]] || err_exit "LC_ALL=$locale quoted print difference for \"$s\" -- $b => '$u' vs \"$b\" => '$q'"
		done
	done
done

# find a supported UTF-8 locale

punt=
for utf_8 in $(PATH=/bin:/usr/bin locale -a 2>/dev/null | egrep '^(de|en|es|fr)_.*?(\.UTF-8)') de_DE fr_FR es_ES en_US ''
do	[[ $utf_8 ]] || break
	while	:
	do	[[ $utf_8 == *.UTF-8 ]] || utf_8=${utf_8%.*}.UTF-8
		if	[[ ! $($SHELL -c "LC_ALL=$utf_8" 2>&1) ]]
		then	[[ $utf_8 == @(de|fr)* ]] && break 2
			[[ $punt ]] || punt=$utf_8
		fi
		[[ $utf_8 == *_* ]] || break
		utf_8=${utf_8%_*}
	done
done
[[ $utf_8 ]] || utf_8=$punt

[[ $utf_8 ]] &&
{

export LC_ALL=C

# test multibyte value/trace format -- $'\303\274' is UTF-8 u-umlaut

c=$(LC_ALL=C $SHELL -c "printf $':%2s:\n' $'\303\274'")
u=$(LC_ALL=$utf_8 $SHELL -c "printf $':%2s:\n' $'\303\274'" 2>/dev/null)
if	[[ "$c" != "$u" ]]
then	LC_ALL=$utf_8
	x=$'+2+ typeset item.text\
+3+ item.text=\303\274\
+4+ print -- \303\274\
\303\274\
+5+ eval $\'arr[0]=(\\n\\ttext=\\303\\274\\n)\'
+2+ arr[0].text=ü\
+6+ print -- \303\274\
ü\
+7+ eval txt=$\'(\\n\\ttext=\\303\\274\\n)\'
+2+ txt.text=\303\274\
+8+ print -- \'(\' text=$\'\\303\\274\' \')\'\
( text=\303\274 )'
	u=$(LC_ALL=$utf_8 PS4='+$LINENO+ ' $SHELL -x -c "
		item=(typeset text)
		item.text=$'\303\274'
		print -- \"\${item.text}\"
		eval \"arr[0]=\$item\"
		print -- \"\${arr[0].text}\"
		eval \"txt=\${arr[0]}\"
		print -- \$txt
	" 2>&1)
	[[ "$u" == "$x" ]] || err_exit LC_ALL=$utf_8 multibyte value/trace format failed

	x=$'00fc\n20ac'
	u=$(LC_ALL=$utf_8 $SHELL -c $'printf "%04x\n" \$\'\"\303\274\"\' \$\'\"\xE2\x82\xAC\"\'')
	[[ $u == $x ]] || err_exit LC_ALL=$utf_8 multibyte %04x printf format failed
fi

if	(( $($SHELL -c $'export LC_ALL='$utf_8$'; print -r "\342\202\254\342\202\254\342\202\254\342\202\254w\342\202\254\342\202\254\342\202\254\342\202\254" | wc -m' 2>/dev/null) == 10 ))
then	LC_ALL=$utf_8 $SHELL -c b1=$'"\342\202\254\342\202\254\342\202\254\342\202\254w\342\202\254\342\202\254\342\202\254\342\202\254"; [[ ${b1:4:1} == w ]]' || err_exit 'multibyte ${var:offset:len} not working correctly'
fi

#$SHELL -c 'export LANG='$utf_8'; printf "\u[20ac]\u[20ac]" > $tmp/two_euro_chars.txt'
printf $'\342\202\254\342\202\254' > $tmp/two_euro_chars.txt
exp="6 2 6"
set -- $($SHELL -c "
	unset LC_CTYPE
	export LANG=$utf_8
	export LC_ALL=C
	command wc -C < $tmp/two_euro_chars.txt
	unset LC_ALL
	command wc -C < $tmp/two_euro_chars.txt
	export LC_ALL=C
	command wc -C < $tmp/two_euro_chars.txt
")
got=$*
[[ $got == $exp ]] || err_exit "command wc LC_ALL default failed -- expected '$exp', got '$got'"
set -- $($SHELL -c "
	if	builtin -f cmd wc 2>/dev/null
	then	unset LC_CTYPE
		export LANG=$utf_8
		export LC_ALL=C
		wc -C < $tmp/two_euro_chars.txt
		unset LC_ALL
		wc -C < $tmp/two_euro_chars.txt
		export LC_ALL=C
		wc -C < $tmp/two_euro_chars.txt
	fi
")
got=$*
[[ $got == $exp ]] || err_exit "builtin wc LC_ALL default failed -- expected '$exp', got '$got'"

# multibyte char straddling buffer boundary

{
	unset i
	integer i
	for ((i = 0; i < 163; i++))
	do	print "#234567890123456789012345678901234567890123456789"
	done
	printf $'%-.*c\n' 15 '#'
	for ((i = 0; i < 2; i++))
	do	print $': "\xe5\xae\x9f\xe8\xa1\x8c\xe6\xa9\x9f\xe8\x83\xbd\xe3\x82\x92\xe8\xa1\xa8\xe7\xa4\xba\xe3\x81\x97\xe3\x81\xbe\xe3\x81\x99\xe3\x80\x82" :'
	done
} > ko.dat

LC_ALL=$utf_8 $SHELL <ko.dat 2>/dev/null || err_exit "script with multibyte char straddling buffer boundary fails"

}

# the ast debug locale has { decimal_point="," thousands_sep="." }

locale=$utf_8
[[ ! $locale || $locale == e[ns]* ]] && locale=debug

#	exp	LC_ALL		LC_NUMERIC	LANG
set -- \
	2,5	$locale		C		''		\
	2.5	C		$locale		''		\
	2,5	$locale		''		C		\
	2,5	''		$locale		C		\
	2.5	C		''		$locale		\
	2.5	''		C		$locale		\

unset a b c
unset LC_ALL LC_NUMERIC LANG
integer a b c
while	(( $# >= 4 ))
do	exp=$1
	unset H V
	typeset -A H
	typeset -a V
	[[ $2 ]] && V[0]="export LC_ALL=$2;"
	[[ $3 ]] && V[1]="export LC_NUMERIC=$3;"
	[[ $4 ]] && V[2]="export LANG=$4;"
	for ((a = 0; a < 3; a++))
	do	for ((b = 0; b < 3; b++))
		do	if	(( b != a ))
			then	for ((c = 0; c < 3; c++))
				do	if	(( c != a && c != b ))
					then	T=${V[$a]}${V[$b]}${V[$c]}
						if	[[ ! ${H[$T]} ]]
						then	H[$T]=1
							got=$($SHELL -c "${T}print \$(( $exp ))" 2>&1)
							[[ $got == $exp ]] || err_exit "${T} sequence failed -- expected '$exp', got '$got'"
						fi
					fi
				done
			fi
		done
	done
	shift 4
done

# special builtin error message localization

exp=2
for cmd in \
	'cd _not_found_; export LC_ALL=debug; cd _not_found_' \
	'cd _not_found_; LC_ALL=debug cd _not_found_' \

do	got=$($SHELL -c "$cmd" 2>&1 | sort -u | wc -l)
	(( ${got:-0} == $exp )) || err_exit "'$cmd' sequence failed -- error message not localized"
done

# setocale(LC_ALL,"") after setlocale() initialization

locale=$utf_8

if	[[ $locale ]]
then	printf 'f1\357\274\240f2\n' > input1
	printf 't2\357\274\240f1\n' > input2
	printf '\357\274\240\n' > delim
	print "export LC_ALL=$locale
join -j1 1 -j2 2 -o 1.1 -t \$(cat delim) input1 input2 > out" > script
	$SHELL -c 'unset LANG ${!LC_*}; $SHELL ./script' ||
	err_exit "join test script failed -- exit code $?"
	exp="f1"
	got="$(<out)"
	[[ $got == "$exp" ]] || err_exit "LC_ALL test script failed -- expected '$exp', got '$got'"
fi

if	[[ $locale && $locale != en* ]]
then	dir=_not_found_
	exp=121
	for lc in LANG LC_MESSAGES LC_ALL
	do	for cmd in "($lc=$locale;cd $dir)" "$lc=$locale;cd $dir;unset $lc" "function tst { typeset $lc=$locale;cd $dir; }; tst"
		do	tst="$lc=C;cd $dir;$cmd;cd $dir;:"
			$SHELL -c "unset LANG \${!LC_*}; $SHELL -c '$tst'" > out 2>&1 ||
			err_exit "'$tst' failed -- exit status $?"
			integer id=0
			unset msg
			typeset -A msg
			got=
			while	read -r line
			do	line=${line##*:}
				if	[[ ! ${msg[$line]} ]]
				then	msg[$line]=$((++id))
				fi
				got+=${msg[$line]}
			done < out
			[[ $got == $exp ]] || err_exit "'$tst' failed -- expected '$exp', got '$got'"
		done
	done
fi

exit $Errors
