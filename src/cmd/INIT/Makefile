/*
 * initialization for all packages
 */

LICENSE = since=1999,author=gsf

PACKAGEROOT = $(INSTALLROOT:T=F:P=L*:O=n)

iffe :: iffe.sh LICENSE=since=1994,author=gsf+kpv

package :: package.sh

regress :: regress.sh LICENSE=since=1995,author=gsf

$(PACKAGEROOT)/lib/package :INSTALLDIR: package.mk

$(PACKAGEROOT)/bin :INSTALLDIR: ignore mamprobe package silent

crossexec :: crossexec.sh

filter :: filter.sh

ignore :: ignore.sh

mamprobe :: mamprobe.sh

silent :: silent.sh

:PACKAGE_INIT: mamake.c proto.c ratz.c release.c

:: RELEASE hosttype.tst p.c

/*
 * hosttype specific cc workaround initialization
 */

if ! CC.HOSTTYPE
	CC.HOSTTYPE := $(_hosttype_|HOSTTYPE)
end

$(BINDIR)/cc :INSTALL: (CC.HOSTTYPE) package hello.c \
		cc.darwin.ppc \
		cc.hp.pa \
		cc.hp.pa64 \
		cc.ibm.risc \
		cc.mvs.390 \
		cc.next.i386 \
		cc.next.m68k \
		cc.osf.alpha \
		cc.sco.i386 \
		cc.sgi.mips2 \
		cc.sgi.mips3 \
		cc.sgi.mips3-o32 \
		cc.sgi.mips4 \
		cc.sgi.mips4-n32
	for cc in $(*:N=*cc.*.*)
	do	case $cc in
		*cc.$(CC.HOSTTYPE))
			if	$(SILENT) test -f $cc &&
				$cc -o hello.exe $(*:O=2) >/dev/null 2>&1 &&
				hello.exe >/dev/null 2>&1
			then	case "$(CC.HOSTTYPE)" in
				*.mips*)$cc -version >/dev/null 2>&1 || break ;;
				esac
				cmp -s $cc $(<) ||
				cp $cc $(<)
			fi
		esac
	done
	$(SILENT) rm -f hello.exe hello$(CC.SUFFIX.OBJECT)

$(BINDIR)/ld :INSTALL: (CC.HOSTTYPE) package $(BINDIR)/cc \
		ld.hp.pa
	if	$(SILENT) test -x $(BINDIR)/cc
	then	for ld in $(*:N=*ld.*.*)
		do	case $ld in
			*ld.$(CC.HOSTTYPE))
				if	$(SILENT) test -f $ld
				then	cmp -s $ld $(<) ||
					cp $ld $(<)
				fi
			esac
		done
	fi

$(BINDIR)/ldd :INSTALL: .VIRTUAL (CC.HOSTTYPE) package $(BINDIR)/cc \
		ldd.darwin.ppc \
		ldd.ibm.risc \
		ldd.mvs.390 \
		ldd.sgi.mips2 \
		ldd.sgi.mips3 \
		ldd.sgi.mips4
	if	$(SILENT) test -x $(BINDIR)/cc
	then	for ldd in $(*:N=*ldd.*.*)
		do	case $ldd in
			*ldd.$(CC.HOSTTYPE))
				if	$(SILENT) test -f $ldd -a ! -x /usr/bin/ldd -a ! -x /bin/ldd
				then	cmp -s $ldd $(<) ||
					cp $ldd $(<)
				fi
			esac
		done
	fi

/*
 * ksh93 function search on PATH
 * ksh93 ld library path search on PATH
 */

