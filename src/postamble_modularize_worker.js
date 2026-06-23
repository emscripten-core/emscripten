#if PTHREADS

// Create code for detecting if we are running in a pthread.
// Normally this detection is done when the module is itself run but
// when running in MODULARIZE mode we need use this to know if we should
// run the module constructor on startup (true only for pthreads).
#if ENVIRONMENT_MAY_BE_WEB || ENVIRONMENT_MAY_BE_WORKER
var isPthread = {{{ pthreadDetection() }}};
#if ENVIRONMENT_MAY_BE_NODE
// In order to support both web and node we also need to detect node here.
var isNode = {{{ nodeDetectionCode() }}};
if (isNode) isPthread = {{{ nodePthreadDetection() }}}
#endif
#else ENVIRONMENT_MAY_BE_NODE
var isPthread = {{{ nodePthreadDetection() }}}
// When running as a pthread, construct a new instance on startup
#endif

#if MODULARIZE == 'instance'
isPthread && init();
#else
isPthread && {{{ EXPORT_NAME }}}();
#endif

#endif // PTHREADS

#if WASM_WORKERS

// Same as above for WASM_WORKERS
// Normally this detection is done when the module is itself run but
// when running in MODULARIZE mode we need use this to know if we should
// run the module constructor on startup (true only for pthreads).
#if ENVIRONMENT_MAY_BE_WEB || ENVIRONMENT_MAY_BE_WORKER
var isWW = {{{ wasmWorkerDetection() }}};
// In order to support both web and node we also need to detect node here.
#if ENVIRONMENT_MAY_BE_NODE
#if !PTHREADS
var isNode = {{{ nodeDetectionCode() }}};
#endif
if (isNode) isWW = {{{ nodeWWDetection() }}};
#endif
#elif ENVIRONMENT_MAY_BE_NODE
var isWW = {{{ nodeWWDetection() }}};
#endif

#if AUDIO_WORKLET
isWW ||= !!globalThis.AudioWorkletGlobalScope;
// When running as a wasm worker, construct a new instance on startup
#endif

#if MODULARIZE == 'instance'
isWW && init();
#else
isWW && {{{ EXPORT_NAME }}}();
#endif

#endif // WASM_WORKERS
