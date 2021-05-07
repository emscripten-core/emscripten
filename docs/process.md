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

 * [emscripten-releases build CI][waterfall] is green on all OSes for the
   desired hash (where the hash is the git hash in the
   [emscripten-releases][releases_repo] repo, which then specifies through
   [DEPS][DEPS] exactly which revisions to use in all other repos).
 * [GitHub CI](https://github.com/emscripten-core/emscripten/branches) is green
   on the `main` branch.

How:

1. Run [`./scripts/create_release.py`][create_release] in the emsdk repository.
   This script will update [emscripten-releases-tags.txt][emscripten_releases_tags],
   adding a new version.  You can either specify the desired hash, or let the
   script pick the current tot build.  The script will create a new git branch
   that can be uploaded as a PR.
3. Tag the `emsdk` repo with the new version number, on the commit that does the
   update, after it lands on main.
4. Tag the `emscripten` repo with the new version number, on the commit referred
   to in the [DEPS][DEPS] file above.
5. Update [`emscripten-version.txt`][emscripten_version] and
   [`ChangeLog.md`][changelog] in the emscripten repo to refer the next,
   upcoming, version.

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
repository][site_repo]. To update the docs, rebuild them and copy them there,
that is:

1. In your emscripten repo checkout, enter `site`.
2. Run `make html`.
3. Run `make install EMSCRIPTEN_SITE=\[path-to-a-checkout-of-the-site-repo\]`
3. Go to the site repo, commit the changes, and push.



Updating the `emcc.py` help text
--------------------------------

`emcc --help` output is generated from the main documentation under `site/`,
so it is the same as shown on the website, but it is rendered to text. After
updating `emcc.rst` in a PR, the following should be done:

1. In your emscripten repo checkout, enter `site`.
2. Run `make clean` (without this, it may not emit the right output).
2. Run `make text`.
3. Add the changes to your PR.


[site_repo]: https://github.com/kripken/emscripten-site
[releases_repo]: https://chromium.googlesource.com/emscripten-releases
[waterfall]: https://ci.chromium.org/p/emscripten-releases/g/main/console
[emscripten_version]: https://github.com/emscripten-core/emscripten/blob/main/emscripten-version.txt
[changelog]: https://github.com/emscripten-core/emscripten/blob/main/ChangeLog.md
[create_release]: https://github.com/emscripten-core/emsdk/blob/main/scripts/create_release.py
[emscripten_releases_tags]: https://github.com/emscripten-core/emsdk/blob/main/emscripten-releases-tags.txt
[DEPS]: https://chromium.googlesource.com/emscripten-releases/+/refs/heads/master/DEPS
