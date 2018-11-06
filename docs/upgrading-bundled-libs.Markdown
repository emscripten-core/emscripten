
Upgrading Bundled System Libraries
==================================

This includes musl, libc++, and others whose source is bundled in our repo
for simplicity.

To update them,

 * Update the files in `system/` with the new version's files. For example,
   for musl that means `system/lib/libc/musl` and `system/include/libc`. You
   need to handle Emscripten's local changes, which are marked with
   `XXX EMSCRIPTEN` comments, may need to handle merge conflicts there. It is
   useful to search the git diff for `EMSCRIPTEN` to see those areas.
 * Update the list of symbols. Those symbols are scanned by the linker in order
   to know whether to link in a system library or not (so a missing symbol
   may cause linking to not include something you need). The files are under
   `system/lib`/ and have the suffix `.symbol`. To generate the new contents,
   build the system library with the new files, and do something like this
   for musl:
   `~/Dev/fastcomp/build/bin/llvm-nm ~/.emscripten_cache/asmjs/libc.bc | sort`
   and write that to the `.symbol` file. Take a look at the git diff to see
   if anything looks odd.
 * Run the test suite to make sure everything works. You can open a PR and let
   the continuous integration run it for you.

