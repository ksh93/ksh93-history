/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1992-2000 AT&T Corp.                *
*        and it may only be used by you under license from         *
*                       AT&T Corp. ("AT&T")                        *
*         A copy of the Source Code Agreement is available         *
*                at the AT&T Internet web site URL                 *
*                                                                  *
*       http://www.research.att.com/sw/license/ast-open.html       *
*                                                                  *
*        If you have copied this software without agreeing         *
*        to the terms of the license you are infringing on         *
*           the license and copyright and are violating            *
*               AT&T's intellectual property rights.               *
*                                                                  *
*                 This software was created by the                 *
*                 Network Services Research Center                 *
*                        AT&T Labs Research                        *
*                         Florham Park NJ                          *
*                                                                  *
*               Glenn Fowler <gsf@research.att.com>                *
*                David Korn <dgk@research.att.com>                 *
*                                                                  *
*******************************************************************/
#pragma prototyped
/*
 * David Korn
 * Glenn Fowler
 * AT&T Research
 *
 * uname
 */

static const char usage[] =
"[-?\n@(#)uname (AT&T Labs Research) 1999-04-10\n]"
USAGE_LICENSE
"[+NAME?uname - identify the current system ]"
"[+DESCRIPTION?By default \buname\b writes the operating system name to"
"	standard output. When options are specified, one or more"
"	system characteristics are written to standard output. When more"
"	than one options is specifed the output is in the order specfied"
"	by the \b-l\b option below.]"
"[a:all?Equivalent to \b-mnrsv\b.]"
"[i:id?Writes the host id in hex.]"
"[l:list-all?Equivalent to \b-mnrsvpi\b.]"
"[m:machine?Writes the name of the hardware type the system is running on.]"
"[n:nodename?Writes hostname or nodename. This is the name by which the"
"	system is know to the communications network.]"
"[p:processor?Writes the name of the processor instruction set architecture.]"
"[r:release?Writes the release level of the operating system implementation.]"
"[s:os|sysname?Writes the name of the operating system. This is the default.]"
"[v:version?Writes the version level of the operating system implementation.]"
"[S:sethost?Set the hostname or nodename to \aname\a. No output is"
"	written to standard output.]:[name]"

"[+SEE ALSO?\bhostname\b(1), \bgetconf\b(1), \buname\b(2), \bsysconf\b(2)]"
;

#if defined(__STDPP__directive) && defined(__STDPP__hide)
__STDPP__directive pragma pp:hide getdomainname gethostid gethostname sethostname
#else
#define getdomainname	______getdomainname
#define gethostid	______gethostid
#define gethostname	______gethostname
#define sethostname	______sethostname
#endif

#include <cmdlib.h>

#include "FEATURE/utsname"

#define MAXHOSTNAME	64

#if _lib_uname && _sys_utsname

#include <sys/utsname.h>

#endif

#if _sys_systeminfo
#if !_lib_systeminfo && _lib_syscall && _sys_syscall
#include <sys/syscall.h>
#if defined(SYS_systeminfo)
#define _lib_systeminfo		1
#define systeminfo(a,b,c)	syscall(SYS_systeminfo,a,b,c)
#endif
#endif
#if _lib_systeminfo
#if !defined(SYS_NMLEN)
#define SYS_NMLEN	9
#endif
#include <sys/systeminfo.h>
#endif
#endif

#if defined(__STDPP__directive) && defined(__STDPP__hide)
__STDPP__directive pragma pp:nohide getdomainname gethostid gethostname sethostname
#else
#undef	getdomainname
#undef	gethostid
#undef	gethostname
#undef	sethostname
#endif

#if _lib_getdomainname
extern int	getdomainname(char*, size_t);
#endif
#if _lib_gethostid
extern int	gethostid(void);
#endif
#if _lib_gethostname
extern int	gethostname(char*, size_t);
#endif
#if _lib_sethostname
extern int	sethostname(const char*, size_t);
#endif

#ifndef HOSTTYPE
#define HOSTTYPE	"unknown"
#endif

static char	hosttype[] = HOSTTYPE;

#if !_lib_uname || !_sys_utsname

#if defined(__STDPP__)
#define SYSNAME		#(getprd machine)
#define RELEASE		#(getprd release)
#define VERSION		#(getprd version)
#define MACHINE		#(getprd architecture)
#else
#define SYSNAME		""
#define RELEASE		""
#define VERSION		""
#define MACHINE		""
#endif

struct utsname
{
	char*	sysname;
	char	nodename[MAXHOSTNAME];
	char*	release;
	char*	version;
	char*	machine;
};

