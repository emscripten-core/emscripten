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

  Module._report_result(0);
}
