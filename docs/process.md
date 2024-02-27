# Development Processes

More information can be found in the [Contribution
section](https://emscripten.org/docs/contributing/contributing.html) of the
webside.

## Landing PRs

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
 * Add `NFC` to the end of the PR title for Non-Functional Changes (i.e.,
   changes that do not add/modify functionality, such as internal refactoring).
 * Add a `[prefix]` to start of the PR title to signify the subsystem or area
   that the PR targets. e.g. `[test] Update foo test` or `[ports] Fix zlib port`

## Coding Style

### C/C++ Code

When writing new C/C++ in emscripten follow the LLVM style (as does binaryen).
You can use `clang-format` to automatically format new code (and `git
clang-format origin/main` to format just the lines you are changing).
See [`.clang-format`][clang-format] for more details.

When editing third party code such (e.g. musl, libc++) follow the upstream
conventions.

### JavaScript Code

We use the same LLVM-based style as for C/C++.  Sadly, `clang-format` doesn't
always work well with our library code since it can use custom macros and
pre-processor.  See [`.clang-format`][clang-format] for more details.

### Python Code

We generally follow the pep8 standard with the major exception that we use 2
spaces for indentation.  `flake8` is run on all PRs to ensure that python code
conforms to this style.  See [`.flake8`][flake8] for more details.

#### Static Type Checking

We are beginning to use python3's type annotation syntax, along with the `mypy`
tool to check python types statically.  See [`.mypy`][mypy] for more details.

The goal is to one day check all type by running `mypy` with
`--disallow-untyped-defs`, but this is happening incrementally over time.

# Release Processes

## Minor version updates (1.X.Y to 1.X.Y+1)

When:

 * Such an update ensures we clear the cache, so it should be done when required
   (for example, a change to libc or libc++).
 * The emsdk compiled versions are based on the version number, so periodically
   we can do this when we want a new precompiled emsdk version to be available.

Requirements:

 * [emscripten-releases build CI][waterfall] is green on all OSes for the
   desired hash (where the hash is the git hash in the
   [emscripten-releases][releases_repo] repo, which then specifies through
   [DEPS][DEPS] exactly which revisions to use in all other repos).
 * [GitHub CI](https://github.com/emscripten-core/emscripten/branches) is green
   on the `main` branch for the emscripten commit referred to in [DEPS][DEPS].

How:

1. Pick a version for a release and make sure it meets the requirements above.
   Let this version SHA be `<non-LTO-sha>`.
1. If we want to do an LTO release as well, create a CL that copies [DEPS][DEPS]
   from <non-lto-sha> to [DEPS.tagged-release][DEPS.tagged-release] in
   [emscripten-releases][releases_repo] repo. When this CL is committed, let the
   resulting SHA be `<LTO-sha>`. An example of this CL is
   https://chromium-review.googlesource.com/c/emscripten-releases/+/3781978.
   After landing the CL, wait for a couple hours before proceeding because
   building and archiving for the new commit will take some time. Check
   https://ci.chromium.org/p/emscripten-releases/g/main/console to see if the
   commit has passed "Archive Binaries" phase for all three platforms and
   additionally "Archive Binaries (arm64)" for Mac.
1. Run [`scripts/create_release.py`][create_release_emsdk] in the emsdk
   repository. When we do both an LTO and a non-LTO release, run:
   ```
   ./scripts/create_release.py <LTO-sha> <non-LTO-sha>
   ```
   This will make the `<LTO-sha>` point to the versioned name release (e.g.
   `3.1.7`) and the `<non-LTO-sha>` point to the assert build release (e.g.
   `3.1.7-asserts`). When we do only a non-LTO release, run:
   ```
   ./scripts/create_release.py <non-LTO-sha>
   ```
   This will make the `<non-LTO-sha>` point directly to the versioned name
   release (e.g. `3.1.7`) and there will be no assert build release. If we run
   [`scripts/create_release.py`][create_release_emsdk] without any arguments, it
   will automatically pick a tot version from
   [emscripten-releases][releases_repo] repo and make it point to the versioned
   name release. Running this
   [`scripts/create_release.py`][create_release_emsdk] script will update
   [emscripten-releases-tags.json][emscripten_releases_tags], adding a new
   version. The script will create a new local git branch and push it up to
   ``origin``.  An example of this PR is emscripten-core/emsdk#1071.
1. [Tag][emsdk_tags] the `emsdk` repo with the new version number, on the commit
   that does the update, after it lands on main.
1. [Tag][emscripten_tags] the `emscripten` repo with the new version number, on
   the commit referred to in the [DEPS][DEPS] (or DEPS.tagged-release) file
   above.
1. Run the [`tools/maint/create_release.py`][create_release_emscripten]
   tool in the emscripten repo to update
   [`emscripten-version.txt`][emscripten_version] and
   [`ChangeLog.md`][changelog].  An example of such PR is
   emscripten-core/emscripten#17439.

## Major version update (1.X.Y to 1.(X+1).0)

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


## Updating the `emscripten.org` Website

The site is currently hosted in `gh-pages` branch of the separate [site
repository][site_repo]. To update the docs, rebuild them and copy them into
this repository.  There is a script that will perform these steps automatically:
[`tools/maint/update_docs.py`][update_docs].  Just run this script with no
arguments if the emscripten-site repository is checked out alongside emscripten
itself, or pass the location of the checkout if not.

You will need the specific sphinx version installed, which you can do using
`pip3 install -r requirements-dev.txt` (depending on your system, you may then
need to add `~/.local/bin` to your path, if pip installs to there).


### Building and viewing the Website locally

To build the site locally for testing purposes you only need a subset of the
`update_docs.py` command just mentioned above. Specifically:

1. Run `pip3` to install python dependencies, as described above.
2. Run `make -C site html`.
3. Run a local webserver on the outout of that command, e.g.,
   `python3 -m http.server 8000 -d site/build/html`.
4. Browse to `http://localhost:8000/` (assuming you use port 8000 as above).


## Updating the `emcc.py` help text

`emcc --help` output is generated from the main documentation under `site/`,
so it is the same as shown on the website, but it is rendered to text. After
updating `emcc.rst` in a PR, the following should be done:

1. In your emscripten repo checkout, enter `site`.
2. Run `make clean` (without this, it may not emit the right output).
2. Run `make text`.
3. Copy the output `build/text/docs/tools_reference/emcc.txt` to
   `../docs/emcc.txt` (both paths relative to the `site/` directory in
   emscripten that you entered in step 1), and add that change to your PR.

See notes above on installing sphinx.


## Updating the LLVM libraries

We maintain our ports of compiler-rt, libcxx, libcxxabi, and libunwind under
https://github.com/emscripten-core/emscripten/tree/main/system/lib from
[the upstream LLVM repository][llvm_repo] and periodically update them to a newer
version when a new LLVM release comes out.

We maintain [a fork of LLVM][llvm_emscripten_fork] for library updates, where we
create a branch for each new LLVM major release. For example, the branch for
LLVM 16 is
https://github.com/emscripten-core/llvm-project/tree/emscripten-libs-16. We
create a new branch for a major version update and reuse the existing branch for
a minor version update. We mostly do updates per LLVM major release.

To update our libraries to a newer LLVM release:

1. If you are updating an existing branch the first step is to run
   [`push_llvm_changes.py`][push_llvm_changes_emscripten] to make sure the
   current branch is up-to-date with the current emscripten codebase.
   ```
   ./system/lib/push_llvm_changes.py <Emscripten's LLVM fork directory>
   ```
   (The existing library branch should be checked out in your Emscripten's LLVM
   fork directory.)
   An example of such PR is emscripten-core/llvm-project#5.

   If you are creating a new branch, first make sure the previous/existing
   branch is up-to-date using
   [`push_llvm_changes.py`][push_llvm_changes_emscripten]. Then
   create the new branch and cherry-pick all the emscripten-specific changes
   from the old branch, resolving any conflicts that might arise.

   In either case, once that branch is up-to-date use the update scripts to copy
   the llvm branch contents into the emscripten tree. Its important in both
   cases to run [`push_llvm_changes.py`][push_llvm_changes_emscripten] first to
   ensure that no emscripten changes are lost in the process.
1. Create a PR to merge new LLVM release tag in the upstream repo into our new
   library branch. For example, if we want to merge `llvmorg-16.0.6` tag into
   our `emscripten-libs-16` branch, you can do
   ```
   git co emscripten-libs-16
   git remote add upstream git@github.com:llvm/llvm-project.git
   git fetch --tags upstream
   git merge llvmorg-16.0.6
   ```
   An example of such PR is emscripten-core/llvm-project#3.
1. Now we have merged all the changes to our LLVM fork branch, pull those
   changes with the new version back into the Emscripten repo. You can use
   [`update_compiler_rt.py`][update_compiler_rt_emscripten],
   [`update_libcxx.py`][update_libcxx_emscripten],
   [`update_libcxxabi.py`][update_libcxxabi_emscripten],
   [`update_libunwind.py`][update_libunwind_emscripten] for that. For example,
   ```
   ./system/lib/update_comiler_rt.py <Emscripten's LLVM fork directory>
   ```
   (The library branch should be checked out in your Emscripten's LLVM fork
   directory.)
   An example of such PR is emscripten-core/emscripten#19515.


## Updating musl

We maintain our musl in
https://github.com/emscripten-core/emscripten/tree/main/system/lib/libc/musl.
We maintain a fork of musl in https://github.com/emscripten-core/musl for
updates and periodically update it to a newer version.

The process for updating musl is similar to that of updating the LLVM libraries.
To update our libraries to a newer musl release:

1. If you are updating an existing branch the first step is to run
   [`push_musl_changes.py`][push_musl_changes_emscripten] to make sure the
   current branch is up-to-date with the current emscripten codebase.

   If you are creating a new branch, first make sure the previous/existing
   branch is up-to-date using
   [`push_musl_changes.py`][push_musl_changes_emscripten]. Then
   create the new branch and cherry-pick all the emscripten-specific changes
   from the old branch, resolving any conflicts that might arise.
1. Create a PR to merge new mrelease tag in the upstream repo into our new
   library branch. For example, if we want to merge musl's `v1.2.4` tag into our
   `merge-v1.2.4` branch, you can do
   ```
   git co merge-v1.2.4
   git remote add upstream git://git.musl-libc.org/musl
   git fetch --tags upstream
   git merge v1.2.4
   ```
1. Now we have merged all the changes to our musl fork branch, pull those
   changes with the new version back into the Emscripten repo. You can use
   [`update_musl.py`][update_musl_emscripten] for that.


[site_repo]: https://github.com/kripken/emscripten-site
[releases_repo]: https://chromium.googlesource.com/emscripten-releases
[waterfall]: https://ci.chromium.org/p/emscripten-releases/g/main/console
[emscripten_version]: https://github.com/emscripten-core/emscripten/blob/main/emscripten-version.txt
[changelog]: https://github.com/emscripten-core/emscripten/blob/main/ChangeLog.md
[create_release_emsdk]: https://github.com/emscripten-core/emsdk/blob/main/scripts/create_release.py
[create_release_emscripten]: https://github.com/emscripten-core/emscripten/blob/main/tools/maint/create_release.py
[emscripten_releases_tags]: https://github.com/emscripten-core/emsdk/blob/main/emscripten-releases-tags.json
[DEPS]: https://chromium.googlesource.com/emscripten-releases/+/refs/heads/main/DEPS
[DEPS.tagged-release]: https://chromium.googlesource.com/emscripten-releases/+/refs/heads/main/DEPS.tagged-release
[emsdk_tags]: https://github.com/emscripten-core/emsdk/tags
[emscripten_tags]: https://github.com/emscripten-core/emscripten/tags
[clang-format]: https://github.com/emscripten-core/emscripten/blob/main/.clang-format
[flake8]: https://github.com/emscripten-core/emscripten/blob/main/.flake8
[mypy]: https://github.com/emscripten-core/emscripten/blob/main/.mypy
[update_docs]: https://github.com/emscripten-core/emscripten/blob/main/tools/maint/update_docs.py
[llvm_repo]: https://github.com/llvm/llvm-project
[llvm_emscripten_fork]: https://github.com/emscripten-core/llvm-project
[push_llvm_changes_emscripten]: https://github.com/emscripten-core/emscripten/blob/main/system/lib/push_llvm_changes.py
[push_musl_changes_emscripten]: https://github.com/emscripten-core/emscripten/blob/main/system/lib/push_musl_changes.py
[update_compiler_rt_emscripten]: https://github.com/emscripten-core/emscripten/blob/main/system/lib/update_compiler_rt.py
[update_libcxx_emscripten]: https://github.com/emscripten-core/emscripten/blob/main/system/lib/update_libcxx.py
[update_libcxxabi_emscripten]: https://github.com/emscripten-core/emscripten/blob/main/system/lib/update_libcxxabi.py
[update_libunwind_emscripten]: https://github.com/emscripten-core/emscripten/blob/main/system/lib/update_libunwind.py
[update_musl_emscripten]: https://github.com/emscripten-core/emscripten/blob/main/system/lib/update_musl.py
