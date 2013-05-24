/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1982-2013 AT&T Intellectual Property          *
*                      and is licensed under the                       *
*                 Eclipse Public License, Version 1.0                  *
*                    by AT&T Intellectual Property                     *
*                                                                      *
*                A copy of the License is available at                 *
*          http://www.eclipse.org/org/documents/epl-v10.html           *
*         (with md5 checksum b35adb5213ca9657e911e9befb180842)         *
*                                                                      *
*              Information and Software Systems Research               *
*                            AT&T Research                             *
*                           Florham Park NJ                            *
*                                                                      *
*                  David Korn <dgk@research.att.com>                   *
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * cd [-LP@]  [dirname]
 * cd [-LP@]  [old] [new]
 * pwd [-LP]
 *
 *   David Korn
 *   AT&T Labs
 *   research!dgk
 *
 */

#include	"defs.h"
#include	<stak.h>
#include	<error.h>
#include	"variables.h"
#include	"path.h"
#include	"name.h"
#include	"builtins.h"
#include	<ls.h>

#ifndef EINTR_REPEAT
#   define EINTR_REPEAT(expr) while((expr) && (errno == EINTR)) errno=0;
#endif

/*
 * Invalidate path name bindings to relative paths
 */
static void rehash(register Namval_t *np,void *data)
{
	Pathcomp_t *pp = (Pathcomp_t*)np->nvalue.cp;
	NOT_USED(data);
	if(pp && *pp->name!='/')
		_nv_unset(np,0);
}

/*
 * Obtain a file handle to the directory "path" relative to directory
 * "dir", or open a NFSv4 xattr directory handle for file dir/path.
 */
int sh_diropenat(Shell_t *shp, int dir, const char *path, bool xattr)
{
	int fd,shfd;
	int savederrno=errno;
	struct stat fs;
#ifndef AT_FDCWD
	NOT_USED(dir);
#endif
#ifndef O_XATTR
	NOT_USED(xattr);
#endif

#ifdef O_XATTR
	if(xattr)
	{
		int apfd; /* attribute parent fd */
		/* open parent node... */
		EINTR_REPEAT((apfd = openat(dir, path, O_RDONLY|O_NONBLOCK|O_cloexec)) < 0);
		if(apfd < 0)
			return -1;

		/* ... and then open a fd to the attribute directory */
		 EINTR_REPEAT((fd = openat(apfd, e_dot, O_XATTR|O_cloexec)) < 0);

		savederrno = errno;
		EINTR_REPEAT(close(apfd) < 0);
		errno = savederrno;
	}
	else
#endif
	{
#ifdef AT_FDCWD
		/*
		 * Open directory. First we try without |O_SEARCH| and
		 * if this fails with EACCESS we try with |O_SEARCH|
		 * again.
		 * This is required ...
		 * - ... because some platforms may require that it can
		 * only be used for directories while some filesystems
		 * (e.g. Reiser4 or HSM systems) allow a |fchdir()| into
		 * files, too)
		 * - ... to preserve the semantics of "cd", e.g.
		 * otherwise "cd" would return [No access] instead of
		 * [Not a directory] for files on filesystems which do
		 * not allow a "cd" into files.
		 * - ... to allow that a
		 * $ redirect {n}</etc ; cd /dev/fd/$n # works on most
		 * platforms.
		 */
		if(*path=='/' && memcmp(path,"/dev/fd/",8)==0 && isdigit(path[8]))
		{
			char	*p = (char*)path+8;
			fd = strtol(p,&p,10);
			if(*p=='/' || *p==0)
			{
				while(*p=='/')
					p++;
				path = *p?(const char*)p:0;
				dir = fd;
			}
		}
		EINTR_REPEAT((fd = openat(dir, path, O_RDONLY|O_NONBLOCK|O_cloexec)) < 0);
#   ifdef O_SEARCH
		if((fd < 0) && (errno == EACCES))
		{
			EINTR_REPEAT((fd = openat(dir, path, O_SEARCH|O_cloexec)) < 0)
		}
#   endif
#else
		/*
		 * Version of openat() call above for systems without
		 * openat API. This only works because we basically
		 * gurantee that |dir| is always the same place as
		 * |cwd| on such machines (but this won't be the case
		 * in the future).
		 */
		/*
		 * This |fchdir()| call is not needed (yet) since
		 * all consumers do not use |dir| when |AT_FDCWD|
		 * is not available.
		 *
		 * fchdir(dir);
		 */
		EINTR_REPEAT((fd = open(path, O_cloexec)) < 0);
#endif
	}

	if(fd < 0)
		return fd;
	if (!fstat(fd, &fs) && !S_ISDIR(fs.st_mode))
	{
		close(fd);
		errno = ENOTDIR;
		return -1;
	}

	/* Move fd to a number > 10 and *register* the fd number with the shell */
	shfd = sh_fcntl(fd, F_dupfd_cloexec, 10);
	savederrno=errno;
	sh_close(fd);
	errno=savederrno;
	return(shfd);
}

