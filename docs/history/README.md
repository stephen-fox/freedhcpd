# History

OpenBSD's dhcpd is a fork of the ISC dhcpd. Since the ISC code goes back
to 1995, I felt it was worth preserving not only the OpenBSD revision
history, but also the ISC revision history and git tags.

OpenBSD first added the forked dhcpd code to its codebase on April 13, 2004.
However, the code that was committed appears to be a fork of the ISC code
from February 14, 1999 (the base commit is an approximation, more on that
in the next section). My understanding is that the ISC license changed
shortly after that time.

## Finding the base commit

Unfortunately, finding the exact commit where OpenBSD forked dhcpd is not
trivial because the unmodified ISC source code was not committed to OpenBSD
first. In other words, the code that was initially committed to OpenBSD
was a reorganized and modified version of the ISC code, so we cannot
simply compare the code from the initial OpenBSD commit with the ISC code
that existed at the time and look for a minimalistic diff or look at source
files' copyright years.

Manually reviewing the OpenBSD and the upstream ISC code seemed to point
to early-mid 1999. I also wrote a terrible shell script (seen below) that
attempted to find the smallest number of changes between the two codebases.
This led me to ISC commit: 5812451fe2feb80397807564a1925be8c3f1970d with
13776 OpenBSD changes:

```sh
#!/bin/sh

die() {
  status=${1}

  shift

  echo "fatal: ${@}" 1>&2

  exit ${status}
}

#
# isc -> OpenBSD layout
#
setup_isc_repo_at_commit() {
  local tmp="$(mktemp -d)"

  (
  cd "${tmp}"
  tar -C "${isc_src}" -c . | tar -x

  git checkout "${1}" 2> /dev/null
  cp -r server src
  cp common/* src/
  cp includes/*.h src/
  cd src/
  rm class.c cdefs.h dhcp-* dlpi.c dns.c ethernet.c execute.c failover.c lpf.c mdb.c n* omapi.c raw.c resolv.c socket.c stables.c tr.c upf.c auth.h failover.h statement.h 2> /dev/null
  )

  echo "${tmp}"
}

num_lines_changed() {
  local isc_dir_tmp="${1}"
  local report_commit_dir="${2}"

  for f in "${obsd_src}"/src/*
  do
    isc_file="${isc_dir_tmp}/src/${f##*/}"
    report_file="${report_commit_dir}/${f##*/}-report.txt"

    # idk, should we default to counting all lines of a file
    # that does not exist in the isc repo?
    #wc -l < "${f}" | tr -d ' ' > "${report_file}"

    [ -f "${isc_file}" ] || continue

    diff \
      -y \
      --suppress-common-lines \
      "${isc_file}" \
      "${f}" \
      | wc -l | tr -d ' ' > "${report_file}"
  done

  local total=0

  for f in "${report_commit_dir}"/*
  do
    n="$(cat ${f})"

    total=$(($n + $total))
  done

  echo "${total}" > "${report_commit_dir}/total.txt"
}

isc_src="${HOME}/src/isc-dhcpd"

[ -d "${isc_src}" ] \
  || die 1 "isc src dir is missing: '${isc_src}'"

obsd_src="${HOME}/src/openbsd-dhcpd-init"

[ -d "${obsd_src}" ] \
  || die 1 "obsd src dir is missing: '${obsd_src}'"

report_dir="isc-obsd-commit-diff-report"

[ -e "${report_dir}" ] \
  && die 1 "report dir already exists at: ${report_dir}"

mkdir "${report_dir}"

# c570dfe68e7144bf6565b5d94121bd26d8f9f004 == last commit before 2004
# 494a48266eec31601da5960462dcadffd13d4b9a == Tue Mar 16 05:46:44 1999 +0000
# 2dddca227624e56f163c555845f8d7f86386d8b3 == Sun Feb 14 06:05:49 1999 +0000

isc_commits="$(cd "${isc_src}" && git rev-list --ancestry-path 2dddca227624e56f163c555845f8d7f86386d8b3..c570dfe68e7144bf6565b5d94121bd26d8f9f004)"

es="${?}"

[ ${es} -ne 0 ] \
  && die 1 "failed to get commit list"

smallest_num_changes=0
smallest_commit=''

for commit in ${isc_commits}
do
  tmp="$(setup_isc_repo_at_commit "${commit}")"

  report_commit_dir="${report_dir}/${commit}"

  mkdir "${report_commit_dir}" \
    || die 1 "failed to create report commit dir"

  num_lines_changed "${tmp}" "${report_commit_dir}"

  rm -rf "${tmp}"

  commit_total_changes="$(cat "${report_commit_dir}/total.txt")"

  if [ ${smallest_num_changes} -eq 0 ] || [ ${commit_total_changes} -lt ${smallest_num_changes} ]
  then
    smallest_num_changes=${commit_total_changes}
    smallest_commit="${commit}"
  fi
done

echo "smallest commit: ${smallest_commit} (${smallest_num_changes} changes)"
```

