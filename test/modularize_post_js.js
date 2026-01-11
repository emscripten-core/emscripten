#preprocess

// This file gets included via `--extern-post-js` in order to run tests
// that are built with `-sMODULARIZE`.

#if PTHREADS || WASM_WORKERS
#if ENVIRONMENT_MAY_BE_WEB || ENVIRONMENT_MAY_BE_WORKER
let isMainThread = !globalThis.self?.name?.startsWith('em-');
#endif
#if ENVIRONMENT_MAY_BE_NODE
if ({{{ nodeDetectionCode() }}})
#if EXPORT_ES6
  ({ isMainThread } = await import('worker_threads'));
#else
  ({ isMainThread } = require('worker_threads'));
#endif
#endif // ENVIRONMENT_MAY_BE_NODE
// Avoid instantiating the module on pthreads.
if (isMainThread)
#endif
{{{ EXPORT_NAME }}}();
