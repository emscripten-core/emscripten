/**
 * This file provides some setup for the emscripten runtime. In particular it prevents the runtime exiting.
 * This is necessary as otherwise things like printf don't seem to work from methods called by JavaScript.
 */
var Module = {
    'noExitRuntime' : true
};