$(BINDIR)/.paths :INSTALL:
	{
	echo FPATH=../fun
	set x x $(.GETCONF LIBPATH:/[:,]/ /G)
	case $#:$(-mam) in
	2:*|*:1)if	test -x /lib/dld.sl
		then	echo SHLIB_PATH=../lib
		elif	test -x /usr/lib/dyld
		then	echo DYLD_LIBRARY_PATH=../lib
		else	case "$(CC.HOSTTYPE)" in
			ibm.*|mvs.*)
				echo LIBPATH=../lib
				;;
			sgi.mips3)	
				echo LD_LIBRARYN32_PATH=../lib
				;;
			sgi.mips4)	
				echo LD_LIBRARYN64_PATH=../lib
				;;
			*)	echo LD_LIBRARY_PATH=../lib
				;;
			esac
		fi
		;;
	*)	while	:
		do	shift 2
			case $1 in
			*32)	case "$(CC.HOSTTYPE)" in
				*3|*32)	echo $2=../lib; break ;;
				esac
				;;
			*64)	case "$(CC.HOSTTYPE)" in
				*4|*64)	echo $2=../lib; break ;;
				esac
				;;
			*)	echo $2=../$1; break
				;;
			esac
			case $# in
			0|1)	break ;;
			esac
		done
		;;
	esac
	} > $(<)

/*
 * probe initialization
 */

for T C+probe C+make+probe.lcl
	if T == "*.lcl"
		$(T) : .DONTCARE
	end
	$(LIBDIR)/probe/$(T:C,+,/,G) :INSTALL: $(T)
end

$(LIBDIR)/make :INSTALLDIR: PROBE.mk TEST.mk WWW.mk MSGFUN.mk MSGKEY.mk

/*
 * proto initialization
 */

$(INCLUDEDIR) :INSTALLDIR: prototyped.h

prototyped.h : $(BINDIR)/proto
	proto -f /dev/null > h.$(tmp).h
	if	$(CMP) -s h.$(tmp).h $(<)
	then	$(RM) -f h.$(tmp).h
	else	$(MV) h.$(tmp).h $(<)
	fi

/*
 * -l* library local conventions
 */

":MAPLIB:" : .MAKE .OPERATOR
	$(LIBDIR)/lib/$(<) :INSTALL: $(<).req
	$(<).req : (CC) $(>)
		r='-'
		for i in $(*)
		do	if	$(CC) -o $(<:B).exe $i -l$(<:B) > /dev/null
			then	$(CC) -o $(<:B).exe $i > /dev/null || {
					r='-l$(<:B)'
					break
				}
			fi
		done 2>/dev/null
		echo " $r" > $(<)
		rm -f $(<:B).exe $(<:B)$(CC.SUFFIX.OBJECT)

/*
 * check if -ldl is required
 * this allows makefiles to use -ldl on all systems
 *
 * NOTE: this works around the sgi botch:
 *	 (1) irix 5.* made -ldl optional but warned
 *	 (2) irix 6.* has no -ldl
 *	 (3) dynamic progs built on irix 5.* and using -ldl fail
 *           at runtime on irix 6.* because -ldl is not there
 */

dl :MAPLIB: dl.c

/*
 * miscellaneous -l* checks
 */

m :MAPLIB: m.c

nsl :MAPLIB: nsl.c

/*
 * what was sco smoking
 * almost all of gethost* are in -lnsl except gethostbyname which
 * is in -lsocket which isn't needed to resolve socket() but seems
 * to do the -lnsl job
 */

socket :MAPLIB: socket.c nsl.c

/*
 * proto.c is snarfed from the standalone src
 * this is a gross hack to allow only one copy of the source
 */

if ! "$(-mam)"

:SAVE: proto.c

.SOURCE.c : ../../lib/libpp

../proto/proto.c : .DONTCARE

proto.c : ../proto/proto.c
	if	grep '^#pragma prototyped' $(*) >/dev/null 2>/dev/null
	then	{
		echo "#define __PROTO__(x) x"
		echo "#define __MANGLE__"
		$(SED) \
		-e '/\/\*DELAY_CONTROL\*\//,/\/\*NODELAY_CONTROL\*\//s/^#/DELAY_CONTROL /' \
		-e '/\/\*DELAY_CONTROL\*\//s/^/#define DELAY_CONTROL # \
	#undef	__STDC__/' \
		-e '/\/\*NODELAY_CONTROL\*\//s/^/#undef	DELAY_CONTROL /' \
		$(*)
		} > c.$(tmp).c
		$(CPP) $(CPPFLAGS) -I$(INSTALLROOT)/include/ast -DPROTO_STANDALONE -D-F$(*) -D-L -I-D -E c.$(tmp).c | $(PROTO) $(PROTOFLAGS:N!=-s) -f > $(<)
		$(RM) -f c.$(tmp).c
	fi

end
