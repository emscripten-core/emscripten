// proxy to/from worker

var ABORT = false;

var newAudioProxy = function (worker) {

  // SDL_OpenAudio is a synchronous function and needs information about audio
  // features. Since it can't interrogate the Browser thread synchronously,
  // this function detects these features and sends the info to the worker
  // process ahead of time so SDL_OpenAudio can then use when invoked.
  function detectAndSendWebAudioFeaturesToWorker() {
    worker.postMessage({
      target: 'audio',
      features: {
        'Audio.mozSetup': typeof(Audio) === 'function' && typeof(new Audio()['mozSetup']) === 'function',
        AudioContext: typeof(AudioContext) !== 'undefined',
        webkitAudioContext: typeof(webkitAudioContext) !== 'undefined'
      }
    });
  }

  var sdlAudioCore = {
    {{{ library_sdl_audio.js }}}
  };

  var SDL = {};

  var Browser = {
    safeSetTimeout:function (func, timeout) {
      return setTimeout(function() {
        if (!ABORT) func();
      }, timeout);
    }
  };

  function SDL_OpenAudio(sdl) {
    for (var attrname in sdl) {
      SDL[attrname] = sdl[attrname];
    }

    function copyMozillaAudioData(arrayBuffer, mozBuffer, sdlAudioFormat, totalSamples) {
      // The input audio data for SDL audio is either 8-bit or 16-bit interleaved across channels, output for Mozilla Audio Data API
      // needs to be Float32 interleaved, so perform a sample conversion.
      if (sdlAudioFormat == 0x8010 /*AUDIO_S16LSB*/) {
        var int16View = new Int16Array(arrayBuffer);
        for (var i = 0; i < totalSamples; i++) {
          mozBuffer[i] = int16View[i] / 0x8000;
        }
      } else if (sdlAudioFormat == 0x0008 /*AUDIO_U8*/) {
        var int8View = new Int8Array(arrayBuffer);
        for (var i = 0; i < totalSamples; i++) {
          var v = int8View[i];
          mozBuffer[i] = ((v >= 0) ? v-128 : v+128) /128;
        }
      }
    }

    function copyAudioData(arrayBuffer, channelData, sizeSamplesPerChannel, sdlAudioFormat, numChannels, channel) {
      if (sdlAudioFormat == 0x8010 /*AUDIO_S16LSB*/) {
        var int16View = new Int16Array(arrayBuffer);
        for(var j = 0; j < sizeSamplesPerChannel; ++j) {
          channelData[j] = int16View[(j*numChannels + channel)] / 0x8000;
        }
      } else if (sdlAudioFormat == 0x0008 /*AUDIO_U8*/) {
        var int8View = new Int8Array(arrayBuffer);
        for(var j = 0; j < sizeSamplesPerChannel; ++j) {
          var v = int8View[j*numChannels + channel];
          channelData[j] = ((v >= 0) ? v-128 : v+128) /128;
        }
      }
    }

    SDL.audio.caller = function SDL_audio_caller() {
      if (!SDL.audio) {
        return;
      }
      worker.postMessage({ target: 'audio', op: 'pushAudio' });
    };

    try {
      sdlAudioCore.SDL_OpenAudio_core(SDL, Browser, copyMozillaAudioData, copyAudioData);
      sdlAudioCore.allocateChannels_core(SDL, 32);
    } catch(e) {
      console.log('Initializing SDL audio threw an exception: "' + e.toString() + '"! Continuing without audio.');
      SDL.audio = null;
      sdlAudioCore.allocateChannels_core(SDL, 0);
    }
  }

  var scriptProcessorNode = {
    sizeSamplesPerChannel: null,
    outputBuffer: null,
    onaudioprocessFn: function (SDL, sizeSamplesPerChannel) {
      scriptProcessorNode.sizeSamplesPerChannel = sizeSamplesPerChannel;
      return function (e) {
        scriptProcessorNode.outputBuffer = e['outputBuffer'];
        worker.postMessage({target: 'audio', op: 'pushAudioForScriptProcessorNode'});
      };
    },
    pushAudio: function (audioBuffer) {
      SDL.fillWebAudioBufferFromHeap(audioBuffer, scriptProcessorNode.sizeSamplesPerChannel, scriptProcessorNode.outputBuffer);  
    }
  };

  return {
    handleMessage: function (message) {
      if (message.SDL_OpenAudio) {
        SDL_OpenAudio(message.SDL_OpenAudio);
      } else if (message.pushAudio) {
        SDL.audio.pushAudio(message.pushAudio, message.pushAudio.byteLength);
      } else if (message.pushAudioForScriptProcessorNode) {
        scriptProcessorNode.pushAudio(message.pushAudioForScriptProcessorNode);
      } else if (message.SDL_PauseAudio) {
        sdlAudioCore.SDL_PauseAudio_core(SDL, Browser, message.SDL_PauseAudio[0], scriptProcessorNode.onaudioprocessFn);
      } else if (message.SDL_CloseAudio) {
        sdlAudioCore.SDL_CloseAudio_core(SDL, Browser, sdlAudioCore.SDL_PauseAudio_core, sdlAudioCore.allocateChannels_core);
        SDL.audio = null;
      }
    },
    detectAndSendWebAudioFeaturesToWorker: detectAndSendWebAudioFeaturesToWorker
  };

};  // newAudioProxy

