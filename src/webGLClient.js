// WebGLWorker client code

function assert(x) {
  if (!x) throw 'failed assert';
}

function WebGLClient() {
  var objects = {};

  function fixArgs(command, args) {
    switch (command) {
      case 'deleteShader':
      case 'deleteProgram':
      case 'getProgramParameter':
      case 'getShaderParameter':
      case 'uniform1i':
      case 'uniform1f':
      case 'uniform4fv':
      case 'uniformMatrix4fv':
      case 'getUniformLocation':
      case 'useProgram':
      case 'linkProgram':
      case 'bindAttribLocation':
      case 'compileShader':
      case 'shaderSource': args[0] = objects[args[0]]; break;
      case 'attachShader': args[0] = objects[args[0]]; args[1] = objects[args[1]]; break;
      case 'bindTexture':
      case 'bindBuffer': args[1] = args[1] ? objects[args[1]] : null; break;
    }
    return args;
  }

  function renderCommands(buffer) {
    var ctx = Module.ctx;
    var i = 0;
    var len = buffer.length;
    //dump('issuing commands, buffer len: ' + len + '\n');
    while (i < len) {
      var command = buffer[i++];
      assert(typeof command === 'string')
      var numArgs = buffer[i++];
      assert(typeof numArgs === 'number', command);
      //dump('issue ' + [command, numArgs, 'peek:' + buffer.slice(i, i+5)] + '\n');
      if (numArgs === 0) {
        //dump('issue: ' + command + '\n');
        if (command === 'getError') {
          assert(ctx.getError() === ctx.NO_ERROR, 'we cannot handle errors, we are async proxied WebGL');
        } else {
          ctx[command]();
        }
      } else if (numArgs > 0) {
        var args = fixArgs(command, buffer.slice(i, i+numArgs));
        i += numArgs;
        //dump('issue+: ' + command + '(' + args + '), ' + numArgs + '\n');
        if (command === 'getShaderParameter' || command === 'getProgramParameter') {
          assert(ctx[command](args[0], args[1]), 'we cannot handle errors, we are async proxied WebGL');
        } else {
          ctx[command].apply(ctx, args);
        }
      } else {
        // negative means a constructor, last argument is the id to save as
        numArgs = -numArgs - 1;
        var args = fixArgs(command, buffer.slice(i, i+numArgs));
        i += numArgs;
        var id = buffer[i++];
        //dump('issue-: ' + command + '(' + args + '), ' + numArgs + '\n');
        objects[id] = ctx[command].apply(ctx, args);
      }
      assert(i <= len);
    }
  }

  var commandBuffers = [];

  function renderAllCommands() {
    // TODO: we can avoid running commands from buffers that are not the last, if they
    //       have no side effects, as each buffer is from a different frame
    for (var i = 0; i < commandBuffers.length; i++) {
      renderCommands(commandBuffers[i]);
    }
    commandBuffers.length = 0;
  }

  this.onmessage = function(msg) {
    //dump('client GL got ' + JSON.stringify(msg) + '\n');
    switch(msg.op) {
      case 'render': {
        if (commandBuffers.length === 0) {
          // requestion a new frame, we will clear the buffers after rendering them
          window.requestAnimationFrame(renderAllCommands);
        }
        commandBuffers.push(msg.commandBuffer);
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
  ['MAX_VERTEX_ATTRIBS', 'MAX_TEXTURE_IMAGE_UNITS', 'MAX_TEXTURE_SIZE', 'MAX_CUBE_MAP_TEXTURE_SIZE', 'VENDOR', 'RENDERER', 'VERSION'].forEach(function(name) {
    parameters[ctx[name]] = ctx.getParameter(ctx[name]);
  });
  worker.postMessage({ target: 'gl', op: 'setPrefetched', parameters: parameters, extensions: ctx.getSupportedExtensions() });
};

