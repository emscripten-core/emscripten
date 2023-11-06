// Emscripten has a -sEXPORT_ES6=1 option to export the generated program as a 'Module'
// object. However, that module is asynchronously loaded, which does not work well
// for Audio Worklets, which require synchronous loading. So instead we manually
// build the page without EXPORT_ES6, and just add an export here.

export default Module;
