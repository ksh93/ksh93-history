## Notice

<details open>

<summary>This repo was last updated on 2022-02-18 with a few minor changes:</summary>

- All versions of ksh from 2004-10-11 93p+ and up now build when using both glibc 2.35 and tcc.
- Added recovered INIT source code from complete source tarballs for the following versions:
  - 2000-10-31 93k (from Scientific Linux)
  - 2001-07-04b 93l+ (from old OpenBSD distfiles)
  - 2003-06-21 93o+ (from old OpenPKG sources)

</details>

<details>

<summary>Previous 2022-01-26 changelog</summary>

- Version 2004-10-11 93p+ now compiles and runs on Linux (`b_2004-10-11` branch).
- Added recovered sourced code for INIT 2003-06-21 93o+ found on the Wayback Machine.
- Some versions imported from the old ast-open-history repo were mislabled (i.e., the git tag had the wrong version name).
Additionally, some versions of ksh were reuploaded by AT&T to apply hotfixes.
The following git tags were renamed to account for this:
  - 2001-07-04 -> 2001-07-04b (reference: https://web.archive.org/web/20130605185416/http://www2.research.att.com/~gsf/download/notes.html)
  - 2006-12-04 -> 2006-12-07a
  - 2006-12-07 -> 2006-12-07b (reference: https://marc.info/?l=ast-users&m=120978599416040&w=2)
  - 2006-12-22 -> 2006-12-22b
  - 2007-04-18 -> 2007-04-18a
  - 2008-10-10 -> 2008-10-14
  - 2009-05-01 -> 2009-05-01b (reference: https://marc.info/?l=ast-users&m=124133736011926&w=2)
  - 2008-11-04a -> 2008-11-04
  - 2008-11-04b -> 2008-11-04-osnet
  - 2010-03-23 -> 2010-04-03-osnet
  - 2010-07-01 -> 2010-07-01b (reference: https://marc.info/?l=ast-users&m=127811568207485&w=2)
- Added releases of ksh93 recovered from the old [ksh93-integration](https://archive.org/details/mail.opensolaris.org) project:
  - 2006-05-30 93r+
  - 2006-06-16 93r+
  - 2006-06-30 93r+
  - 2006-07-24 93r+
  - 2006-09-12 93s-
  - 2006-12-04 93s
  - 2006-12-22a 93s (pre-hotfix)
  - 2007-04-18b 93s+ (post-hotfix)
  - 2007-10-31 93s+
  - 2009-06-22 93t+
  - 2010-04-17-osnet 93t+
- Added releases of ksh93 obtained from the openSUSE build service:
  - 2008-09-21 93t
  - 2008-10-10 93t
  - 2009-03-10 93t+
- Added 2001-01-01a ksh93k+ (from Splack Linux 8.0) to the archive (previously only the 2001-01-01a INIT code was found).
- Added 2008-06-24 ksh93t (from Mandrake Linux) to the archive.
- Added a build fix for many 93u+ and 93v- releases when compiling with `tcc` and `-D_std_malloc`.

You'll need to reclone the repo because the git history was regenerated (or you can download it [as a tarball](https://archive.org/download/ast-open-archive/git-repos/ksh93-history.tar.bz2)).

</details>

## ksh93-history repo
This is an archive of all released versions of ksh93 that I could find.
Unlike the [ast-open-archive](https://github.com/ksh93/ast-open-archive) repo,
ksh93-history only provides the ast-ksh and INIT packages, with build patches applied for debugging purposes.
Most of these versions are from Daniel Douglas's archive (see https://github.com/ksh93/ksh/discussions/388 and the [ast-open-history](https://archive.org/download/ast-open-archive/git-repos/ast-open-git.tar.bz2) repo),
although that archive is missing a few versions of ksh93 archived elsewhere.

## Showing diffs between versions
Each branch for versions ksh93p+ 2004-10-11 and up provides old versions of ksh with build fixes applied for use on Linux with either `gcc` or `tcc`.
For example, to checkout and build ksh93u+ 2012-06-26 the commands are:
```sh
$ git checkout b_2012-06-26  # Branches are the release date prefixed by 'b_'
$ bin/package make
```
Additionally, unpatched versions of ksh can be accessed via tags, e.g.:
```sh
# Show all changes in ksh93v- 2014-06-25 (minus license header changes)
$ git show -I '^\*     ' -I '^#      ' 2014-06-25

# Show a diff of versions 2009-05-01b and 2009-06-22
$ git diff -I '^\*     ' -I '^#      ' 2009-05-01b 2009-06-22
```
`git blame` can be used to view which version of ksh introduced or changed a line of code, e.g.:
```sh
$ git blame src/cmd/ksh93/sh/name.c
```