int	b_cd(int argc, char *argv[],Shbltin_t *context)
{
	register char *dir;
	Pathcomp_t *cdpath = 0;
	register const char *dp;
	register Shell_t *shp = context->shp;
	int saverrno=0;
	int rval;
	bool flag=false,xattr=false;
	char *oldpwd;
	int newdirfd;
	Namval_t *opwdnod, *pwdnod;
	if(sh_isoption(shp,SH_RESTRICTED))
		errormsg(SH_DICT,ERROR_exit(1),e_restricted+4);
	while((rval = optget(argv,sh_optcd))) switch(rval)
	{
		case 'L':
			flag = false;
			break;
		case 'P':
			flag = true;
			break;
#ifdef O_XATTR
		case '@':
			xattr = true;
			break;
#endif
		case ':':
			errormsg(SH_DICT,2, "%s", opt_info.arg);
			break;
		case '?':
			errormsg(SH_DICT,ERROR_usage(2), "%s", opt_info.arg);
			break;
	}
	argv += opt_info.index;
	argc -= opt_info.index;
	dir =  argv[0];
	if(error_info.errors>0 || argc >2)
		errormsg(SH_DICT,ERROR_usage(2),"%s",optusage((char*)0));
	oldpwd = (char*)shp->pwd;
	opwdnod = (shp->subshell?sh_assignok(OLDPWDNOD,1):OLDPWDNOD); 
	pwdnod = (shp->subshell?sh_assignok(PWDNOD,1):PWDNOD); 
	if(argc==2)
		dir = sh_substitute(shp,oldpwd,dir,argv[1]);
	else if(!dir)
		dir = nv_getval(HOME);
	else if(*dir == '-' && dir[1]==0)
		dir = nv_getval(opwdnod);
	if(!dir || *dir==0)
		errormsg(SH_DICT,ERROR_exit(1),argc==2?e_subst+4:e_direct);
#if _WINIX
	if(*dir != '/' && (dir[1]!=':'))
#else
	if(*dir != '/')
#endif /* _WINIX */
	{
		if(!(cdpath = (Pathcomp_t*)shp->cdpathlist) && (dp=sh_scoped(shp,CDPNOD)->nvalue.cp))
		{
			if(cdpath=path_addpath(shp,(Pathcomp_t*)0,dp,PATH_CDPATH))
			{
				shp->cdpathlist = (void*)cdpath;
				cdpath->shp = shp;
			}
		}
		if(!oldpwd)
			oldpwd = path_pwd(shp,1);
	}
	if(*dir=='.')
	{
		/* test for pathname . ./ .. or ../ */
		int	n=0;
		char	*sp;
		for(dp=dir; *dp=='.'; dp++)
		{
			if(dp[1] =='.' && (dp[2]=='/' || dp[2]==0))
				n++,dp+=2;
			else if(*dp && *dp!='/')
				break;
			if(*dp==0)
				break;
		}
		if(n)	
		{
			cdpath = 0;
			sp = oldpwd + strlen(oldpwd);
			while(n--)
			{
				while(--sp > oldpwd && *sp!='/');
				if(sp==oldpwd)
					break;
				
			}
			sfwrite(shp->strbuf,oldpwd,sp+1-oldpwd);
			sfputr(shp->strbuf,dp,0);
			dir = sfstruse(shp->strbuf);
		}
	}
#ifdef O_XATTR
	if (xattr)
#   ifdef PATH_BFPATH
		cdpath = NULL;
#   else
		cdpath = "";
#   endif
#endif

	rval = -1;
	do
	{
		dp = cdpath?cdpath->name:"";
		cdpath = path_nextcomp(shp,cdpath,dir,0);
#if _WINIX
                if(*stakptr(PATH_OFFSET+1)==':' && isalpha(*stakptr(PATH_OFFSET)))
		{
			*stakptr(PATH_OFFSET+1) = *stakptr(PATH_OFFSET);
			*stakptr(PATH_OFFSET)='/';
		}
#endif /* _WINIX */
                if(*stakptr(PATH_OFFSET)!='/')

		{
			char *last=(char*)stakfreeze(1);
			stakseek(PATH_OFFSET);
			stakputs(oldpwd);
			/* don't add '/' of oldpwd is / itself */
			if(*oldpwd!='/' || oldpwd[1])
				stakputc('/');
			stakputs(last+PATH_OFFSET);
			stakputc(0);
		}
		if(!flag)
		{
			register char *cp;
			stakseek(PATH_MAX+PATH_OFFSET);
#if SHOPT_FS_3D
			if(!(cp = pathcanon(stakptr(PATH_OFFSET),PATH_MAX,PATH_DOTDOT)))
				continue;
			/* eliminate trailing '/' */
			while(*--cp == '/' && cp>stakptr(PATH_OFFSET))
				*cp = 0;
#else
			if(*(cp=stakptr(PATH_OFFSET))=='/')
				if(!pathcanon(cp,PATH_MAX,PATH_DOTDOT))
					continue;
#endif /* SHOPT_FS_3D */
		}
		rval = newdirfd = sh_diropenat(shp, shp->pwdfd,
			path_relative(shp,stakptr(PATH_OFFSET)), xattr);
		if(newdirfd >=0)
		{
			/* chdir for directories on HSM/tapeworms may take minutes */
			if(fchdir(newdirfd) >= 0)
			{
				if(shp->pwdfd >= 0)
					sh_close(shp->pwdfd);
				shp->pwdfd=newdirfd;
				goto success;
			}
		}
#ifndef O_SEARCH
		else
		{
			if((rval=chdir(path_relative(shp,stakptr(PATH_OFFSET)))) >= 0)
			{
				if(shp->pwdfd >= 0)
				{
					sh_close(shp->pwdfd);
#ifdef AT_FDCWD
					shp->pwdfd = AT_FDCWD;
#else
					shp->pwdfd = -1;
#endif
				}
			}
		}
#endif
		if(saverrno==0)
                        saverrno=errno;
		if(newdirfd >=0)
			sh_close(newdirfd);
	}
	while(cdpath);
	if(rval<0 && *dir=='/' && *(path_relative(shp,stakptr(PATH_OFFSET)))!='/')
	{
		rval = newdirfd = sh_diropenat(shp,
			shp->pwdfd,
			dir, xattr);
		if(newdirfd >=0)
		{
			/* chdir for directories on HSM/tapeworms may take minutes */
			if(fchdir(newdirfd) >= 0)
			{
				if(shp->pwdfd >= 0)
					sh_close(shp->pwdfd);
				shp->pwdfd=newdirfd;
				goto success;
			}
		}
#ifndef O_SEARCH
		else
		{
			if(chdir(dir) >=0)
			{
				if(shp->pwdfd >= 0)
				{
					sh_close(shp->pwdfd);
					shp->pwdfd=-1;
				}
			}
		}
#endif
	}
	/* use absolute chdir() if relative chdir() fails */
	if(rval<0)
	{
		if(saverrno)
			errno = saverrno;
		errormsg(SH_DICT,ERROR_system(1),"%s:",dir);
	}
success:
	if(dir == nv_getval(opwdnod) || argc==2)
		dp = dir;	/* print out directory for cd - */
	if(flag)
	{
		dir = stakptr(PATH_OFFSET);
		if (!(dir=pathcanon(dir,PATH_MAX,PATH_PHYSICAL)))
		{
			dir = stakptr(PATH_OFFSET);
			errormsg(SH_DICT,ERROR_system(1),"%s:",dir);
		}
		stakseek(dir-stakptr(0));
	}
	dir = (char*)stakfreeze(1)+PATH_OFFSET;
	if(*dp && (*dp!='.'||dp[1]) && strchr(dir,'/'))
		sfputr(sfstdout,dir,'\n');
	if(*dir != '/')
		return(0);
	nv_putval(opwdnod,oldpwd,NV_RDONLY);
	flag = (strlen(dir)>0)?true:false;
	/* delete trailing '/' */
	while(--flag>0 && dir[flag]=='/')
		dir[flag] = 0;
	nv_putval(pwdnod,dir,NV_RDONLY);
	nv_onattr(pwdnod,NV_NOFREE|NV_EXPORT);
	shp->pwd = pwdnod->nvalue.cp;
	nv_scan(shp->track_tree,rehash,(void*)0,NV_TAGGED,NV_TAGGED);
	path_newdir(shp,shp->pathlist);
	path_newdir(shp,shp->cdpathlist);
	if(oldpwd)
		free(oldpwd);
	return(0);
}

