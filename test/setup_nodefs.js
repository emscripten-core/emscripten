const statfsSync = require('fs').statfsSync;

const DEFAULT_BLOCKS = 1e6;

Module['preRun'] = () => {
  try {
    const statfs = statfsSync('/');
    ENV.EXPECTED_BLOCKS = statfs.blocks.toString();
  } catch (e) {
    // Older versions of Node don't support statfsSync
  }
};

Module['onRuntimeInitialized'] = () => {
  out('mounting node filesystem under /nodefs');
  FS.mkdir('/nodefs');
  FS.mount(NODEFS, { root: '.' }, '/nodefs');
  FS.chdir('/nodefs');
};
