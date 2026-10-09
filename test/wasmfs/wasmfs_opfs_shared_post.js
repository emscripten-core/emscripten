// Check that WasmFS can open an OPFS file while another context holds a
// shared ('readwrite-unsafe') access handle for it.
async function run_test() {
  let access;
  let modeRead = false;
  try {
    let root = await navigator.storage.getDirectory();
    let child = await root.getFileHandle("data", {create: true});
    access = await child.createSyncAccessHandle({
      get mode() {
        modeRead = true;
        return 'readwrite-unsafe';
      }
    });
  } catch (e) {
    console.log("test setup failed");
    throw e;
  }

  if (!modeRead) {
    console.log("skipping: createSyncAccessHandle does not support `mode`");
    access.close();
    // Remove the file so that later tests in the same origin start clean.
    Module._try_unlink();
    Module._report_result(0);
    return;
  }

  if (Module._try_open_wronly() != 1) {
    throw "Unexpected failure opening file for writing";
  }

  if (Module._try_open_rdwr() != 1) {
    throw "Unexpected failure opening file for reading and writing";
  }

  if (Module._try_open_rdonly() != 1) {
    throw "Unexpected failure opening file for reading";
  }

  access.close();

  // Remove the file so that later tests in the same origin start clean.
  if (Module._try_unlink() != 1) {
    throw "Did not succeed to unlink the file";
  }

  Module._report_result(0);
}
