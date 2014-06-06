// WebGLWorker client code

function WebGLClient() {
}

WebGLClient.prefetch = function() {
  var canvas = document.createElement('canvas');
  var ctx = canvas.getContext('webgl-experimental') || canvas.getContext('webgl');
  if (!ctx) return;
  var data = {};
  ['MAX_VERTEX_ATTRIBS'].forEach(function(name) {
    data[ctx[name]] = ctx.getParameter(ctx[name]);
  });
  worker.postMessage({ target: 'gl', op: 'setPrefetchedParameters', data: data });
};