#### Notes on finding base commit

Here are my original notes on finding the base commit:

ppearance of dhcpd in OpenBSD:
  - Tue Apr 13 23:41:48 2004 +0000
  - 5534f31d93285db5579730dbcf545aefa8fc899d
- isc-dhcpd license change commit: f39b6e0089fb9a1bc0d06d12a9c5c3d995744bb5
  - Tue Mar 16 05:50:46 1999
  - This commit changes bpf.c's license header ("Copyright (c) 1995, 1996,
    1998, 1999") which is what the OpenBSD bpf.c has
  - In other words, we want the commit *before* this one (f39b6e00...)

maybe this? 494a48266eec31601da5960462dcadffd13d4b9a

Possibilities:

- 494a48266eec31601da5960462dcadffd13d4b9a
  - Tue Mar 16 05:50:46 1999 +0000
  - Commit before license change commit (f39b6e00)
  - Only removes files, none of which are in OpenBSD
- 3cd5e20af9f03539d15a3ec3c632dac25db0b3af
  - Tue Mar 16 05:45:05 1999 +0000
  - Only removes files, none of which are in OpenBSD
- 101021cf1e5a14f2ff1c864819baf1854fda0cfc
  - Tue Mar 16 05:43:18 1999 +0000
  - Only removes files, none of which are in OpenBSD
- cfd64a6043f92c2a8c17ad42af0a64581d0ea2d6
  - Tue Mar 16 05:30:49 1999 +0000
  - Only removes files, none of which are in OpenBSD
- 0cd0c3e07d48cc18857f84cb79a1f7d9b595db36
  - Tue Mar 16 05:26:04 1999 +0000
  - Only removes files, none of which are in OpenBSD
- d9dc2726bc7d74e10988ded9640887e372db146b
  - Tue Mar 16 05:26:04 1999 +0000
  - This removes `shared-network ISC-BIGGIE` from "dhcpd.conf.5",
    which is still in the equivalent OpenBSD file
- 47434ab2bab3bff196b7ed79dbad17129fa331ab
  - Tue Mar 16 00:56:36 1999 +0000
  - Updates some C arrays (`static char message []` and
    `static char ocopyright[]`) which are not in OpenBSD
- 0527c2e320a87c30ee91f4c10dab49dda7a9622c
  - Wed Mar 10 20:42:27 1999 +0000
  - no
- 74f45f9642639bf4dfc7a28595363055d4c1b955
  - Thu Feb 25 23:30:43 1999 +0000
  - I previously wrote "pretty sure this is it", but notes do not corroborate
  - based on osdep.h copyright years change in the commit that follows it
  - no
- (...)
- 8ae2d59584635a17803f292a1fb1924dfb6754c5
  - Wed Feb 24 17:56:53 1999 +0000
  - no
- 339b0231e472d8953057bd5cdd74d5a1215f8ce1
  - Sun Feb 14 19:40:22 1999 +0000
  - This commit bookends a series of commits that happened on Feb 14
  - Originally wrote "no", but the diff comparison script I wrote led me
    to an earlier commit in the Feb 14 commit series
  - Looking at this commit's diff, it changed "dhcpd.h" to have:
    `*dummy_interfaces, *fallback_interface;`
    ... which is present in the OpenBSD code and "bootp.c" to have:
    `} else`
    ... which is also in the OpenBSD code
  - The above similarities seem to indicate this commit was included in
    the OpenBSD code
  - Going to use this commit as the base
