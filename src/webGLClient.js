// WebGLWorker client code

function assert(x) {
  if (!x) throw 'failed assert';
}

function WebGLClient() {
  var objects = {};

  var calls = {
    0: { name: 'NULL', args: 0 },
    1: { name: 'getExtension', args: 1 },
    2: { name: 'enable', args: 1 },
    3: { name: 'disable', args: 1 },
    4: { name: 'clear', args: 1 },
    5: { name: 'clearColor', args: 4 },
    6: { name: 'createShader', args: -2 },
    7: { name: 'deleteShader', args: 1 },
    8: { name: 'shaderSource', args: 2 },
    9: { name: 'compileShader', args: 1 },
    10: { name: 'createProgram', args: -1 },
    11: { name: 'deleteProgram', args: 1 },
    12: { name: 'attachShader', args: 2 },
    13: { name: 'bindAttribLocation', args: 3 },
    14: { name: 'linkProgram', args: 1 },
    15: { name: 'getProgramParameter', args: 2 },
    16: { name: 'getUniformLocation', args: -3 },
    17: { name: 'useProgram', args: 1 },
    18: { name: 'uniform1i', args: 2 },
    19: { name: 'uniform1f', args: 2 },
    20: { name: 'uniform3fv', args: 2 },
    21: { name: 'uniform4fv', args: 2 },
    22: { name: 'uniformMatrix4fv', args: 3 },
    23: { name: 'vertexAttrib4fv', args: 2 },
    24: { name: 'createBuffer', args: -1 },
    25: { name: 'deleteBuffer', args: 1 },
    26: { name: 'bindBuffer', args: 2 },
    27: { name: 'bufferData', args: 3 },
    28: { name: 'bufferSubData', args: 3 },
    29: { name: 'viewport', args: 4 },
    30: { name: 'vertexAttribPointer', args: 6 },
    31: { name: 'enableVertexAttribArray', args: 1 },
    32: { name: 'disableVertexAttribArray', args: 1 },
    33: { name: 'drawArrays', args: 3 },
    34: { name: 'drawElements', args: 4 },
    35: { name: 'getError', args: 0 },
    36: { name: 'createTexture', args: -1 },
    37: { name: 'deleteTexture', args: 1 },
    38: { name: 'bindTexture', args: 2 },
    39: { name: 'texParameteri', args: 3 },
    40: { name: 'texImage2D', args: 9 },
    41: { name: 'compressedTexImage2D', args: 7 },
    42: { name: 'activeTexture', args: 1 },
    43: { name: 'getShaderParameter', args: 2 },
    44: { name: 'clearDepth', args: 1 },
    45: { name: 'depthFunc', args: 1 },
    46: { name: 'frontFace', args: 1 },
    47: { name: 'cullFace', args: 1 },
    48: { name: 'pixelStorei', args: 2 },
    49: { name: 'depthMask', args: 1 },
    50: { name: 'depthRange', args: 2 },
    51: { name: 'blendFunc', args: 2 },
    52: { name: 'scissor', args: 4 },
    53: { name: 'colorMask', args: 4 },
    54: { name: 'lineWidth', args: 1 },
    55: { name: 'createFramebuffer', args: -1 },
    56: { name: 'deleteFramebuffer', args: 1 },
    57: { name: 'bindFramebuffer', args: 2 },
    58: { name: 'framebufferTexture2D', args: 5 },
    59: { name: 'createRenderbuffer', args: -1 },
    60: { name: 'deleteRenderbuffer', args: 1 },
    61: { name: 'bindRenderbuffer', args: 2 },
    62: { name: 'renderbufferStorage', args: 4 },
    63: { name: 'framebufferRenderbuffer', args: 4 },
    64: { name: 'debugPrint', args: 1 },
  };

  function fixArgs(command, args) {
    switch (command) {
      case 'deleteFramebuffer':
      case 'deleteRenderbuffer':
      case 'deleteBuffer':
      case 'deleteShader':
      case 'deleteProgram':
      case 'deleteTexture': {
        var id = args[0];
        args[0] = objects[id];
        objects[id] = null; // stop holding on to the object globally
        break;
      }
      case 'getProgramParameter':
      case 'getShaderParameter':
      case 'uniform1i':
      case 'uniform1f':
      case 'uniform3fv':
      case 'uniform4fv':
      case 'uniformMatrix4fv':
      case 'getUniformLocation':
      case 'useProgram':
      case 'linkProgram':
      case 'bindAttribLocation':
      case 'compileShader':
      case 'shaderSource': args[0] = objects[args[0]]; break;
      case 'attachShader': args[0] = objects[args[0]]; args[1] = objects[args[1]]; break;
      case 'bindRenderbuffer':
      case 'bindFramebuffer':
      case 'bindTexture':
      case 'bindBuffer': args[1] = args[1] ? objects[args[1]] : null; break;
      case 'framebufferRenderbuffer':
      case 'framebufferTexture2D': args[3] = args[3] ? objects[args[3]] : null; break;
    }
    return args;
  }

  function renderCommands(buffer) {
    var ctx = Module.ctx;
    var i = 0;
    var len = buffer.length;
    //dump('issuing commands, buffer len: ' + len + '\n');
    while (i < len) {
      var info = calls[buffer[i++]];
      var command = info.name;
      assert(typeof command === 'string')
      var numArgs = info.args;
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
        //} else if (command === 'debugPrint') {
        //  dump(args[0] + '\n');
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
      //var err;
      //while ((err = ctx.getError()) !== ctx.NO_ERROR) {
      //  dump('warning: GL error ' + err + ', after ' + [command, numArgs] + '\n');
      //}
      assert(i <= len);
    }
  }

  var commandBuffers = [];

  function renderAllCommands() {
    // TODO: we can avoid running commands from buffers that are not the last, if they
    //       have no side effects, as each buffer is from a different frame
    //if (commandBuffers.length > 1) dump('extra buffs: ' + (commandBuffers.length-1) + '\n');
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
  ['MAX_VERTEX_ATTRIBS', 'MAX_TEXTURE_IMAGE_UNITS', 'MAX_TEXTURE_SIZE', 'MAX_CUBE_MAP_TEXTURE_SIZE', 'MAX_VERTEX_UNIFORM_VECTORS', 'MAX_FRAGMENT_UNIFORM_VECTORS', 'MAX_VARYING_VECTORS', 'MAX_COMBINED_TEXTURE_IMAGE_UNITS', 'VENDOR', 'RENDERER', 'VERSION'].forEach(function(name) {
    parameters[ctx[name]] = ctx.getParameter(ctx[name]);
  });
  worker.postMessage({ target: 'gl', op: 'setPrefetched', parameters: parameters, extensions: ctx.getSupportedExtensions() });
};

