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

ignore :: ignore.sh

mamprobe :: mamprobe.sh

silent :: silent.sh

/*
 * these commands are built here for binary packages
 * the package command handles the $(BINDIR) install
 */

":PACKAGE_INIT:" : .MAKE .OPERATOR /* drop after release 2000-02-14 */
	local I B
	.LIST.PACKAGE.BINARY : .LIST.PACKAGE.INIT
	.LIST.PACKAGE.INIT : .MAKE
		local I
		for I $(*)
			print ;;;$(I:T=F:P=A);$(BINDIR)/$(I:B)
		end
	for I $(>)
		B := $(I:B)
		$(BINDIR) :INSTALLDIR: $(B)
			if	test ! -f $(<)
			then	cp $(*) $(<)
			fi
		$(B) :: $(I)
		.LIST.PACKAGE.INIT : $(B)
	end

:PACKAGE_INIT: mamake.c proto.c ratz.c release.c

:: RELEASE hosttype.tst p.c

/*
 * ksh93g function search on PATH
 */

$(BINDIR)/.fpath :INSTALL:
	echo ../fun > $(<)

/*
 * hosttype specific cc workaround initialization
 */

if ! CC.HOSTTYPE
	CC.HOSTTYPE := $(_hosttype_)
end

$(BINDIR)/cc :INSTALL: (CC.HOSTTYPE) package hello.c \
		cc.darwin.ppc ldd.darwin.ppc \
		cc.hp.pa ld.hp.pa \
		cc.mvs.390 \
		cc.next.i386 \
		cc.osf.alpha \
		cc.sco.i386 \
		cc.sgi.mips2 ldd.sgi.mips2 \
		cc.sgi.mips3 ldd.sgi.mips3 \
		cc.sgi.mips3-o32 \
		cc.sgi.mips4 ldd.sgi.mips4 \
		cc.sgi.mips4-n32
	set x $(*:O>2)
	while	:
	do	case $# in
		0)	break ;;
		esac
		cc=$1 # hp /bin/sh scripts modify parent args!
		shift
		argv=$*
		export cc argv
		case $cc in
		*cc.$(CC.HOSTTYPE))
			test -f $cc &&
			$cc -o hello.exe $(*:O=2) >/dev/null 2>&1 &&
			hello.exe >/dev/null 2>&1 && {
				cmp -s $cc $(<) ||
				cp $cc $(<)
				for i in $argv
				do	case $i in
					*ld.$(CC.HOSTTYPE))
						cmp -s $i $(<:D:B=ld) ||
						cp $i $(<:D:B=ld)
						;;
					*ldd.$(CC.HOSTTYPE))
						cmp -s $i $(<:D:B=ldd) ||
						cp $i $(<:D:B=ldd)
						;;
					*)	break
						;;
					esac
				done
			} ||
			: no $(CC.HOSTTYPE) cc workaround needed
			break
		esac
	done
	rm -f hello.exe hello$(CC.SUFFIX.OBJECT)
	case "$(CC.HOSTTYPE)" in
	sgi.mips3)
		if test ! -d $(INSTALLROOT)/lib32
		then	$(LN) -s lib $(INSTALLROOT)/lib32
		fi
		;;
	sgi.mips4)
		if test ! -d $(INSTALLROOT)/lib64
		then	$(LN) -s lib $(INSTALLROOT)/lib64
		fi
		;;
	esac

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

$(BINDIR)/proto : .TERMINAL .NULL

prototyped.h : $(BINDIR)/proto
	$(*) -f /dev/null > h.$(tmp).h
	if	$(CMP) -s h.$(tmp).h $(<)
	then	$(RM) -f h.$(tmp).h
	else	$(MV) h.$(tmp).h $(<)
	fi

/*
 * -l* library local conventions
 */

":MAPLIB:" : .MAKE .OPERATOR
	local L
	for L $(>)
		$(LIBDIR)/lib :INSTALLDIR: $(L)
		$(L) : (CC) $(L).c
			if	$(CC) -o $(<).exe $(*) -l$(<)
			then	if	$(CC) -o $(<).exe $(*)
				then	echo ' -'
				else	echo ' -l$(<)'
				fi
			else	echo ' -'
			fi 2> /dev/null > $(<)
			rm -f $(<).exe $(<)$(CC.SUFFIX.OBJECT)
	end

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

:MAPLIB: dl

/*
 * check if -lm is required
 * this allows makefiles to use -lm on all systems
 */

:MAPLIB: m

/*
 * proto.c is snarfed from the standalone src
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
		$(CC) $(CCFLAGS) -DPROTO_STANDALONE -D-F$(*) -D-L -I-D -E c.$(tmp).c | $(PROTO) $(PROTOFLAGS:N!=-s) -f > $(<)
		$(RM) -f c.$(tmp).c
	fi

end
