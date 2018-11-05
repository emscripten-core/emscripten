Development Processes
---------------------

Landing PRs
===========

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
-----------------

Minor version updates (1.X.Y to 1.X.Y+1)
========================================

When:

 * Such an update ensures we clear the cache, so it should be done when required
   (for example, a change to libc or libc++).
 * The emsdk compiled versions are based on the version number, so periodically
   we can do this when we want a new precompiled emsdk version to be available.

Requirements:

 * GitHub CI is green. (Currently, that includes good coverage for Linux, but
   nothing else - we should keep trying to find resources to do better here.)

How:

 * Ask on irc if there are any concerns.
 * The emscripten, emscripten-fastcomp, and emscripten-fastcomp-clang repos
   should each be updated: the emscripten-version.txt file in each, and a git
   tag (with the simple version number).
 * A tag should also be done in the binaryen repo.


Major version update (1.X.Y to 1.(X+1).0)
=========================================

When:

 * We should do such an update when we have a reasonable assurance of stability.

Requirements:

 * GitHub CI is green.
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

 * First, follow all the steps for a minor version update.
 * Also merge the `incoming` branch to `master`. This should not be done
   immediately, rather first we should at minimum see that CI and new builds are
   all green. If a problem occurs, we may only merge to master the minor version
   update that fixes things.
