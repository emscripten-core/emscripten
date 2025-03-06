#preprocess

// This file gets included via `--extern-post-js` in order to run tests
// that are built with `-sMODULARIZE`.

#if PTHREADS
// Avoid instantiating the module on pthreads.
#if EXPORT_ES6
const isMainThread = (await import('worker_threads')).isMainThread;
#else
const { isMainThread } = require('worker_threads');
#endif
if (isMainThread)
#endif
{{{ EXPORT_NAME }}}();
