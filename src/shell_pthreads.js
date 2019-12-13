if (ENVIRONMENT_IS_WORKER) {
  // Grab imports from the pthread to local scope.
  buffer = {{{EXPORT_NAME}}}['buffer'];
  tempDoublePtr = {{{EXPORT_NAME}}}['tempDoublePtr'];
  DYNAMIC_BASE = {{{EXPORT_NAME}}}['DYNAMIC_BASE'];
  DYNAMICTOP_PTR = {{{EXPORT_NAME}}}['DYNAMICTOP_PTR'];
  // Note that not all runtime fields are imported above. Values for STACK_BASE, STACKTOP and STACK_MAX are not yet known at worker.js load time.
  // These will be filled in at pthread startup time (the 'run' message for a pthread - pthread start establishes the stack frame)
}

// Internal: Because fastcomp emits a "if (!ENVIRONMENT_IS_PTHREAD)" check, still provide the variable ENVIRONMENT_IS_PTHREAD here. However
// use ENVIRONMENT_IS_WORKER instead of ENVIRONMENT_IS_PTHREAD everywhere.
var ENVIRONMENT_IS_PTHREAD = ENVIRONMENT_IS_WORKER;
