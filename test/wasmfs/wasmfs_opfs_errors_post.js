async function run_test() {
  let access;
  try {
    let root = await navigator.storage.getDirectory();
    let child = await root.getFileHandle("data", {create: true});
    access = await child.createSyncAccessHandle();
  } catch (e) {
    console.log("test setup failed");
    throw e;
  }

  // We won't be able to open the file for writing since there is already a
  // file handle for it.

  if (Module._try_open_wronly() != 0) {
    throw "Did not get expected EACCES opening file for writing";
  }

  if (Module._try_open_rdwr() != 0) {
    throw "Did not get expected EACCES opening file for reading and writing";
  }

  if (Module._try_open_rdonly() != 1) {
    throw "Unexpected failure opening file for reading only";
  }

  if (Module._try_truncate() != 0) {
    throw "Did not get expected EIO when resizing file";
  }

  if (Module._try_unlink() != 0) {
    throw "Did not get expected EIO when unlinking file";
  }

  await access.close();

  // We can open the file in any mode now that there is no open access
  // handle for it.

  if (Module._try_open_wronly() != 1) {
    throw "Unexpected failure opening file for writing";
  }

  if (Module._try_open_rdwr() != 1) {
    throw "Unexpected failure opening file for reading and writing";
  }

  if (Module._try_open_rdonly() != 1) {
    throw "Unexpected failure opening file for reading";
  }

  if (Module._try_truncate() != 1) {
    throw "Unexpected failure when resizing file";
  }

  if (Module._try_oob_read() != 0) {
    throw "Did not get expected EINVAL doing out of bounds read";
  }

  if (Module._try_oob_write() != 0) {
    throw "Did not get expected EINVAL doing out of bounds write";
  }

  if (Module._try_unlink() != 1) {
    throw "Did not succeed to unlink the file (which should work now that " +
          "nothing prevents it)";
  }

  if (Module._try_rename_dir() != 0) {
    throw "Did not get expected EBUSY while renaming directory";
  }

  Module._report_result(0);
}
