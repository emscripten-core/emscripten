const statfsSync = require('fs').statfsSync;

Module['preRun'] = () => {
  const statfs = statfsSync('/');
  ENV.EXPECTED_BLOCKS = statfs.blocks.toString();
};

Module['onRuntimeInitialized'] = () => {
  out('mounting node filesystem under /nodefs');
  FS.mkdir('/nodefs');
  FS.mount(NODEFS, { root: '.' }, '/nodefs');
  FS.chdir('/nodefs');
};