int	b_pwd(int argc, char *argv[],Shbltin_t *context)
{
	register int n, flag = 0;
	register char *cp;
	register Shell_t *shp = context->shp;
	NOT_USED(argc);
	while((n = optget(argv,sh_optpwd))) switch(n)
	{
		case 'L':
			flag = 0;
			break;
		case 'P':
			flag = 1;
			break;
		case ':':
			errormsg(SH_DICT,2, "%s", opt_info.arg);
			break;
		case '?':
			errormsg(SH_DICT,ERROR_usage(2), "%s", opt_info.arg);
			break;
	}
	if(error_info.errors)
		errormsg(SH_DICT,ERROR_usage(2),"%s",optusage((char*)0));
	if(*(cp = path_pwd(shp,0)) != '/')
		errormsg(SH_DICT,ERROR_system(1), e_pwd);
	if(flag)
	{
#if SHOPT_FS_3D
		if(shp->gd->lim.fs3d && (flag = mount(e_dot,NIL(char*),FS3D_GET|FS3D_VIEW,0))>=0)
		{
			cp = (char*)stakseek(++flag+PATH_MAX);
			mount(e_dot,cp,FS3D_GET|FS3D_VIEW|FS3D_SIZE(flag),0);
		}
		else
#endif /* SHOPT_FS_3D */
			cp = strcpy(stakseek(strlen(cp)+PATH_MAX),cp);
		pathcanon(cp,PATH_MAX,PATH_PHYSICAL);
	}
	sfputr(sfstdout,cp,'\n');
	return(0);
}

