/***************************************************************
*                                                              *
*           This software is part of the ast package           *
*              Copyright (c) 1982-2000 AT&T Corp.              *
*      and it may only be used by you under license from       *
*                     AT&T Corp. ("AT&T")                      *
*       A copy of the Source Code Agreement is available       *
*              at the AT&T Internet web site URL               *
*                                                              *
*     http://www.research.att.com/sw/license/ast-open.html     *
*                                                              *
*     If you received this software without first entering     *
*       into a license with AT&T, you have an infringing       *
*           copy and cannot use it without violating           *
*             AT&T's intellectual property rights.             *
*                                                              *
*               This software was created by the               *
*               Network Services Research Center               *
*                      AT&T Labs Research                      *
*                       Florham Park NJ                        *
*                                                              *
*              David Korn <dgk@research.att.com>               *
*                                                              *
***************************************************************/
#pragma prototyped
/*
 * UNIX shell
 *
 * S. R. Bourne
 * Rewritten by David Korn
 * AT&T Labs
 *
 */

#include	"defs.h"
#include	<fcin.h>
#include	<ls.h>
#include	"variables.h"
#include	"path.h"
#include	"io.h"
#include	"jobs.h"
#include	"history.h"
#include	"test.h"
#include	"FEATURE/externs"

#define RW_ALL	(S_IRUSR|S_IRGRP|S_IROTH|S_IWUSR|S_IWGRP|S_IWOTH)

struct edata
{
	Shell_t		*sh;
	char		**envp;
	char		*libpath;
	int		exec_err;
};


static const char	usrbin[] = "/usr/bin";
static char		*prune(char*, const char*);
static char		*execs(const char*, const char*, char**, struct edata*);
static int		canexecute(char*,int);
static int		path_inpath(const char*,const char*);
static void		funload(int,const char*);
static void		exscript(char*, char*[], char**);
static int		name_offset;

static Namval_t		*tracknod;
static int		pruned;

/*
 * make sure PWD is set up correctly
 * Return the present working directory
 * Invokes getcwd() if flag==0 and if necessary
 * Sets the PWD variable to this value
 */
char *path_pwd(int flag)
{
	register char *cp;
	register char *dfault = (char*)e_dot;
	register int count = 0;
	if(sh.pwd)
		return((char*)sh.pwd);
	while(1) 
	{
		/* try from lowest to highest */
		switch(count++)
		{
			case 0:
				cp = nv_getval(PWDNOD);
				break;
			case 1:
				cp = nv_getval(HOME);
				break;
			case 2:
				cp = "/";
				break;
			case 3:
				cp = (char*)e_crondir;
				if(flag) /* skip next case when non-zero flag */
					++count;
				break;
			case 4:
			{
				if(cp=getcwd(NIL(char*),0))
				{  
					nv_offattr(PWDNOD,NV_NOFREE);
					nv_unset(PWDNOD);
					PWDNOD->nvalue.cp = cp;
					goto skip;
				}
				break;
			}
			case 5:
				return(dfault);
		}
		if(cp && *cp=='/' && test_inode(cp,e_dot))
			break;
	}
	if(count>1)
	{
		nv_offattr(PWDNOD,NV_NOFREE);
		nv_putval(PWDNOD,cp,NV_RDONLY);
	}
skip:
	nv_onattr(PWDNOD,NV_NOFREE|NV_EXPORT);
	sh.pwd = (char*)(PWDNOD->nvalue.cp);
	return(cp);
}

/*
 * returns true if directory already appears in path
 */
static int dir_in_path(const char *dir, int dlen,  const char *path, int plen)
{
	const char *cp = path, *pend=path+plen;
	if(*cp==':')
		cp++;
	while(cp+dlen <= pend)
	{
		if(memcmp(dir,cp,dlen)==0 && (cp+dlen==pend || cp[dlen]==':'))
			return(1);
		while(cp<pend && *cp++!=':');
	}
	return(0);
}

/*
 * copies path to stack eliminating duplicates
 * if mode==1, only one of /bin or /usr/bin will be copied
 */
