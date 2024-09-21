Module['preRun'] = () => {
  console.log('preRun');
  // it is ok to call cwrap before the runtime is loaded. we don't need the code
  // and everything to be ready, since cwrap just prepares to call code, it 
  // doesn't actually call it
  var wrappedAdd = Module['cwrap']('add', 'number', ['number', 'number']);
  // but to call the compiled code, we must wait for the runtime
  Module['onRuntimeInitialized'] = async () => {
    console.log('onRuntimeInitialized');
    if (wrappedAdd(5, 6) != 11) throw '5 + 6 should be 11';
    // report success
    await fetch('http://localhost:8888/report_result?0');
    window.close();
  };
};
