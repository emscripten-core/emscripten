var statusElement = document.getElementById('status');
var progressElement = document.getElementById('progress');
var spinnerElement = document.getElementById('spinner');

var Module = {
  print: (() => {
    var element = document.getElementById('output');
    if (element) element.value = ''; // clear browser cache
    return (...args) => {
      var text = args.join(' ');
      console.log(text);
      if (element) {
        element.value += text + "\n";
        element.scrollTop = element.scrollHeight; // focus on bottom
      }
    };
  })(),
  printErr: (...args) => {
    var text = args.join(' ');
    console.error(text);
  },
  canvas: (() => {
    var canvas = document.getElementById('canvas');
    return canvas;
  })(),
  setStatus: (text) => {
    if (!Module.setStatus.last) Module.setStatus.last = { time: Date.now(), text: '' };
    if (text === Module.setStatus.text) return;
    var m = text.match(/([^(]+)\((\d+(\.\d+)?)\/(\d+)\)/);
    var now = Date.now();
    if (m && now - Date.now() < 30) return;
    if (m) {
      text = m[1];
      progressElement.value = parseInt(m[2])*100;
      progressElement.max = parseInt(m[4])*100;
      progressElement.hidden = false;
      spinnerElement.hidden = false;
    } else {
      progressElement.value = null;
      progressElement.max = null;
      progressElement.hidden = true;
      if (!text) spinnerElement.hidden = true;
    }
    statusElement.innerHTML = text;
  },
  totalDependencies: 0,
  monitorRunDependencies: (left) => {
    this.totalDependencies = Math.max(this.totalDependencies, left);
    Module.setStatus(left ? 'Preparing... (' + (this.totalDependencies-left) + '/' + this.totalDependencies + ')' : 'All downloads complete.');
  }
};
Module.setStatus('Downloading...');
window.onerror = () => {
  Module.setStatus('Exception thrown, see JavaScript console');
  spinnerElement.style.display = 'none';
  Module.setStatus = (text) => {
    if (text) Module.printErr('[post-exception status] ' + text);
  };
};

function downloadWasm(url) {
  console.log('fetching wasm: ', url);
  return fetch(url).then((response) => response.arrayBuffer());
}

var wasm = downloadWasm('manual_wasm_instantiate.wasm');

Module.instantiateWasm = (imports, successCallback) => {
  console.log('instantiateWasm: instantiating asynchronously');
  wasm.then((bytes) => {
    console.log('wasm download finished, begin instantiating');
    var wasmInstantiate = WebAssembly.instantiate(bytes, imports).then((output) => {
      if (typeof WasmOffsetConverter != "undefined") {
        wasmOffsetConverter = new WasmOffsetConverter(bytes, output.module);
      }
      console.log('wasm instantiation succeeded');
      Module.testWasmInstantiationSucceeded = 1;
      successCallback(output.instance);
    }).catch((e) => {
      console.log('wasm instantiation failed! ' + e);
    });
  });
  return {};
}

var script = document.createElement('script');
script.src = "manual_wasm_instantiate.js";
document.body.appendChild(script);