static char *path_to_stak(register const char *path, int mode)
{
	register const char *cp=path, *base;
	register int n, len;
	register int dot=0;
	stakseek(0);
	while(*cp)
	{
		if (*cp==':'  || (*cp=='.' && (cp[1]==':' || cp[1]==0)) )
		{
			if(((cp++==path) || *cp==':') && !dot++)
			{
				stakputc('.');
				stakputc(':');
			}
			while(*cp==':')
				cp++;
		}
		else
		{
			base = cp;
			while((n= *cp) && n!=':')
				cp++;
			if((n=cp-base)>0 && (len=staktell()))
			{
				if(dir_in_path(base,n,path=stakptr(0),len))
					continue;
				if(mode && n==8 && memcmp(usrbin,base,8)==0 && dir_in_path(usrbin+4,4,path,len))
					continue;
				if(mode && n==4 && memcmp(usrbin+4,base,4)==0 && dir_in_path(usrbin,8,path,len))
					continue;
			}
			stakwrite(base,n+1);
		}
	}
	return(stakfreeze(1));
}

/*
 * given <s> return a colon separated list of directories to search on the stack
 * This routine adds names to the tracked alias list, if possible, and returns
 * a reduced path string for tracked aliases
 */
char *path_get(const char *name)
/*@
 	assume name!=NULL;
	return path satisfying path!=NULL;
@*/
{
	register char *path=0;
	register char *sp = sh.lastpath;
	static int bin_is_usrbin = -1;
	if(strchr(name,'/'))
		return("");
	if(!sh_isstate(SH_DEFPATH))
		path = nv_getval(nv_scoped(PATHNOD));
	if(!path)
		path = (char*)e_defpath;
	if(bin_is_usrbin < 0)
		bin_is_usrbin = test_inode(usrbin+4,usrbin);
	path = path_to_stak(path,bin_is_usrbin); 
	if(sh_isstate(SH_DEFPATH))
		return(path);
	if(sp || *name && ((tracknod=nv_search(name,sh.track_tree,0)) &&
		nv_isattr(tracknod,NV_NOALIAS)==0 &&
		(sp=nv_getval(tracknod))))
	{
		path = prune(path,sp);
		pruned++;
	}
	return(path);
}

int	path_open(const char *name,char *path)
/*@
	assume name!=NULL;
 @*/
{
	register int fd;
	struct stat statb;
	if(strchr(name,'/'))
	{
		if(sh_isoption(SH_RESTRICTED))
			errormsg(SH_DICT,ERROR_exit(1),e_restricted,name);
	}
	else
	{
		if(!path)
			path = (char*)e_defpath;
		path = stakcopy(path);
	}
	do
	{
		path=path_join(path,name);
		if((fd = sh_open(path_relative(stakptr(PATH_OFFSET)),O_RDONLY,0)) >= 0)
		{
			if(fstat(fd,&statb)<0 || S_ISDIR(statb.st_mode))
			{
				errno = EISDIR;
				sh_close(fd);
				fd = -1;
			}
		}
	}
	while( fd<0 && path);
	if((fd = sh_iomovefd(fd)) > 0)
	{
		fcntl(fd,F_SETFD,FD_CLOEXEC);
		sh.fdstatus[fd] |= IOCLEX;
	}
	return(fd);
}

/*
 * This routine returns 1 if first directory in <path> is also in <fpath>
 * If <path> begins with :, or first directory is ., $PWD must be in <fpath>
 * Otherwise, 0 is returned
 */

static int path_inpath(const char *path, const char *fpath)
{
	register const char *dp = fpath;
	register const char *sp = path;
	register int c, match=1;
	if(!dp || !sp || *sp==0)
		return(0);
	if(*sp==':' || (*sp=='.' && ((c=sp[1])==0 || c==':')))
		sp = path = path_pwd(1);
	while(1)
	{
		if((c= *dp++)==0 || c == ':')
		{
			if(match==1 && sp>path && (*sp==0 || *sp==':'))
				return(1);
			if(c==0)
				return(0);
			match = 1;
			sp = path;
		}
		else if(match)
		{
			if(*sp++ != c)
				match = 0;
		}
	}
	/* NOTREACHED */
}