Module.ctx = Module.canvas.getContext('2d');

// render

var renderFrameData = null;

function renderFrame() {
  var dst = Module.canvasData.data;
  if (dst.set) {
    dst.set(renderFrameData);
  } else {
    for (var i = 0; i < renderFrameData.length; i++) {
      dst[i] = renderFrameData[i];
    }
  }
  Module.ctx.putImageData(Module.canvasData, 0, 0);
  renderFrameData = null;
}

window.requestAnimationFrame = window.requestAnimationFrame || window.mozRequestAnimationFrame ||
                               window.webkitRequestAnimationFrame || window.msRequestAnimationFrame ||
                               renderFrame;

// end render

var worker = new Worker('{{{ filename }}}.js');
var audioProxy = newAudioProxy(worker);
var workerResponded = false;

worker.onmessage = function worker_onmessage(event) {
  if (!workerResponded) {
    workerResponded = true;
    if (Module.setStatus) Module.setStatus('');
  }

  var data = event.data;
  switch (data.target) {
    case 'set': {
      this[data.property] = data.value;
      break;
    }
    case 'stdout': {
      Module.print(data.content);
      break;
    }
    case 'stderr': {
      Module.printErr(data.content);
      break;
    }
    case 'window': {
      window[data.method]();
      break;
    }
    case 'canvas': {
      switch (data.op) {
        case 'resize': {
          Module.canvas.width = data.width;
          Module.canvas.height = data.height;
          Module.canvasData = Module.ctx.getImageData(0, 0, data.width, data.height);
          worker.postMessage({ target: 'canvas', boundingClientRect: cloneObject(Module.canvas.getBoundingClientRect()) });
          break;
        }
        case 'render': {
          if (renderFrameData) {
            // previous image was not rendered yet, just update image
            renderFrameData = data.image.data;
          } else {
            // previous image was rendered so update image and request another frame
            renderFrameData = data.image.data;
            window.requestAnimationFrame(renderFrame);
          }
          break;
        }
        default: throw 'eh?';
      }
      break;
    }
    case 'audio': {
      audioProxy.handleMessage(data);
      break;
    }
    case 'status': {
      switch (data.status) {
        case 'ready': {
          audioProxy.detectAndSendWebAudioFeaturesToWorker();
          break;
        }
      }
      break;
    }
    default: throw 'what? ' + data.target;
  }
};

function cloneObject(event) {
  var ret = {};
  for (var x in event) {
    if (x == x.toUpperCase()) continue;
    var prop = event[x];
    if (typeof prop === 'number' || typeof prop === 'string') ret[x] = prop;
  }
  return ret;
};

['keydown', 'keyup', 'keypress', 'blur', 'visibilitychange'].forEach(function(event) {
  document.addEventListener(event, function(event) {
    worker.postMessage({ target: 'document', event: cloneObject(event) });
    event.preventDefault();
  });
});

['unload'].forEach(function(event) {
  window.addEventListener(event, function(event) {
    worker.postMessage({ target: 'window', event: cloneObject(event) });
  });
});

['mousedown', 'mouseup', 'mousemove', 'DOMMouseScroll', 'mousewheel', 'mouseout'].forEach(function(event) {
  Module.canvas.addEventListener(event, function(event) {
    worker.postMessage({ target: 'canvas', event: cloneObject(event) });
    event.preventDefault();
  }, true);
});
