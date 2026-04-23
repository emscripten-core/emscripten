let { promise, resolve } = Promise.withResolvers();
promise.then(() => console.log('exiting'));

Module['onRuntimeInitialized'] = function() {
  console.log('initialized');
  _create_thread_async();
};