/*
 *  set tracked alias node <np> to value <sp>
 */

void path_alias(register Namval_t *np,register char *sp)
/*@
	assume np!=NULL;
@*/
{
	if(!sp)
		nv_unset(np);
	else
	{
		register const char *vp = np->nvalue.cp;
		register int n = 1;
		register int nofree = nv_isattr(np,NV_NOFREE);
		struct stat statb;
		nv_offattr(np,NV_NOPRINT);
		if(!vp || (n=strcmp(sp,vp))!=0)
		{
			int subshell = sh.subshell;
			sh.subshell = 0;
			nv_putval(np,sp,0);
			sh.subshell = subshell;
		}
		nv_setattr(np,NV_TAGGED|NV_EXPORT);
		if(sp && lstat(sp,&statb)>=0 && S_ISLNK(statb.st_mode))
			nv_setsize(np,statb.st_size+1);
		else
			nv_setsize(np,0);
		if(nofree && n==0)
			nv_onattr(np,NV_NOFREE);
	}
}


/*
 * given a pathname return the base name
 */

char	*path_basename(register const char *name)
/*@
	assume name!=NULL;
	return x satisfying x>=name && *x!='/';
 @*/
{
	register const char *start = name;
	while (*name)
		if ((*name++ == '/') && *name)	/* don't trim trailing / */
			start = name;
	return ((char*)start);
}

/*
 * load functions from file <fno>
 */
static void funload(int fno, const char *name)
{
	char buff[IOBSIZE+1];
	int savestates = sh_isstate(~0);
	sh_onstate(SH_NOLOG|SH_NOALIAS);
	sh.readscript = (char*)name;
	sh_eval(sfnew(NIL(Sfio_t*),buff,IOBSIZE,fno,SF_READ),0);
	sh.readscript = 0;
	sh_setstate(savestates);
}

/*
 * do a path search and track alias if requested
 * if flag is 0, or if name not found, then try autoloading function
 * if flag==2, returns 1 if name found on FPATH
 * returns 1, if function was autoloaded.
 * If endpath!=NULL, Path search ends when path matches endpath.
 */

int	path_search(register const char *name,const char *endpath, int flag)
/*@
	assume name!=NULL;
	assume flag==0 || flag==1 || flag==2;
@*/
{
	register Namval_t *np;
	register int fno;
	if(flag)
	{
		/* if not found on pruned path, rehash and try again */
		while(!(sh.lastpath=path_absolute(name,endpath)) && pruned)
			nv_onattr(tracknod,NV_NOALIAS);
		if(!sh.lastpath && (np=nv_search(name,sh.fun_tree,HASH_NOSCOPE))&&np->nvalue.ip)
			return(1);
		np = 0;
	}
	if(flag==0 || !sh.lastpath)
	{
		register char *path=0;
		if(strmatch(name,e_alphanum))
			path = nv_getval(nv_scoped(FPATHNOD));
		if(path && (fno=path_open(name,path))>=0)
		{
			if(flag==2)
			{
				sh_close(fno);
				return(1);
			}
			funload(fno,name);
			if((np=nv_search(name,sh.fun_tree,HASH_NOSCOPE))&&np->nvalue.ip)
				return(1);
		}
		return(0);
	}
	else if(!sh_isstate(SH_DEFPATH) && *name!='/')
	{
		if((np=tracknod) || ((endpath||sh_isoption(SH_TRACKALL)) &&
			(np=nv_search(name,sh.track_tree,NV_ADD)))
		  )
			path_alias(np,sh.lastpath);
	}
	return(0);
}


/*
 * do a path search and find the full pathname of file name
 * end search of path matches endpath without checking execute permission
 */

