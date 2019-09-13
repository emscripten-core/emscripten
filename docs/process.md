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
   on the incoming branch.

How:

1. Open a PR for the emsdk to update
   [emscripten-releases-tags.txt](https://github.com/emscripten-core/emsdk/blob/master/emscripten-releases-tags.txt),
   adding the version and the hash. Updating the "latest" tag there to the new
   release is possible, but can also be deferred if you want to do more testing
   before users fetching "latest" get this release.
2. Tag the emscripten repo on the emscripten commit used by that release (which
   you can tell from the DEPS file), using something like
   `git checkout [COMMIT]` ; `git tag [VERSION]` ; `git push --tags`.
3. Update
   [emscripten-version.txt](https://github.com/emscripten-core/emscripten/blob/incoming/emscripten-version.txt)
   in the emscripten repo. This is a delayed update, in that the tag will refer
   to the actual release, but the update to emscripten-version.txt is a new
   commit to emscripten that happens later.
   * To minimize the difference, we should pick hashes for releases that are
     very recent, and try to avoid anything else landing in between - can ask
     on irc/chat for people to not land anything, or do this at a time of day
     when that's unlikely, etc.
   * There is no need to open a PR for this change, you can optionally just
     commit it directly.


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

1. Follow all the steps for a minor version update.
2. Merge the `incoming` branch to `master`. This should not be done immediately,
   rather first we should at minimum see that CI and new builds are all green.
   If a problem occurs, we may only merge to master the minor version update
   that fixes things.


Updating the `emscripten.org` Website
--------------------------------------

The site is currently hosted in `gh-pages` branch of the separate [site
repository](site_repo). To update the docs, rebuild them and copy them there,
that is:

1. In your emscripten repo checkout, enter `site`.
2. Run `make html`.
3. Run `cp -R build/html/* \[path-to-a-checkout-of-the-site-repo\]`
3. Go to the site repo, commit the changes, and push.

[site_repo]: https://github.com/kripken/emscripten-site


Packaging Emscripten
--------------------

If you package Emscripten for users in some manner, the details in the rest of
this document should be helpful with understanding versioning and so forth.
This section goes into that in more detail.

The core
[DEPS](https://chromium.googlesource.com/emscripten-releases/+/refs/heads/master/DEPS)
file in the chromium `emscripten-releases` contains all the information about versions
in all the repos. This is the repo used by our CI to build and test emscripten. How
this works is that you look at the DEPS file at a particular git revision. It has
lines like these:
```
  'binaryen_revision': '06698d7a32cb4eeb24fea942e83d1b15e86a73e6',
  'emscripten_revision': '7224b7930ec2a6abca332300e247619e1aea1719',
  'llvm_project_revision': '33ef687d94604aeb73bedbcf3050524465a3439f',
```
Those are the git hashes in those repos. For building that git commit in
emscripten-releases, our CI fetched and built those revisions.

In principle you can build any git hash in that repo. You can look at the
[CI UI](https://ci.chromium.org/p/emscripten-releases/g/main/console) to see
if our CI shows green for any hash, and if so, it should be safe to build.

Alternatively, you may want to build our official emscripten release tags, which
are the versions the emsdk lets users install. To find the mapping between the
emscripten versions and the git hash for the DEPS file, the emsdk has
[emscripten-releases-tags.txt](https://github.com/emscripten-core/emsdk/blob/master/emscripten-releases-tags.txt).
All versions listed there should be safe to build, as we check that the CI
was green on them.

To see how our CI builds things, the relevant script is
[build.py](https://github.com/WebAssembly/waterfall/blob/master/src/build.py).
In general, the repos you need to build are LLVM and Binaryen (as emscripten
itself doesn't have any binaries to build). (Note: If you are packaging the older
fastcomp compiler backend then you must also get the fastcomp and fastcomp-clang
repos, and build them together to get LLVM and clang.)

When packaging build results, you need the following executables (when using
the wasm backend):

  * From LLVM:
    * clang
    * clang++ (note: this is a symlink to clang)
    * wasm-ld
    * llc
    * llvm-nm
    * llvm-ar
    * llvm-as
    * llvm-dis
    * llvm-dwarfdump
  * From Binaryen:
    * wasm-emscripten-finalize
    * wasm-opt
    * wasm-dis
    * wasm-as
    * wasm2js
    * wasm-metadce

(If you are packaging the older fastcomp backend instead of the wasm backend,
you don't need wasm-ld or wasm2js, and you do need llvm-link and opt.)

You also need to set up the `~/.emscripten` file for your users. Emscripten
will try to do so on first run if such a file does not exist; the simplest
thing is to look at those contents, edit the paths as needed if anything is
wrong, and then use that file. (You can also look at how the emsdk generates
the `.emscripten` file, which it does at the `activate step.) Some of the
key values in that file include:

 * `LLVM_ROOT`: The path to the LLVM binaries.
 * `BINARYEN_ROOT`: The path to binaryen (the binaries are expected in `/bin` under there; note that
    despite the name this differs from `LLVM_ROOT` which points directly to the binaries).
 * `NODE_JS`: The path to Node.js, which is needed internally.
 * `COMPILER_ENGINE`: The VM used internally for the JS compiler. Normally this should be `NODE_JS`.
 * `JS_ENGINES`: The full list of JS engines (or just `[NODE_JS]`). Used in the test suite.

