/*
 * coshell library
 */

:PACKAGE: ast

LICENSE = since=1990,author=gsf

coshell 1.0 :LIBRARY: RELEASE coshell.3 coshell.h colib.h \
		coopen.c coclose.c coinit.c coexec.c \
		cowait.c cokill.c cosync.c coquote.c codata.c \
		procrun.c system.c

ignore :: ignore.sh

silent :: silent.sh

$(INCLUDEDIR) :INSTALLPROTO: coshell.h