char	*path_absolute(register const char *name, const char *endpath)
/*@
	assume name!=NULL;
	return x satisfying x && *x=='/';
@*/
{
	register int	f;
	register char *path;
	register const char *fpath=0;
	register int isfun,len;
	Namval_t *np;
	pruned = 0;
	path = path_get(name);
	if(*path && strmatch(name,e_alphanum))
		fpath = nv_getval(nv_scoped(FPATHNOD));
	while(1)
	{
		sh_sigcheck();
		isfun = (fpath && path_inpath(path,fpath));
		path=path_join(path,name);
		if(endpath && strcmp(endpath,stakptr(PATH_OFFSET))==0)
			return((char*)endpath);
		f = canexecute(stakptr(PATH_OFFSET),isfun);
		if(isfun && f>=0)
		{
			funload(f,stakptr(PATH_OFFSET));
			f = -1;
			path = 0;
			break;
		}
		if(f>=0)
			break;
		if((len=name_offset-PATH_OFFSET)==0)
			goto skip;
		*stakptr(name_offset-1) = 0;
		np = nv_search(stakptr(PATH_OFFSET),sh.track_tree,NV_ADD);
		*stakptr(name_offset-1) = '/';
		/* skip .fpath check if already checked */
		if(np && (nv_isattr(np,NV_NOPRINT|NV_NOALIAS)==NV_NOPRINT))
			goto skip;
		nv_offattr(np,NV_NOALIAS);
		nv_onattr(np,NV_NOPRINT|NV_TAGGED);
		if(np->nvenv)
		{
			stakseek(name_offset);
			stakputs(np->nvenv);
			stakputc('/');
		}
		else
		{
			/* make sure there is room for .fpath */
			if(staktell()-name_offset<6)
				stakseek(name_offset+6);
			strcpy(stakptr(name_offset),".fpath");
			if((f=open(stakptr(PATH_OFFSET),O_RDONLY))>=0)
			{
				struct stat statb;
				fstat(f,&statb);
				len = statb.st_size;
				stakseek(name_offset+len);
				read(f,stakptr(name_offset),len);
				*stakptr(name_offset+len-1) = 0;
				np->nvenv = strdup(stakptr(name_offset));
				*stakptr(name_offset+len-1) = '/';
				close(f);
			}
			else
				goto skip;
		}
		nv_onattr(np,NV_NOALIAS);
		stakputs(name);
		f = canexecute(stakptr(PATH_OFFSET),1);
		if(f>=0)
		{
			funload(f,stakptr(PATH_OFFSET));
			f = -1;
			path = 0;
		}
	skip:
		if(!path)
			break;
	}
	if(f<0)
		return(0);
	/* check for relative pathname */
	if(*stakptr(PATH_OFFSET)!='/')
		path_join(path_pwd(1),(char*)stakfreeze(1)+PATH_OFFSET);
	return((char*)stakfreeze(1)+PATH_OFFSET);
}

/*
 * returns 0 if path can execute
 * sets exec_err if file is found but can't be executable
 */
#undef S_IXALL
#ifdef S_IXUSR
#   define S_IXALL	(S_IXUSR|S_IXGRP|S_IXOTH)
#else
#   ifdef S_IEXEC
#	define S_IXALL	(S_IEXEC|(S_IEXEC>>3)|(S_IEXEC>>6))
#   else
#	define S_IXALL	0111
#   endif /*S_EXEC */
#endif /* S_IXUSR */

static int canexecute(register char *path, int isfun)
/*@
	assume path!=NULL;
@*/
{
	struct stat statb;
	register int fd=0;
	path = path_relative(path);
	if(isfun)
	{
		if((fd=open(path,O_RDONLY,0))<0 || fstat(fd,&statb)<0)
			goto err;
	}
	else if(stat(path,&statb) < 0)
	{
#ifdef _UWIN
		/* check for .exe or .bat suffix */
		char *cp;
		if(errno==ENOENT && (!(cp=strrchr(path,'.')) || strlen(cp)>4 || strchr(cp,'/')))
		{
			int offset = staktell();
			stakputs(".bat");
			path = stakptr(PATH_OFFSET);
			if(stat(path,&statb) < 0)
			{
				if(errno!=ENOENT)
					goto err;
				memcpy(stakptr(offset),".sh",4);
				if(stat(path,&statb) < 0)
					goto err;
			}
		}
		else
#endif /* _UWIN */
		goto err;
	}
	errno = EPERM;
	if(S_ISDIR(statb.st_mode))
		errno = EISDIR;
	else if((statb.st_mode&S_IXALL)==S_IXALL || sh_access(path,X_OK)>=0)
		return(fd);
	if(isfun && fd>=0)
		sh_close(fd);
err:
	return(-1);
}