- (... several other Feb 14 commits ...)
- 92e3e6911bef49037fb1bcabeeb05ae9a803e50e
  - Sun Feb 14 19:06:57 1999 +0000
  - This updates bootp.c's license from:
    `Copyright (c) 1995, 1996, 1997, 1998 The Internet Software Consortium`
    to:
    `Copyright (c) 1995, 1996, 1997, 1998, 1999`
  - The OpenBSD source file does not have "1997" in that string, however
    there is no point in time in the isc code where "1997" was missing from
    this string. git blame:
    - f39b6e0089fb9a1bc0d06d12a9c5c3d995744bb5
    - ef0afca963c5ed218da3d18362dc7742b520852e
  - no
- a2405f2e71139988cc0aa888c56208c3f0d9ab1e
  - Sun Feb 14 19:04:45 1999 +0000
  - Modifies "Makefile.dist", not in OpenBSD
  - no
- (...)
- 5812451fe2feb80397807564a1925be8c3f1970d
  - Sun Feb 14 18:49:45 1999 +0000
  - Script says this commit has the smallest number of differences with
    OpenBSD initial commit
  - Interestingly, this commit changes the "memory.c" license header
    to something different than OpenBSD's "memory.c"
  - Also removes "Enter a new subnet" comment from "memory.c", which is
    present in the OpenBSD code
  - In other words, I think we want the commit before this one?
  - ... so maybe?
- b25cbbad959adc01ee3f90cbc6d8744bb7c4d3b7
  - Sun Feb 14 18:46:20 1999 +0000
  - This commit changes a comment in "dispatch.c" to be:
    `Wait for packets to come in using poll().  When a packet comes in,`
    ... which appears in the OpenBSD source code, so this commit seems
    very promising
- (...) bunch of commits on Sun Feb 14, skip to next commit which is
  1998 - which contradicts the source file copyright dates in OpenBSD
- 31c08e370fa796db9ee71dadc4528b09e176a460
  - Tue Dec 22 22:34:54 1998 +0000
  - Adds Linux source file

Other thoughts:

- Maybe we can use "dhcpd.conf.cat5" to test this since it appears
  in the first OpenBSD commit, but not the later ones?
- Get commits between:
  - `git rev-list --ancestry-path 2dddca227624e56f163c555845f8d7f86386d8b3..494a48266eec31601da5960462dcadffd13d4b9a`
  - ... or: `git log 2dddca227624e56f163c555845f8d7f86386d8b3..494a48266eec31601da5960462dcadffd13d4b9a --oneline | cut -d ' ' -f 1`
- Commits:
  - 494a4826: Tue Mar 16 05:50:46 1999 +0000
  - 2dddca22: Last commit of 1999

find-commit-with-smallest-diff.sh said:

```
smallest commit: 5812451fe2feb80397807564a1925be8c3f1970d (13776 changes)
```

## Combining the ISC and OpenBSD revision histories together

I looked at a few ways of doing this and wrote a bunch of notes about it
(seen below). I decided the least ugly way to do this was to introduce
a "bridge commit" by myself between the last ISC commit and the first
OpenBSD commit that moves the ISC files around as henning had done.
Maybe that will make people sad (I am sorry!), but I went back and
forth on this decision *a lot* and it seemed like the least terrible
option.

I initially tried avoiding a "bridge" commit (one created be me that goes
between the last ISC commit and first OpenBSD commit that essentially does
`git mv`, see `combine-isc-and-obsd-dhcpd.sh`). This caused `git log --follow`
to not work consistently, so I ended up going with a bridge commit using
`combine-isc-and-obsd-dhcpd-bridge-commit.sh`.

After running that script, I did:

```sh
mv isc-and-openbsd freedhcpd

cd freedhcpd

git branch -m main

git remote add origin git@gitlab.com:stephen-fox/freedhcpd
git remote add github git@github.com:stephen-fox/freedhcpd
git remote add codeberg git@codeberg.org:stephen-fox/freedhcpd

git checkout -b upstream

git checkout -b last-isc-commit
git reset --hard 339b0231e472d8953057bd5cdd74d5a1215f8ce1
git tag last-isc-commit
git checkout main
git branch -D last-isc-commit

git checkout -b first-openbsd-commit
git reset --hard 51f908b6349642110c6733ce3570f8c0bd12dd82
git tag first-openbsd-commit
git checkout main
git branch -D first-openbsd-commit

git push -u origin $(git branch --show-current)
git push origin --all
git push github --all
git push codeberg --all

git push --tags
git push --tags github
git push --tags codeberg
```