int
uname(register struct utsname* ut)
{
#ifdef HOSTTYPE
	char*		sys = 0;
	char*		arch = 0;

	if (*hosttype)
	{
		sys = hosttype;
		if (arch = strchr(sys, '.'))
		{
			*arch++ = 0;
			if (!*arch) arch = 0;
		}
		if (!*sys) sys = 0;
	}
#endif
#ifdef _lib_gethostname
	if (gethostname(ut->nodename, sizeof(ut->nodename) - 1))
		return(-1);
#else
	strncpy(ut->nodename, "local", sizeof(ut->nodename) - 1);
#endif
#ifdef HOSTTYPE
	if (!(ut->sysname = sys))
#endif
	if (!*(ut->sysname = SYSNAME))
		ut->sysname = ut->nodename;
#ifdef HOSTTYPE
	if (!(ut->machine = arch))
#endif
	ut->machine = MACHINE;
	ut->release = RELEASE;
	ut->version = VERSION;
	return(0);
}

#endif

#define S_FLAG	(1<<0)
#define N_FLAG	(1<<1)
#define R_FLAG	(1<<2)
#define V_FLAG	(1<<3)
#define M_FLAG	(1<<4)

#define P_FLAG	(1<<5)
#define I_FLAG	(1<<6)
#define C_FLAG	(1<<7)
#define D_FLAG	(1<<8)
#define T_FLAG	(1<<9)
#define B_FLAG	(1<<10)

#define X_FLAG	(1<<11)

#define FLAGS	12

#define A_FLAGS	(all&(M_FLAG<<1)-1)
#define L_FLAGS	(all)

#define OPTBEG	"snrvm"
#define OPTEND	"alS:[nodename] "

#ifndef MACHINE
#if defined(__STDPP__)
#define MACHINE		#(getprd architecture)
#else
#define MACHINE		""
#endif
#endif

#ifndef HOSTTYPE
#define HOSTTYPE	"unknown"
#endif

static int	all = ((M_FLAG<<1)-1);

#define extra(m)        do{if((char*)&ut.m[sizeof(ut.m)]>last)last=(char*)&ut.m[sizeof(ut.m)];}while(0)
#define output(f,v,u)	((flags&(f))?sfputr(sfstdout,*(v)?(v):(sfsprintf(buf,sizeof(buf),"[%s]",u),buf),flags>=((f)<<1)?' ':'\n'):0)