/*
 * Return path relative to present working directory
 */

char *path_relative(register const char* file)
/*@
	assume file!=NULL;
	return x satisfying x!=NULL;
@*/
{
	register const char *pwd;
	register const char *fp = file;
	/* can't relpath when sh.pwd not set */
	if(!(pwd=sh.pwd))
		return((char*)fp);
	while(*pwd==*fp)
	{
		if(*pwd++==0)
			return((char*)e_dot);
		fp++;
	}
	if(*pwd==0 && *fp == '/')
	{
		while(*++fp=='/');
		if(*fp)
			return((char*)fp);
		return((char*)e_dot);
	}
	return((char*)file);
}

char *path_join(register char *path,const char *name)
/*@
	assume path!=NULL;
	assume name!=NULL;
@*/
{
	/* leaves result on top of stack */
	register char *scanp = path;
	register int c;
	stakseek(PATH_OFFSET);
	if(*scanp=='.')
	{
		if((c= *++scanp)==0 || c==':')
			path = scanp;
		else if(c=='/')
			path = ++scanp;
		else
			scanp--;
	}
	while(*scanp && *scanp!=':')
		stakputc(*scanp++);
	if(scanp!=path)
	{
		stakputc('/');
		/* position past ":" unless a trailing colon after pathname */
		if(*scanp && *++scanp==0)
			scanp--;
	}
	else
		while(*scanp == ':')
			scanp++;
	path=(*scanp ? scanp : 0);
	name_offset = staktell();
	stakputs(name);
	return((char*)path);
}


void	path_exec(register const char *arg0,register char *argv[],struct argnod *local)
/*@
	assume arg0!=NULL argv!=NULL && *argv!=NULL;
@*/
{
	register const char *path = "";
	struct edata data;
	nv_setlist(local,NV_EXPORT|NV_IDENT|NV_ASSIGN);
	data.sh = &sh;
	data.envp=sh_envgen();
	if(strchr(arg0,'/'))
	{
		/* name containing / not allowed for restricted shell */
		if(sh_isoption(SH_RESTRICTED))
			errormsg(SH_DICT,ERROR_exit(1),e_restricted,arg0);
	}
	else
		path=path_get(arg0);
	/* leave room for inserting _= pathname in environment */
	data.envp--;
	data.exec_err = ENOENT;
	sfsync(NIL(Sfio_t*));
	timerdel(NIL(void*));
	data.libpath = astconf("LIBPATH",NIL(char*),NIL(char*));
	while(path=execs(path,arg0,argv,&data));
	/* force an exit */
	((struct checkpt*)sh.jmplist)->mode = SH_JMPEXIT;
	if((errno = data.exec_err)==ENOENT)
		errormsg(SH_DICT,ERROR_exit(ERROR_NOENT),e_found,arg0);
	else
		errormsg(SH_DICT,ERROR_system(ERROR_NOEXEC),e_exec,arg0);
}

/*
 * This routine constructs a short path consisting of all
 * Relative directories up to the directory of fullname <name>
 */
static char *prune(register char *path,const char *fullname)
/*@
	assume path!=NULL;
	return x satisfying x!=NULL && strlen(x)<=strlen(in path);
@*/
{
	register char *p = path;
	register char *s;
	int n = 1; 
	const char *base;
	char *inpath = path;
	if(!fullname  || *fullname != '/' || *path==0)
		return(path);
	base = path_basename(fullname);
	do
	{
		/* a null path means current directory */
		if(*path == ':')
		{
			*p++ = ':';
			path++;
			continue;
		}
		s = path;
		path=path_join(path,base);
		if(*s != '/' || (n=strcmp(stakptr(PATH_OFFSET),fullname))==0)
		{
			/* position p past end of path */
			while(*s && *s!=':')
				*p++ = *s++;
			if(n==0 || !path)
			{
				*p = 0;
				return(inpath);
			}
			*p++ = ':';
		}
	}
	while(path);
	/* if there is no match just return path */
	path = nv_getval(nv_scoped(PATHNOD));
	if(!path)
		path = (char*)e_defpath;
	strcpy(inpath,path);
	return(inpath);
}

