/**
 * If DYNCALLS is enabled, then we will emit dynCall_* calls. If we happen to
 * emit JS with such a call, but that code path is never reached, and the wasm
 * does not contain any method with that signature, then we would not emit a
 * dynCall_* method and closure would error. Define the common signatures here
 * to avoid that.
 *
 * TODO: Fix https://github.com/emscripten-core/emscripten/issues/13858 in
 *       another way, either by landing
 *       https://github.com/emscripten-core/emscripten/pull/12088 or otherwise
 *       removing DYNCALLS.
 */
/**
 * @suppress {duplicate, undefinedVars}
 */
var dynCall_v;
/**
 * @suppress {duplicate, undefinedVars}
 */
var dynCall_vi;
/**
 * @suppress {duplicate, undefinedVars}
 */
var dynCall_vii;
/**
 * @suppress {duplicate, undefinedVars}
 */
var dynCall_iii;