int
b_uname(int argc, char** argv, void* context)
{
	register int	n;
	register int	flags = 0;
	register char*	s;
	char*		t;
	char*		sethost = 0;
	struct utsname	ut;
	char*		last = (char*)&ut.machine + sizeof(ut.machine);
	char		buf[257];

	static char	opts[sizeof(OPTBEG) + sizeof(OPTEND) + FLAGS] = OPTBEG;

	NoP(argc);
	cmdinit(argv, context, ERROR_CATALOG);
	s = &opts[sizeof(OPTBEG) - 1];
	*s++ = 'p';
	all |= P_FLAG;
#if _mem_idnumber_utsname || _lib_gethostid || defined(SI_HW_SERIAL)
	*s++ = 'i';
	all |= I_FLAG;
#if _mem_idnumber_utsname
	extra(idnumber);
#endif
#endif
#if defined(SI_HW_PROVIDER)
	*s++ = 'c';
	all |= C_FLAG;
#endif
#if _lib_getdomainname || defined(SI_SRPC_DOMAIN)
	*s++ = 'd';
	all |= D_FLAG;
#endif
#if _mem_m_type_utsname
	*s++ = 't';
	all |= T_FLAG;
	extra(m_type);
#endif
#if _mem_base_rel_utsname
	*s++ = 'b';
	all |= B_FLAG;
	extra(base_rel);
#endif
#if _lib_uname && _sys_utsname
	if (last < ((char*)(&ut + 1)))
	{
		*s++ = 'x';
		all |= X_FLAG;
	}
#endif
	strcpy(s, OPTEND);
	while (n = optget(argv, usage)) switch (n)
	{
	case 'a':
		flags |= A_FLAGS;
		break;
#if _mem_base_rel_utsname
	case 'b':
		flags |= B_FLAG;
		break;
#endif
#if defined(SI_HW_PROVIDER)
	case 'c':
		flags |= C_FLAG;
		break;
#endif
#if _lib_getdomainname || defined(SI_SRPC_DOMAIN)
	case 'd':
		flags |= D_FLAG;
		break;
#endif
#if _mem_idnumber_utsname || _lib_gethostid || defined(SI_HW_SERIAL)
	case 'i':
		flags |= I_FLAG;
		break;
#endif
	case 'l':
		flags |= L_FLAGS;
		break;
	case 'm':
		flags |= M_FLAG;
		break;
	case 'n':
		flags |= N_FLAG;
		break;
	case 'p':
		flags |= P_FLAG;
		break;
	case 'r':
		flags |= R_FLAG;
		break;
	case 's':
		flags |= S_FLAG;
		break;
#if _mem_m_type_utsname
	case 't':
		flags |= T_FLAG;
		break;
#endif
	case 'v':
		flags |= V_FLAG;
		break;
	case 'x':
		flags |= X_FLAG;
		break;
	case 'S':
		sethost = opt_info.arg;
		break;
	case ':':
		error(2, "%s", opt_info.arg);
		break;
	case '?':
		error(ERROR_usage(2), "%s", opt_info.arg);
		break;
	}
	argv += opt_info.index;
	if (error_info.errors || *argv || sethost && flags)
		error(ERROR_usage(2), "%s", optusage(NiL));
	if (sethost)
	{
#if _lib_sethostname
		if (sethostname(sethost, strlen(sethost) + 1))
#else
#ifdef	ENOSYS
		errno = ENOSYS;
#else
		errno = EPERM;
#endif
#endif
		error(ERROR_system(1), "%s: cannot set host name", sethost);
	}
	else
	{
		s = buf;
		if (!flags)
			flags = S_FLAG;
		memzero(&ut, sizeof(ut));
		if (uname(&ut) < 0)
			error(ERROR_usage(2), "information unavailable");
		output(S_FLAG, ut.sysname, "sysname");
#if !_mem_nodeext_utsname && _lib_gethostname
		if ((flags & N_FLAG) && sizeof(ut.nodename) <= 9 && !gethostname(s, sizeof(buf)))
			output(N_FLAG, s, "nodename");
		else
#endif
		output(N_FLAG, ut.nodename, "nodename");
		output(R_FLAG, ut.release, "release");
		output(V_FLAG, ut.version, "version");
		output(M_FLAG, ut.machine, "machine");
		if (flags & P_FLAG)
		{
#if defined(SI_ARCHITECTURE)
			if ((n = systeminfo(SI_ARCHITECTURE, s, sizeof(buf) - 1)) > 0)
				s[n] = 0;
			else
#endif
			{
				if (t = strchr(hosttype, '.'))
					t++;
				else
					t = hosttype;
				strncpy(s, t, sizeof(buf) - 1);
			}
			output(P_FLAG, s, "processor");
		}
#if _mem_idnumber_utsname
		output(I_FLAG, ut.idnumber, "id");
#else
#if _lib_gethostid || defined(SI_HW_SERIAL)
		if (flags & I_FLAG)
		{
#if _lib_gethostid
			sfsprintf(s, sizeof(buf), "%08x", gethostid());
#else
			if ((n = systeminfo(SI_HW_SERIAL, s, sizeof(buf) - 1)) > 0) s[n] = 0;
			else *s = 0;
#endif
			output(I_FLAG, s, "id");
		}
#endif
#endif
#if defined(SI_HW_PROVIDER)
		if (flags & C_FLAG)
		{
			if ((n = systeminfo(SI_HW_PROVIDER, s, sizeof(buf) - 1)) > 0) s[n] = 0;
			else *s = 0;
			output(C_FLAG, s, "vendor");
		}
#endif
#if _lib_getdomainname || defined(SI_SRPC_DOMAIN)
		if (flags & D_FLAG)
		{
#if _lib_getdomainname
			if (getdomainname(s, sizeof(buf)) < 0)
				*s = 0;
#else
			if ((n = systeminfo(SI_SRPC_DOMAIN, s, sizeof(buf) - 1)) > 0) s[n] = 0;
			else *s = 0;
#endif
			output(D_FLAG, s, "domain");
		}
#endif
#if _mem_m_type_utsname
		output(T_FLAG, ut.m_type, "m_type");
#endif
#if _mem_base_rel_utsname
		output(B_FLAG, ut.base_rel, "base_rel");
#endif
		if (flags & X_FLAG)
		{
			char*	b;
			int	sep = 0;

			s = b = last;
			while (s < (char*)(&ut + 1))
			{
				if (!(n = *s++))
				{
					if ((s - b) > 1)
					{
						if (sep) sfputc(sfstdout, ' ');
						else sep = 1;
						sfputr(sfstdout, b, -1);
					}
					b = s;
				}
				else if (n < 0x20 || n >= 0x7E) break;
			}
			sfputc(sfstdout, '\n');
		}
	}
	return(error_info.errors);
}