Here is the awful shell script I used:

```sh
#!/bin/sh

set -eux

die() {
  local es="${1}"
  shift
  echo "fatal: ${@}" 1>&2
}

work_dir=${HOME}/src

#
# OpenBSD dhcpd src setup.
#
if [ ! -d "${work_dir}/openbsd-src" ]
then
  git clone https://github.com/openbsd/src ${work_dir}/openbsd-src
fi

if [ ! -d "${work_dir}/openbsd-dhcpd" ]
then
  cp -r ${work_dir}/openbsd-src ${work_dir}/openbsd-dhcpd

  (
  cd ${work_dir}/openbsd-dhcpd
  git remote remove origin
  git filter-repo --path usr.sbin/dhcpd
  )
fi

if [ ! -d "${work_dir}/openbsd-dhcpd-init" ]
then
  cp -r ${work_dir}/openbsd-dhcpd ${work_dir}/openbsd-dhcpd-init

  (
  cd ${work_dir}/openbsd-dhcpd-init
  git reset --hard $(git rev-list --max-parents=0 HEAD)
  )
fi

#
# ISC dhcpd src setup.
#
if [ ! -d "${work_dir}/isc-dhcpd" ]
then
  git clone https://github.com/isc-projects/dhcp ${work_dir}/isc-dhcpd
  ( cd ${work_dir}/isc-dhcpd && git remote remove origin)
fi

if [ -d "${work_dir}/isc-and-openbsd" ]
then
  die 1 "isc-and-dhcpd dir already exists"
fi

cp -r ${work_dir}/isc-dhcpd ${work_dir}/isc-and-openbsd

(
cd ${work_dir}/isc-and-openbsd
git checkout master
git reset --hard 339b0231e472d8953057bd5cdd74d5a1215f8ce1

# Remove all commits and tags
git for-each-ref --contains 6b6126fbd7882466355eda89b50a81957e4f4e32 | while read l; do ref="$(echo "$l" | cut -d $'\x09' -f 2)"; echo ${ref}; tag="${ref##*refs/tags/}"; [ "${ref}" != "${tag}" ] && git tag -d "${tag}"; git update-ref -d "${ref}"; done

# ISC-DHCPD commits are now complete with original hashes
ISC_LAST=$(git rev-parse HEAD)

# Restructure files
mkdir usr.sbin
git mv server usr.sbin/dhcpd

for f in ${work_dir}/openbsd-dhcpd-init/usr.sbin/dhcpd/*; do name="${f##*/}"; existing="$(find . -type f -name "${name}" | head -n 1)"; [ -z "${existing}" ] && continue; echo "${existing}" | grep -F 'usr.sbin/' && continue; git mv "${existing}" "usr.sbin/dhcpd/${name}"; done

git mv Makefile.dist usr.sbin/dhcpd/Makefile

find . -not -path . -not -path ./usr.sbin -not -path ./.git -maxdepth 1 -print0 | xargs -0 git rm -rf

# Create bridge commit (this doesn't change ISC_LAST's hash)
git commit -m "isc: Restructure to match OpenBSD layout"
BRIDGE_COMMIT=$(git rev-parse HEAD)

# Add OpenBSD repo
git remote add obsd ${work_dir}/openbsd-dhcpd
git fetch obsd

# Find OpenBSD's first commit
FIRST_OBSD=$(git rev-list --max-parents=0 obsd/master)

# Graft OpenBSD's first commit to have the bridge commit as parent
git replace --graft $FIRST_OBSD $BRIDGE_COMMIT

# Checkout OpenBSD (now includes ISC + bridge via graft)
git checkout obsd/master
git checkout -B master

# Make permanent
git filter-branch -f --force -- --all

# Cleanup
git replace -d $FIRST_OBSD || true
git remote remove obsd
git gc --prune=now
)
```
