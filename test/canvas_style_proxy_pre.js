var Module = Module || {};
Module.postRun = Module.postRun || [];
Module.postRun.push(function () {
  postMessage({ target: 'window', method: 'verifyCanvasStyle' });
})
