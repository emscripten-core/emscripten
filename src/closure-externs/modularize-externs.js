/**
 * In MODULARIZE mode the JS code may be executed later, after `document.currentScript` is gone, so
 * we store it to `_scriptName` outside the wrapper function. Therefore, it cannot be minified.
 * In EXPORT_ES6 mode we use `import.meta.url` and for Node.js CommonJS builds we use `__filename`.
 * @suppress {duplicate, undefinedVars}
 */
var _scriptName;

/**
 * Used in MODULARIZE mode as the name of the incoming module argument.
 * This is generated outside of the code we pass to closure so from closure's
 * POV this is "extern".
 */
var moduleArg;

/**
 * @suppress {duplicate, undefinedVars}
 */
var Module;
