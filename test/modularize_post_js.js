#preprocess

// This file gets included via `--extern-post-js` in order to run tests
// that are built with `-sMODULARIZE`.

#if PTHREADS
// Avoid instantiating the module on pthreads.
if (!isPthread)
#endif
{{{ EXPORT_NAME }}}();
