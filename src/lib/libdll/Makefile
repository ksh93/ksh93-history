:PACKAGE: ast

LICENSE = since=1997,author=gsf

_HOSTTYPE_ == "$(CC.HOSTTYPE)"

CCFLAGS = $(CC.OPTIMIZE) $(CC.DLL)

static = $(CC.SUFFIX.DYNAMIC:T!=V)

dll 1.0 :LIBRARY: dlfcn.c dllfind.c dllnext.c dlllook.c -ldl -ldld

$(INCLUDEDIR) :INSTALLPROTO: dlldefs.h

dlldefs.h :COPY: FEATURE/dll

:: RELEASE
