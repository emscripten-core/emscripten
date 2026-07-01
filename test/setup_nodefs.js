Module['onRuntimeInitialized'] = () => {
  out('mounting node filesystem under /nodefs');
  FS.mkdir('/nodefs');
  FS.mount(NODEFS, { root: '.' }, '/nodefs');
  FS.chdir('/nodefs');
};
