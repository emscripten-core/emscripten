// WebGLWorker client code

function WebGLClient() {
  var data = {};
  ['MAX_VERTEX_ATTRIBS'].forEach(function(name) {
    data[name] = Module.ctx.getParameter(Module.ctx[name]);
  });
  worker.postMessage({ target: 'gl', op: 'setPrefetchedParameters', data: data });
}

