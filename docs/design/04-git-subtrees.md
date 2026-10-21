# Design Doc: Managing External Libraries with `git subtree`

- **Status**: Draft

## Context

Emscripten vendors several external libraries under `system/lib/` to provide
core runtime capabilities, including C/C++ standard libraries, memory
allocators, and compiler runtimes. Currently, these libraries are maintained
through disparate and often cumbersome mechanisms:

1. **External Fork + Synchronization Scripts**:
   - `musl` (`system/lib/libc/musl`): Maintained via an external GitHub fork
     (https://github.com/emscripten-core/musl) and two sync scripts:
     `system/lib/push_musl_changes.py` and `system/lib/update_musl.py`.
   - **LLVM Runtimes** (`compiler-rt`, `libcxx`, `libcxxabi`, `libunwind`,
     `llvm-libc`, `openmp`): Synced from branches in
     `emscripten-core/llvm-project` using `system/lib/push_llvm_changes.py` and
     individual `update_*.py` scripts.

2. **Manual File Copying**:
   - `mimalloc` (`system/lib/mimalloc`): Tracked in-tree with
     Emscripten-specific primitives, but updated periodically by copying new
     upstream releases directly into the directory without automated tooling.

3. **Static Single-File Vendoring**:
   - `dlmalloc` (`system/lib/dlmalloc.c`) and `stb_image`
     (`system/lib/stb_image.c`): Single source files with low churn.

### Current Pain Points

1. **Desynchronization**: When changes are made directly to vendored files
   in Emscripten PRs (especially in `system/lib/libc/musl`), authors often
   forget to backport them to the downstream fork repo.
2. **Lost Git History & Attribution**: Copying trees using `shutil.copytree` or
   manual file replacement discards commit history, author attribution, and
   intermediate change context between repositories.
3. **Complex Update Process**: Updating an external library requires juggling
   multiple local checkouts, performing merges in external repositories, running
   custom copy scripts, and opening coordinated PRs across repositories.
4. **Difficult Diffing & Auditing**: Inspecting the exact delta between
   Emscripten's vendored code and upstream releases requires manual comparisons
   across disparate clones.

### Prior Art

Managing vendored system libraries via `git subtree` was recently and
successfully adopted in `wasi-libc`:
https://github.com/WebAssembly/wasi-libc/pull/797

## Goals

- Standardize the management of external in-tree libraries using `git subtree`.
- Make day-to-day edits to vendored libraries self-contained within the
  Emscripten repository (standard commits and PRs directly touch vendored
  directories without requiring out-of-tree scripts).
- Enable single-command 3-way merges when upgrading to new upstream releases or
  cherry-picking fixes (`git subtree pull --squash`).
- Simplify diffing and patch auditing against upstream releases and tags
  (`git diff <tag>: HEAD:<prefix>`).
- Establish a staged rollout, starting with a low-risk, self-contained pilot
  library (**`mimalloc`**), followed by **`musl`**, and later evaluating LLVM
  runtime libraries.
- Deprecate and remove custom synchronization scripts (such as
  `system/lib/push_musl_changes.py` and `system/lib/update_musl.py`).

## Non-Goals

- Migrating all external libraries simultaneously in a single large change.
- Switching to `git submodule` (submodules require recursive clones, detached
  `HEAD` management, and complicate contributor workflows).
- Replacing single-file, highly customized or dormant vendored code (like
  `dlmalloc.c`) where upstream churn is negligible.

## Candidate Libraries Analysis

| Library | In-Tree Path | Upstream Repository | Current Workflow | Complexity / Recommendation |
| :--- | :--- | :--- | :--- | :--- |
| **`mimalloc`** | `system/lib/mimalloc` | [microsoft/mimalloc](https://github.com/microsoft/mimalloc) | Manual copy-paste | **Recommended Pilot**. Dedicated repo, cleanly tagged releases, small footprint, self-contained. |
| **`musl`** | `system/lib/libc/musl` | [git.musl-libc.org/musl](git://git.musl-libc.org/musl) | Fork + `update_musl.py` | **High Priority**. Follows `wasi-libc` precedent. Eliminates sync scripts and perpetual fork drift. |
| **`compiler-rt`** | `system/lib/compiler-rt` | [llvm/llvm-project](https://github.com/llvm/llvm-project) | Fork + `update_compiler_rt.py` | **Medium Priority**. Upstream is a monorepo; needs subtree split or tracking a branch/fork. |
| **`libcxx` / `libcxxabi`** | `system/lib/libcxx{,abi}` | [llvm/llvm-project](https://github.com/llvm/llvm-project) | Fork + `update_libcxx*.py` | **Medium Priority**. Same monorepo considerations as `compiler-rt`. |
| **`libunwind`** | `system/lib/libunwind` | [llvm/llvm-project](https://github.com/llvm/llvm-project) | Fork + `update_libunwind.py` | **Medium Priority**. Same monorepo considerations as `compiler-rt`. |
| **`llvm-libc`** | `system/lib/llvm-libc` | [llvm/llvm-project](https://github.com/llvm/llvm-project) | Fork + `update_llvm_libc.py` | **Medium Priority**. Same monorepo considerations as `compiler-rt`. |
| **`openmp`** | `system/lib/openmp` | [llvm/llvm-project](https://github.com/llvm/llvm-project) | Fork + `update_openmp.py` | **Medium Priority**. Same monorepo considerations as `compiler-rt`. |
| **`dlmalloc`** | `system/lib/dlmalloc.c` | Doug Lea (v2.8.6) | Vendored single file | **Low Priority / Skip**. Upstream is inactive; heavily customized for Emscripten. |
| **`stb_image`** | `system/lib/stb_image.c` | [nothings/stb](https://github.com/nothings/stb) | Vendored single file | **Low Priority**. Low update frequency; single file. |

### Why Start with `mimalloc`?

We recommend starting the `git subtree` migration with **`mimalloc`** as a pilot
before applying it to `musl`:

1. **Isolated & Low Risk**: Unlike libc, an issue in `mimalloc` does not risk
   breaking the fundamental runtime or core toolchain startup.
2. **Dedicated, Clean Upstream**: Upstream is a standalone GitHub repository
   (`microsoft/mimalloc`) with standard release tags (`v3.5.1`, `v3.4.1`,
   etc.), making remote management straightforward.
3. **No Legacy Fork Baggage**: `mimalloc` does not have an external
   `emscripten-core` fork repo or existing sync scripts to unwind; it has been
   updated purely by manual file copying.
4. **Small & Contained Patchset**: Emscripten-specific support is localized to
   `system/lib/mimalloc/src/prim/emscripten/prim.c` (much of which is already
   upstreamed) and compiler flag adjustments in `tools/system_libs.py`.
5. **High Active Churn**: `mimalloc` releases regularly (v3.3.1 -> v3.4.1 ->
   v3.5.1 in recent months). Adopting `git subtree` provides immediate value for
   future version bumps.

## Design

### 1. Subtree Architecture (Example: `mimalloc`)

We use `git subtree` with the `--squash` flag to vendor external libraries under
their designated directory prefixes:

- Upstream repository: `https://github.com/microsoft/mimalloc`
- In-tree prefix path: `system/lib/mimalloc`

```
       Upstream (https://github.com/microsoft/mimalloc)
                            │
              git fetch / git subtree pull
                            ▼
┌─────────────────────────────────────────────────────────────┐
│ emscripten-core/emscripten                                  │
│                                                             │
│  ├── system/lib/mimalloc/        ◄── Managed via subtree    │
│  │   ├── include/                ◄── Tracked in-tree        │
│  │   └── src/                                               │
│  │       └── prim/emscripten/    ◄── Emscripten primitives  │
│  └── tools/system_libs.py        ◄── Compiles libmimalloc   │
└─────────────────────────────────────────────────────────────┘
                            │
              git subtree split / push (Optional)
                            ▼
       Upstream PR / Fork (e.g. microsoft/mimalloc)
```

### 2. Key Design Decisions

#### A. Always Use `--squash`
Upstream repositories often have tens of thousands of commits. Without
`--squash`, the full upstream history is imported directly into Emscripten's
git log. With `--squash`, git synthesizes a single commit representing the
upstream snapshot at the merge point, while embedding subtree metadata
(`git-subtree-dir` and `git-subtree-split`) in the commit message to maintain
proper ancestry for future 3-way merges.

#### B. Track the Full Upstream Directory Layout
When importing an upstream repository via subtree, retain its upstream
directory structure:
- Upstream libraries like `mimalloc` and `musl` are compact (a few megabytes).
- Pruning or excluding unused directories creates artificial merge conflicts
  during future `git subtree pull` operations.
- Build logic in `tools/system_libs.py` explicitly selects which source files
  are compiled for WebAssembly, ensuring uncompiled files do not affect binary
  output or cache sizes.

#### C. Emscripten as the Single Source of Truth
Local changes, bug fixes, and porting adaptations are committed directly to
the Emscripten repository. Separate fork repositories (like
`emscripten-core/musl`) no longer serve as intermediate synchronization hubs.
When local patches are ready to be contributed upstream, `git subtree split` can
extract them cleanly into a branch suitable for a pull request to the upstream
project.

#### D. Remote & Tag Management (Preventing Tag Collisions)
By default, `git fetch` and `--tags` import all upstream tags (e.g. `v1.2.6`,
`v3.5.1`) into the repository's local `refs/tags/*` namespace. This presents
two problems:
1. **Tag Collisions & Clutter**: Generic version tags from different upstreams
   (e.g. `v1.0`, `v2.0`) can collide with each other or clutter Emscripten's
   tag list and shell autocompletion.
2. **Polluting `git describe`**: Automated tooling and build scripts that run
   `git describe --tags` to determine the compiler version can pick up reachable
   upstream tags instead of Emscripten release tags.

To prevent foreign tag pollution, we adopt the following conventions:

1. **Configure Remotes with `--no-tags`**:
   Subtree remotes should be added with `--no-tags` (or set
   `remote.<name>.tagOpt = --no-tags`):
   ```bash
   git remote add --no-tags mimalloc-upstream \
     https://github.com/microsoft/mimalloc
   ```
   `git subtree pull` does not require local tags to exist; Git can fetch the
   specified tag or commit into `FETCH_HEAD` on demand without storing it in
   `refs/tags/*`.

2. **Namespaced Refspecs (Optional for Local Tag Access)**:
   Maintainers who want upstream tags available locally (e.g. for diffing) can
   namespace them outside of `refs/tags/*`:
   ```bash
   # Isolate under refs/remotes/ (hidden from git tag and git describe)
   git config --add remote.mimalloc-upstream.fetch \
     '+refs/tags/*:refs/remotes/mimalloc-upstream/tags/*'
   ```
   Alternatively, prefix them inside `refs/tags/` using a subdirectory:
   ```bash
   git config --add remote.mimalloc-upstream.fetch \
     '+refs/tags/*:refs/tags/mimalloc/*'
   ```

3. **URL-Based Ephemeral Pulls (The wasi-libc Model)**:
   Contributors do not need upstream remotes configured in everyday checkouts.
   Maintainers performing a version update can pull directly from the repository
   URL:
   ```bash
   git subtree pull --prefix=system/lib/mimalloc \
     https://github.com/microsoft/mimalloc v3.5.2 --squash \
     -m "Update mimalloc to v3.5.2"
   ```
   This imports the squashed commit without leaving any foreign remotes or tags
   behind.

## Migration Guide & Example: `mimalloc`

Here is the step-by-step process for initializing `mimalloc` as a `git subtree`
without losing existing modifications:

### Step 1: Configure the Upstream Remote
```bash
git remote add --no-tags mimalloc-upstream https://github.com/microsoft/mimalloc
git fetch mimalloc-upstream tag v3.5.1
```

### Step 2: Prepare the Initial Branch with Local Patches
Create a temporary branch based on upstream `v3.5.1` (the version currently in
Emscripten) and overlay our current in-tree files:

```bash
# Check out base upstream release tag
git checkout -b temp-mimalloc-init FETCH_HEAD

# Overlay current Emscripten mimalloc directory to preserve local patches
cp -r /path/to/emscripten/system/lib/mimalloc/* .
git add -A
git commit -m "Apply Emscripten modifications on top of mimalloc v3.5.1"
```

### Step 3: Initialize the Subtree in Emscripten
In the Emscripten working branch:

```bash
git checkout -b init-mimalloc-subtree

# 1. Remove previously un-tracked mimalloc directory
git rm -rf system/lib/mimalloc
git commit -m "[lib] Remove loose mimalloc files for subtree initialization"

# 2. Add the subtree using the prepared branch
git subtree add --prefix=system/lib/mimalloc temp-mimalloc-init --squash \
  -m "Initialize system/lib/mimalloc as git subtree tracking mimalloc v3.5.1"

# 3. Clean up the temporary branch
git branch -D temp-mimalloc-init
```

### Step 4: Verify with Tests
```bash
./test/runner test_other.test_mimalloc_headers
./test/runner test_other.test_mimalloc_no_asan
./test/runner test_hello_world -sMALLOC=mimalloc
```

## Common Operations Guide (Using `mimalloc`)

### 1. Showing Diffs Against Upstream
Inspect the exact diff between Emscripten's tree and an upstream tag:

```bash
git fetch --no-tags mimalloc-upstream tag v3.5.1

# Diff entire subtree against upstream v3.5.1
git diff FETCH_HEAD: HEAD:system/lib/mimalloc

# Diff a specific file
git diff v3.5.1:src/prim/emscripten/prim.c \
  HEAD:system/lib/mimalloc/src/prim/emscripten/prim.c
```

To view Emscripten's changes as individual commit logs:
```bash
git subtree split --prefix=system/lib/mimalloc -b mimalloc-local-changes
git log v3.5.1..mimalloc-local-changes
```

### 2. Updating to a New Upstream Release (e.g. `v3.5.2`)
Updating to a new version is a single command:

```bash
# Option A: Pull by remote name (with remote configured with --no-tags)
git subtree pull --prefix=system/lib/mimalloc \
  mimalloc-upstream v3.5.2 --squash \
  -m "Update mimalloc to v3.5.2"

# Option B: Pull directly by repository URL (no local remote or tags stored)
git subtree pull --prefix=system/lib/mimalloc \
  https://github.com/microsoft/mimalloc v3.5.2 --squash \
  -m "Update mimalloc to v3.5.2"

# Resolve any merge conflicts in-place if necessary
# Run tests
./test/runner test_other.test_mimalloc_headers
```

### 3. Cherry-Picking an Upstream Bugfix
Backporting an upstream commit before a new release:

```bash
git fetch mimalloc-upstream
git subtree pull --prefix=system/lib/mimalloc \
  mimalloc-upstream <commit-sha> --squash \
  -m "Backport upstream mimalloc commit <commit-sha>"
```

### 4. Contributing Local Fixes Upstream
Extract local Emscripten commits into a standalone branch for a pull request to
`microsoft/mimalloc`:

```bash
git subtree split --prefix=system/lib/mimalloc -b mimalloc-upstream-pr
# Push to personal GitHub fork and open upstream PR
git push git@github.com:<user>/mimalloc.git mimalloc-upstream-pr:my-fix
```

## Next Step: Applying to `musl`

Following successful validation with `mimalloc`, the same process applies to
`system/lib/libc/musl`:

1. Configure remote:
   ```bash
   git remote add --no-tags musl-upstream git://git.musl-libc.org/musl
   git fetch musl-upstream tag v1.2.6
   ```
2. Prepare initial branch based on `v1.2.6` (overlaying `system/lib/libc/musl`).
3. Replace `system/lib/libc/musl` with:
   ```bash
   git subtree add --prefix=system/lib/libc/musl temp-musl-init --squash \
     -m "Initialize system/lib/libc/musl as git subtree tracking musl v1.2.6"
   ```
4. Run libc test suites (`./test/runner test_libc*`).
5. **Delete obsolete synchronization scripts**:
   ```bash
   git rm system/lib/push_musl_changes.py system/lib/update_musl.py
   ```

## Evaluating LLVM Runtime Libraries

Migrating LLVM runtimes (`compiler-rt`, `libcxx`, `libcxxabi`, `libunwind`,
`llvm-libc`, `openmp`) presents unique considerations because upstream is a
multi-gigabyte monorepo (`llvm/llvm-project`):

1. **Subtree Against Monorepo Subdirectories**:
   Running `git subtree pull` directly against the monorepo root is impractical
   because each library resides in a nested subdirectory (`runtimes/`,
   `libcxx/`, etc.), and pulling the root repository would drag in the entire
   LLVM history.
2. **Intermediate Split Branches**:
   A clean approach for LLVM runtimes is to maintain lightweight split branches
   in `emscripten-core/llvm-project` (or using GitHub Actions to generate
   per-runtime subtree branches corresponding to upstream release tags, e.g.
   `llvmorg-22.1.8-libcxx`).
3. **Phased Approach**:
   Given these considerations, LLVM runtimes should be evaluated in Phase 3
   after establishing experience with `mimalloc` and `musl`. Until then, the
   existing `update_*.py` and `push_llvm_changes.py` scripts can continue
   operating.

## Workflow Comparison

| Operation | Previous Workflow | New `git subtree` Workflow |
| :--- | :--- | :--- |
| **Day-to-day Patches** | Edit in Emscripten, run push script, push to fork repo. | **Edit files directly in Emscripten and commit.** |
| **Upstream Updates** | Merge in fork repo, resolve conflicts, run copy script. | **Run `git subtree pull --squash` directly in Emscripten.** |
| **Conflict Resolution** | Done in external clone, then copied blindly via script. | Resolved in-place in Emscripten with standard git tools. |
| **Auditing Diffs** | Manually diffing two separate checkouts. | `git diff <tag>: HEAD:<prefix>` |
| **Single Source of Truth**| Split between fork repos and Emscripten repo. | **Emscripten repository is the single source of truth.** |

## Rollout Roadmap

1. **Phase 1 (Pilot)**: Initialize `mimalloc` as a git subtree. Validate
   day-to-day development, local patches, and an upstream release bump.
2. **Phase 2 (`musl`)**: Convert `system/lib/libc/musl` to a git subtree
   following the proven pattern and retire `push_musl_changes.py` and
   `update_musl.py`.
3. **Phase 3 (LLVM Runtimes Evaluation)**: Investigate migrating `compiler-rt`,
   `libcxx`, and other LLVM runtimes, addressing monorepo subtree extraction
   strategy.
