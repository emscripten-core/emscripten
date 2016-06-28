#!/bin/sh

# Run the `update-copyright-year' script on all files in the git repository,
# taking care of exceptions stored in file `no-copyright'.

topdir=`git rev-parse --show-toplevel`
toolsdir=$topdir/src/tools

git ls-files --full-name $topdir        \
| sed 's|^|../../|'                     \
| grep -vFf $toolsdir/no-copyright      \
| xargs $toolsdir/update-copyright-year

# EOF