static char *execs(const char *ap,const char *arg0,register char **argv, struct edata *dp)
/*@
	assume ap!=NULL;
	assume argv!=NULL && *argv!=NULL;
@*/
{
	
	register char *path, *prefix, *opath=0;
	int offset;
	sh_sigcheck();
	prefix=path_join((char*)ap,arg0);
	if (dp->libpath)
	{
		register int	c;
		const char*	dir;
		const char*	var;
		int		dirlen;
		int		varlen;
		int		n;
		int		r;
		char*		ep;
		char*		pp;
		Namval_t*	vp;

		/* save original pathname */
		opath = stakfreeze(1)+PATH_OFFSET;
		stakseek(PATH_OFFSET);
		stakputs(opath);
		path = stakptr(PATH_OFFSET);
		ep = strrchr(path,'/');
		vp=nv_search(arg0,sh.track_tree,0);
#if _lib_readlink
		if(!vp || nv_size(vp)>0)
		{
			/* check for symlink and use symlink name */
			char buff[PATH_MAX+1];
			char save[PATH_MAX+1];
			while((c=readlink(path,buff,PATH_MAX))>0)
			{
				buff[c] = 0;
				c = PATH_OFFSET;
				if(ep && *buff!='/')
				{
					if(buff[0]=='.' && buff[1]=='.' && (r = strlen(path) + 1) <= PATH_MAX)
						memcpy(save, path, r);
					else
						r = 0;
					c += (ep+1-path);
				}
				stakseek(c);
				stakputs(buff);
				stakputc(0);
				path = stakptr(PATH_OFFSET);
				if(buff[0]=='.' && buff[1]=='.')
				{
					pathcanon(path, 0);
					if(r && access(path,X_OK))
					{
						memcpy(path, save, r);
						break;
					}
				}
				ep = strrchr(path,'/');
			}
		}
#endif
		if(ep &&  (ep - path) > 4)
		{
			stakputc(0);
			do
			{
				n = ep - path;
				for (var = dp->libpath; (c = *dp->libpath) != 0 && c != ':' && c != ','; dp->libpath++);
				if (c == ':')
				{
					if ((dirlen = dp->libpath++ - var) < 0)
						break;
					dir = var;
					for (var = dp->libpath; (c = *dp->libpath) != 0 && c != ','; dp->libpath++);
				}
				else
					dirlen = 0;
				if ((varlen = dp->libpath++ - var) <= 0)
					break;
				offset = staktell();
				stakwrite(var, varlen);
				stakputc(0);
				vp = nv_open(stakptr(offset), sh.var_tree, 0);
				stakseek(offset);
				if ((pp = nv_getval(vp)) && *pp == 0)
					pp = 0;
				if (dirlen > 0 && n >= 3 && ep[-1] == 'n' && ep[-2] == 'i' && ep[-3] == 'b')
				{
					n -= 3;
					stakwrite(path, n);
					stakwrite(dir, dirlen);
					stakputc(0);
					r = access(stakptr(offset), 0);
					stakseek(offset);
					if (r < 0)
					{
						if (c != 0)
							continue;
						break;
					}
					if (pp == 0 || strncmp(pp, path, n) || memcmp(pp + n, dir, dirlen) || pp[n +  dirlen] != ':' && pp[n + dirlen] != 0)
					{
					prepend:
						stakputc(0);
						offset = staktell();
						stakputs(nv_name(vp));
						stakputc('=');
						stakwrite(path, n);
						if (dirlen > 0)
							stakwrite(dir, dirlen);
						if (pp)
						{
							stakputc(':');
							stakputs(pp);
						}
						stakputc(0);
						*dp->envp-- = stakptr(offset);
					}
					break;
				}
				else if (pp == 0 || strncmp(pp, path, n) || pp[n] != ':' && pp[n] != 0)
				{
					dirlen = 0;
					goto prepend;
				}
			} while (c != 0);
		}
	}
	dp->envp[0] =  stakptr(0);
	dp->envp[0][0] =  '_';
	dp->envp[0][1] =  '=';
	sfsync(sfstderr);
	sh_sigcheck();
	if(!opath)
		opath = stakptr(PATH_OFFSET);
	path = path_relative(opath);
#ifdef SHELLMAGIC
	if(*path!='/' && path!=opath)
	{
		/*
		 * The following code because execv(foo,) and execv(./foo,)
		 * may not yield the same results
		 */
		char *sp = (char*)malloc(strlen(path)+3);
		sp[0] = '.';
		sp[1] = '/';
		strcpy(sp+2,path);
		path = sp;
	}
#endif /* SHELLMAGIC */
	execve(path, &argv[0] ,dp->envp);
#ifdef SHELLMAGIC
	if(*path=='.' && path!=opath)
	{
		free(path);
		path = path_relative(opath);
	}
#endif /* SHELLMAGIC */
	switch(errno)
	{
#ifdef apollo
	    /* 
  	     * On apollo's execve will fail with eacces when
	     * file has execute but not read permissions. So,
	     * for now we will pretend that EACCES and ENOEXEC
 	     * mean the same thing.
 	     */
	    case EACCES:
#endif /* apollo */
	    case ENOEXEC:
#ifdef SHOPT_SUID_EXEC
	    case EPERM:
		/* some systems return EPERM if setuid bit is on */
#endif
		exscript(path,argv,dp->envp);
#ifndef apollo
	    case EACCES:
	    {
		struct stat statb;
		if(stat(path,&statb)>=0 && S_ISDIR(statb.st_mode))
			errno = EISDIR;
	    }
		/* FALL THROUGH */
#endif /* !apollo */
#ifdef ENAMETOOLONG
	    case ENAMETOOLONG:
#endif /* ENAMETOOLONG */
#ifndef SHOPT_SUID_EXEC
	    case EPERM:
		dp->exec_err = errno;
#endif
	    case ENOTDIR:
	    case ENOENT:
	    case EINTR:
#ifdef EMLINK
	    case EMLINK:
#endif /* EMLINK */
		return(prefix);
	    default:
		errormsg(SH_DICT,ERROR_system(ERROR_NOEXEC),e_exec,path);
	}
	return 0;
}

