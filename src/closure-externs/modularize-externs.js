// In MODULARIZE mode the JS code may be executed later, after `document.currentScript` is gone, so we store
// it to `_scriptName` outside the wrapper function. Therefore, it cannot be minified.
// In EXPORT_ES6 mode we use `import.meta.url` and for Node.js CommonJS builds we use `__filename`.

/**
 * @suppress {duplicate, undefinedVars}
 */
var _scriptName;
