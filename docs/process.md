Development Processes
=====================

Landing PRs
-----------

 * Even after the code of a PR is approved, it should only be landed if the
   CI on github is green, or the failures are known intermittent things
   (with very strong reason to think they unrelated to the current PR).
 * If you see an approved PR of someone without commit access (that either
   you or someone else approved), land it for them (after checking CI as
   mentioned earlier).
 * If you approve a PR by someone with commit access, if there is no urgency
   then leave it for them to land. (They may have other PRs to land alongside
   it, etc.)
 * It is strongly recommended to land PRs with github's "squash" option, which
   turns the PR into a single commit. This makes sense if the PR is small,
   which is also strongly recommended. However, sometimes separate commits may
   make more sense, *if and only if*:
    * The PR is not easily separable into a series of small PRs (e.g., review
      must consider all the commits, either because the commits are hard to
      understand by themselves, or because review of a later PR may influence
      an earlier PR's discussion).
    * The individual commits have value (e.g., they are easier to understand
      one by one).
    * The individual commits are compatible with bisection (i.e., all tests
      should pass after each commit).
   When landing multiple commits in such a scenario, use the "rebase" option,
   to avoid a merge commit.


Release Processes
=================

Minor version updates (1.X.Y to 1.X.Y+1)
----------------------------------------

When:

 * Such an update ensures we clear the cache, so it should be done when required
   (for example, a change to libc or libc++).
 * The emsdk compiled versions are based on the version number, so periodically
   we can do this when we want a new precompiled emsdk version to be available.

Requirements:

 * [emscripten-releases build CI](https://ci.chromium.org/p/emscripten-releases/g/main/console)
   is green on all OSes for the desired hash (where the hash is the git hash in
   the
   [emscripten-releases](https://chromium.googlesource.com/emscripten-releases)
   repo, which then specifies through
   [DEPS](https://chromium.googlesource.com/emscripten-releases/+/refs/heads/master/DEPS)
   exactly which revisions to use in all other repos).
 * [GitHub CI](https://github.com/emscripten-core/emscripten/branches) is green
   on the `master` branch.

How:

1. Open a PR for the emsdk to update
   [emscripten-releases-tags.txt](https://github.com/emscripten-core/emsdk/blob/master/emscripten-releases-tags.txt),
   adding the version and the hash. Updating the "latest" tag there to the new
   release is possible, but can also be deferred if you want to do more testing
   before users fetching "latest" get this release.
2. Run [update_bazel_workspace.sh](https://github.com/emscripten-core/emsdk/blob/master/scripts/update_bazel_workspace.sh).
   This will update the bazel toolchain based on the new "latest" tag in emscripten-releases-tags.txt.
3. Tag the emsdk repo as well, on the commit that does the update, after it
   lands on master.
4. Update
   [emscripten-version.txt](https://github.com/emscripten-core/emscripten/blob/master/emscripten-version.txt)
   in the emscripten repo. This is a delayed update, in that the tag will refer
   to the actual release, but the update to emscripten-version.txt is a new
   commit to emscripten that happens later.
   * To minimize the difference, we should pick hashes for releases that are
     very recent, and try to avoid anything else landing in between - can ask
     on irc/chat for people to not land anything, or do this at a time of day
     when that's unlikely, etc.
   * There is no need to open a PR for this change, you can optionally just
     commit it directly.
5. Tag the emscripten repo on the emscripten commit on which
   `emscripten-version.txt` was updated. (This could also be the commit from the
   DEPS file as well, but this way is less confusing when just working on the
   emscripten repo, and the difference should only be one commit anyhow.)

Major version update (1.X.Y to 1.(X+1).0)
-----------------------------------------

When:

 * We should do such an update when we have a reasonable assurance of stability.

Requirements:

 * All the requirements for a minor update.
 * No major change recently landed.
 * No major recent regressions have been filed.
 * All tests pass locally for the person doing the update, including the main
   test suite (no params passed to `runner.py`), `other`, `browser`, `sockets`,
   `sanity`, `binaryen*`. (Not all of those are run on all the bots.)
 * A minor version was recently tagged, no major bugs have been reported on it,
   and nothing major landed since it did. (Bugs are often only found on tagged
   versions, so a big feature should first be in a minor version update before
   it is in a major one.)

How:

1. Follow the same steps for a minor version update.


Updating the `emscripten.org` Website
--------------------------------------

The site is currently hosted in `gh-pages` branch of the separate [site
repository](site_repo). To update the docs, rebuild them and copy them there,
that is:

1. In your emscripten repo checkout, enter `site`.
2. Run `make html`.
3. Run `make install EMSCRIPTEN_SITE=\[path-to-a-checkout-of-the-site-repo\]`
3. Go to the site repo, commit the changes, and push.

[site_repo]: https://github.com/kripken/emscripten-site


Updating the `emcc.py` help text
--------------------------------

`emcc --help` output is generated from the main documentation under `site/`,
so it is the same as shown on the website, but it is rendered to text. After
updating `emcc.rst` in a PR, the following should be done:

1. In your emscripten repo checkout, enter `site`.
2. Run `make clean` (without this, it may not emit the right output).
2. Run `make text`.
3. Add the changes to your PR.
