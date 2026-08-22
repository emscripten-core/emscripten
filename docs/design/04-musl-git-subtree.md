# Design Doc: Managing `system/lib/libc/musl` with `git subtree`

- **Status**: Draft

## Context

Currently, Emscripten manages its C standard library (`system/lib/libc/musl`)
via an out-of-tree workflow involving a separate GitHub fork
(https://github.com/emscripten-core/musl) and two synchronization scripts:

- `system/lib/push_musl_changes.py`: Copies local file modifications from
  `system/lib/libc/musl` out to a separate local checkout of the
  `emscripten-core/musl` repository.
- `system/lib/update_musl.py`: Deletes `system/lib/libc/musl` and copies a
  filtered subset of files from an external musl clone into Emscripten.

### Current Pain Points

1. **Desynchronization**: Changes made directly to `system/lib/libc/musl` in
   Emscripten PRs often get forgotten and are not pushed back to the
   `emscripten-core/musl` fork.
2. **Lost Git History & Attribution**: File copying via `shutil.copytree`
   discards commit history and commit metadata between the two repositories.
3. **Complex Update Process**: Upgrading musl requires juggling two checkouts,
   performing merges in the external repo, running copy scripts, and creating
   coordinated commits/PRs.
4. **Difficult Diffing**: Inspecting Emscripten's exact patchset against upstream
   musl requires manual comparisons across separate directories or
   repositories.

## Goals

- Manage `system/lib/libc/musl` directly inside the Emscripten repository using
  `git subtree`.
- Make day-to-day edits to musl in Emscripten self-contained (regular commits
  and PRs touch `system/lib/libc/musl` directly without external scripts).
- Enable single-command 3-way merges when upgrading to new upstream musl
  releases or cherry-picking fixes.
- Simplify diffing and patch auditing against upstream musl releases and tags.
- Deprecate and remove `system/lib/push_musl_changes.py` and
  `system/lib/update_musl.py`.

## Non-Goals

- Replacing other system libraries (like `compiler-rt` or `openmp`) with
  subtrees in this change.
- Switching to `git submodule` (which requires recursive clones and detached
  pointer management).

## Design

### 1. Subtree Architecture

We use `git subtree` with the `--squash` flag to vendor musl under the
`system/lib/libc/musl` prefix:

- Upstream musl repository: `git://git.musl-libc.org/musl` (or GitHub mirror
  https://github.com/bminor/musl)
- Prefix path: `system/lib/libc/musl`

```
       Upstream musl (git://git.musl-libc.org/musl)
                           │
             git fetch / git subtree pull
                           ▼
┌─────────────────────────────────────────────────────────────┐
│ emscripten-core/emscripten                                  │
│                                                             │
│  ├── system/lib/libc/musl/       ◄── Managed via subtree    │
│  │   ├── src/                    ◄── Tracked in-tree        │
│  │   └── include/                                           │
│  └── tools/system_libs.py        ◄── Builds needed objects  │
└─────────────────────────────────────────────────────────────┘
                           │
             git subtree push / split (Optional)
                           ▼
       Downstream Fork (github.com/emscripten-core/musl)
```

### 2. Key Design Decisions

#### A. Always Use `--squash`
Upstream musl has >10,000 commits over 15+ years. Without `--squash`, all
upstream history would pollute `git log` in Emscripten. With `--squash`, git
synthesizes a single commit representing the upstream tree snapshot at the merge
point while recording subtree metadata (`git-subtree-dir` and
`git-subtree-split`) in the commit message to maintain ancestry for future
merges.

#### B. Import the Full Musl Upstream Tree
Previously, `update_musl.py` excluded unused architectures (e.g. `arch/arm`,
`arch/mips`) and directories like `malloc/`.

With `git subtree`, we will track the complete upstream musl directory layout:
- Musl is lightweight (~4 MB total, ~300 KB compressed in git objects).
- Deleting upstream files creates artificial merge conflicts during future
  upstream pulls.
- `tools/system_libs.py` already selects and compiles only the specific files
  needed for WebAssembly, so uncompiled files do not affect output size.

## Subtree Initialization Plan

To transition cleanly without losing any existing Emscripten patches:

### Step 1: Configure Remote
```bash
git remote add musl-upstream git://git.musl-libc.org/musl
git fetch musl-upstream --tags
```

### Step 2: Prepare the Initial Subtree Branch
Create a branch based on upstream `v1.2.6` (the version currently in
Emscripten) and apply existing Emscripten local patches:

```bash
# Checkout base upstream tag
git checkout -b temp-musl-init v1.2.6

# Copy existing Emscripten musl directory over the tree
cp -r /path/to/emscripten/system/lib/libc/musl/* .
git add -A
git commit -m "Apply Emscripten patches on top of musl v1.2.6"
```

*(Optional: Push this branch to `emscripten-core/musl:emscripten` to keep the
fork synchronized).*

### Step 3: Add the Subtree in Emscripten
In the Emscripten repository branch:

```bash
# 1. Remove existing tracked files under system/lib/libc/musl
git checkout -b init-musl-subtree
git rm -rf system/lib/libc/musl
git commit -m "[libc] Remove un-tracked musl directory for subtree init"

# 2. Add subtree from the prepared branch
git subtree add --prefix=system/lib/libc/musl temp-musl-init --squash \
  -m "Initialize system/lib/libc/musl as git subtree tracking musl v1.2.6"

# 3. Verify tests pass
./test/runner test_hello_world

# 4. Remove obsolete sync scripts
git rm system/lib/push_musl_changes.py system/lib/update_musl.py
```

## Workflow Comparison

| Operation                  | Previous Workflow | New `git subtree` Workflow |
| :------------------------- | :---------------- | :------------------------- |
| **Day-to-day Patches**     | Edit in Emscripten, run `push_musl_changes.py`, push to fork. | **Edit files directly in Emscripten and commit.** |
| **Upstream Updates**       | Merge in fork repo, resolve conflicts, run `update_musl.py`. | **Run `git subtree pull --squash` directly in Emscripten.** |
| **Conflict Resolution**    | Done in external clone, then copied blindly via script. | Resolved in-place in Emscripten with standard git tools. |
| **Auditing Diffs**         | Manually diffing two separate checkouts. | `git diff v1.2.6: HEAD:system/lib/libc/musl` |
| **Single Source of Truth** | Split between fork repo and Emscripten repo. | **Emscripten repository is the single source of truth.** |

## Common Operations Guide

### 1. Showing Diffs Against Upstream musl

#### Diff against an upstream tag:
```bash
git fetch musl-upstream --tags
# Diff entire subtree against upstream v1.2.6
git diff v1.2.6: HEAD:system/lib/libc/musl

# Diff a specific file or subdirectory
git diff v1.2.6:src/stdio HEAD:system/lib/libc/musl/src/stdio
git diff v1.2.6:include/stdio.h HEAD:system/lib/libc/musl/include/stdio.h
```

#### Extract Emscripten patches as standalone commits:
```bash
git subtree split --prefix=system/lib/libc/musl -b musl-local-branch
git log v1.2.6..musl-local-branch
git diff v1.2.6..musl-local-branch
```

### 2. Updating to a New Upstream musl Release (e.g. `v1.2.7`)

```bash
git fetch musl-upstream --tags
git subtree pull --prefix=system/lib/libc/musl musl-upstream v1.2.7 --squash \
  -m "Update musl to v1.2.7"

# Resolve any merge conflicts if necessary
# Run libc tests
./test/runner test_libc*
```

### 3. Cherry-picking an Upstream Fix

```bash
git fetch musl-upstream
git subtree pull --prefix=system/lib/libc/musl musl-upstream <commit-sha> --squash \
  -m "Backport upstream musl commit <commit-sha>"
```

### 4. Exporting Local Patches to the Fork or Upstream

```bash
# Push directly to GitHub fork
git subtree push --prefix=system/lib/libc/musl emscripten-musl emscripten

# Or generate patches to submit to musl mailing list
git subtree split --prefix=system/lib/libc/musl -b musl-export
git format-patch musl-upstream/master..musl-export
```
