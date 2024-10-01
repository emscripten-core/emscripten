#!/bin/sh
#
# Git post-checkout script that takes care of running emscirpten's
# bootstrap script when changing branches.
#
# The bootstrap script itself is smart enough to basically do nothing unless
# one of the relevant files was updated (e.g. package.json).

# Test for the existence of the bootstrap script itself to handle branches
# that predate its existence.
test -f ./bootstrap && exec ./bootstrap
