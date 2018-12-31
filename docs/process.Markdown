Release Processes
=================


Minor version updates (1.X.Y to 1.X.Y+1)
----------------------------------------

When:

 * Such an update ensures we clear the cache, so it should be done when required (for example, a change to libc or libc++).
 * The emsdk compiled versions are based on the version number, so periodically we can do this when we want a new precompiled emsdk version to be available.

Requirements:

 * GitHub CI is green. (Currently, that includes good coverage for Linux, but nothing else - we should keep trying to find resources to do better here.)

How:

 * Ask on irc if there are any concerns.
 * The emscripten, emscripten-fastcomp, and emscripten-fastcomp-clang repos should each be updated: the emscripten-version.txt file in each, and a git tag (with the simple version number).
 * A tag should also be done in the binaryen repo.


Major version update (1.X.Y to 1.(X+1).0)
-----------------------------------------

When:

 * We should do such an update when we have a reasonable assurance of stability.

Requirements:

 * GitHub CI is green.
 * No major change recently landed.
 * No major recent regressions have been filed.
 * All tests pass locally for the person doing the update, including the main test suite (no params passed to `runner.py`), `other`, `browser`, `sockets`, `sanity`, `binaryen*`. (Not all of those are run on all the bots.)
 * A minor version was recently tagged, no major bugs have been reported on it, and nothing major landed since it did. (Bugs are often only found on tagged versions, so a big feature should first be in a minor version update before it is in a major one.)

How:

 * First, follow all the steps for a minor version update.
 * Also merge the `incoming` branch to `master`. This should not be done immediately, rather first we should at minimum see that CI and new builds are all green. If a problem occurs, we may only merge to master the minor version update that fixes things.
