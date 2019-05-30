// Three configurations we can be running in:
// 1) We could be the application main() thread running in the main JS UI thread. (ENVIRONMENT_IS_WORKER == false and ENVIRONMENT_IS_PTHREAD == false)
// 2) We could be the application main() thread proxied to worker. (with Emscripten -s PROXY_TO_WORKER=1) (ENVIRONMENT_IS_WORKER == true, ENVIRONMENT_IS_PTHREAD == false)
// 3) We could be an application pthread running in a worker. (ENVIRONMENT_IS_WORKER == true and ENVIRONMENT_IS_PTHREAD == true)
#if USE_PTHREADS

// ENVIRONMENT_IS_PTHREAD=true will have been preset in worker.js. Make it false in the main runtime thread.
var ENVIRONMENT_IS_PTHREAD = Module.ENVIRONMENT_IS_PTHREAD || false;
if (!ENVIRONMENT_IS_PTHREAD) {
  var PthreadWorkerInit = {}; // Collects together variables that are needed at initialization time for the web workers that host pthreads.
}
#if MODULARIZE
else {
  // Grab imports from the pthread to local scope.
  var buffer = {{{EXPORT_NAME}}}.buffer;
  var tempDoublePtr = {{{EXPORT_NAME}}}.tempDoublePtr;
  var STATICTOP = {{{EXPORT_NAME}}}.STATICTOP;
  var DYNAMIC_BASE = {{{EXPORT_NAME}}}.DYNAMIC_BASE;
  var DYNAMICTOP_PTR = {{{EXPORT_NAME}}}.DYNAMICTOP_PTR;
  var PthreadWorkerInit = {{{EXPORT_NAME}}}.PthreadWorkerInit;
  // Note that not all runtime fields are imported above. Values for STACK_BASE, STACKTOP and STACK_MAX are not yet known at worker.js load time.
  // These will be filled in at pthread startup time (the 'run' message for a pthread - pthread start establishes the stack frame)
}
#endif

#endif