/*
 * File is executable but not machine code.
 * Assume file is a Shell script and execute it.
 */


static void exscript(register char *path,register char *argv[],char **envp)
/*@
	assume path!=NULL;
	assume argv!=NULL && *argv!=NULL;
@*/
{
	register Sfio_t *sp;
	sh.comdiv=0;
	sh.bckpid = 0;
	sh.st.ioset=0;
	/* clean up any cooperating processes */
	if(sh.cpipe[0]>0)
		sh_pclose(sh.cpipe);
	if(sh.cpid)
		sh_close(*sh.outpipe);
	sh.cpid = 0;
	if(sp=fcfile())
		while(sfstack(sp,SF_POPSTACK));
	job_clear();
	if(sh.infd>0)
		sh_close(sh.infd);
	sh_setstate(SH_FORKED);
	sfsync(sfstderr);
#ifdef SHOPT_SUID_EXEC
	/* check if file cannot open for read or script is setuid/setgid  */
	{
		static char name[] = "/tmp/euidXXXXXXXXXX";
		register int n;
		register uid_t euserid;
		char *savet;
		struct stat statb;
		if((n=sh_open(path,O_RDONLY,0)) >= 0)
		{
			/* move <n> if n=0,1,2 */
			n = sh_iomovefd(n);
			if(fstat(n,&statb)>=0 && !(statb.st_mode&(S_ISUID|S_ISGID)))
				goto openok;
			sh_close(n);
		}
		if((euserid=geteuid()) != sh.userid)
		{
			strncpy(name+9,fmtbase((long)getpid(),10,0),sizeof(name)-10);
			/* create a suid open file with owner equal effective uid */
			if((n=open(name,O_CREAT|O_TRUNC|O_WRONLY,S_ISUID|S_IXUSR)) < 0)
				goto fail;
			unlink(name);
			/* make sure that file has right owner */
			if(fstat(n,&statb)<0 || statb.st_uid != euserid)
				goto fail;
			if(n!=10)
			{
				sh_close(10);
				fcntl(n, F_DUPFD, 10);
				sh_close(n);
				n=10;
			}
		}
		savet = *--argv;
		*argv = path;
		execve(e_suidexec,argv,envp);
	fail:
		/*
		 *  The following code is just for compatibility
		 */
		if((n=open(path,O_RDONLY,0)) < 0)
			errormsg(SH_DICT,ERROR_system(1),e_open,path);
		*argv++ = savet;
	openok:
		sh.infd = n;
	}
#else
	sh.infd = sh_chkopen(path);
#endif /* SHOPT_SUID_EXEC */
#ifdef SHOPT_ACCT
	sh_accbegin(path) ;  /* reset accounting */
#endif	/* SHOPT_ACCT */
	sh.arglist = sh_argcreate(argv);
	sh.lastarg = strdup(path);
	/* save name of calling command */
	sh.readscript = error_info.id;
	/* close history file if name has changed */
	if(sh.hist_ptr && (path=nv_getval(HISTFILE)) && strcmp(path,sh.hist_ptr->histname))
	{
		hist_close(sh.hist_ptr);
		(HISTCUR)->nvalue.lp = 0;
	}
	sh_offstate(SH_FORKED);
	siglongjmp(*sh.jmplist,SH_JMPSCRIPT);
}

