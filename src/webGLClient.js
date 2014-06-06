// WebGLWorker client code

function WebGLClient() {
  function renderCommands(buffer) {
  }

  this.onmessage = function(msg) {
    dump('client GL got ' + JSON.stringify(msg) + '\n');
    switch(msg.op) {
      case 'render': {
        renderCommands(msg.commandBuffer);
        break;
      }
      default: throw 'weird gl onmessage ' + JSON.stringify(msg);
    }
  };
}

WebGLClient.prefetch = function() {
  var canvas = document.createElement('canvas');
  var ctx = canvas.getContext('webgl-experimental') || canvas.getContext('webgl');
  if (!ctx) return;
  var parameters = {};
  ['MAX_VERTEX_ATTRIBS'].forEach(function(name) {
    parameters[ctx[name]] = ctx.getParameter(ctx[name]);
  });
  worker.postMessage({ target: 'gl', op: 'setPrefetched', parameters: parameters, extensions: ctx.getSupportedExtensions() });
};