#ifdef SHOPT_ACCT
#   include <sys/acct.h>
#   include "FEATURE/time"

    static struct acct sabuf;
    static struct tms buffer;
    static clock_t	before;
    static char *SHACCT; /* set to value of SHACCT environment variable */
    static shaccton;	/* non-zero causes accounting record to be written */
    static int compress(time_t);
    /*
     *	initialize accounting, i.e., see if SHACCT variable set
     */
    void sh_accinit(void)
    {
	SHACCT = getenv("SHACCT");
    }
    /*
    * suspend accounting unitl turned on by sh_accbegin()
    */
    void sh_accsusp(void)
    {
	shaccton=0;
#ifdef AEXPAND
	sabuf.ac_flag |= AEXPND;
#endif /* AEXPAND */
    }

    /*
     * begin an accounting record by recording start time
     */
    void sh_accbegin(const char *cmdname)
    {
	if(SHACCT)
	{
		sabuf.ac_btime = time(NIL(time_t *));
		before = times(&buffer);
		sabuf.ac_uid = getuid();
		sabuf.ac_gid = getgid();
		strncpy(sabuf.ac_comm, (char*)path_basename(cmdname),
			sizeof(sabuf.ac_comm));
		shaccton = 1;
	}
    }
    /*
     * terminate an accounting record and append to accounting file
     */
    void	sh_accend(void)
    {
	int	fd;
	clock_t	after;

	if(shaccton)
	{
		after = times(&buffer);
		sabuf.ac_utime = compress(buffer.tms_utime + buffer.tms_cutime);
		sabuf.ac_stime = compress(buffer.tms_stime + buffer.tms_cstime);
		sabuf.ac_etime = compress( (time_t)(after-before));
		fd = open( SHACCT , O_WRONLY | O_APPEND | O_CREAT,RW_ALL);
		write(fd, (const char*)&sabuf, sizeof( sabuf ));
		close( fd);
	}
    }
 
    /*
     * Produce a pseudo-floating point representation
     * with 3 bits base-8 exponent, 13 bits fraction.
     */
    static int compress(register time_t t)
    {
	register int exp = 0, rund = 0;

	while (t >= 8192)
	{
		exp++;
		rund = t&04;
		t >>= 3;
	}
	if (rund)
	{
		t++;
		if (t >= 8192)
		{
			t >>= 3;
			exp++;
		}
	}
	return((exp<<13) + t);
    }
#endif	/* SHOPT_ACCT */

