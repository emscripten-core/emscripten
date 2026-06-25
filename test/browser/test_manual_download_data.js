var statusElement = document.getElementById('status');
var progressElement = document.getElementById('progress');
var spinnerElement = document.getElementById('spinner');

var Module = {
  print: (() => {
    var element = document.getElementById('output');
    if (element) element.value = ''; // clear browser cache
    return function(text) {
      if (arguments.length > 1) text = Array.prototype.slice.call(arguments).join(' ');
      console.log(text);
      if (element) {
        element.value += text + "\n";
        element.scrollTop = element.scrollHeight; // focus on bottom
      }
    };
  })(),
  printErr: (text) => {
    if (arguments.length > 1) text = Array.prototype.slice.call(arguments).join(' ');
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
  },
  getPreloadedPackage: (remotePackageName, remotePackageSize) => {
    console.log(`Runtime asking for remote package ${remotePackageName}, expected size ${remotePackageSize} bytes.`);
    return Module['downloadedData'];
  },
};
Module.setStatus('Downloading...');
window.onerror = function() {
  Module.setStatus('Exception thrown, see JavaScript console');
  spinnerElement.style.display = 'none';
  Module.setStatus = function(text) {
    if (text) Module.printErr('[post-exception status] ' + text);
  };
};

function download(url) {
  return fetch(url).then((rsp) => rsp.arrayBuffer());
}

function addScriptToDom(scriptCode) {
  return new Promise((resolve, reject) => {
    var script = document.createElement('script');
    var blob = new Blob([scriptCode], { type: 'application/javascript' });
    var objectUrl = URL.createObjectURL(blob);
    script.src = objectUrl;
    script.onload = function() {
      console.log('added js script to dom');
      script.onload = script.onerror = null;
      URL.revokeObjectURL(objectUrl);
      resolve();
    }
    script.onerror = function(e) {
      script.onload = script.onerror = null;
      URL.revokeObjectURL(objectUrl);
      console.error('script failed to add to dom: ' + e);
      reject(e.message || "(out of memory?)");
    }
    document.body.appendChild(script);
  });
}

var dataDownload = download('/test/test_manual_download_data.data').then((data) => {
  console.log('downloaded data file');
  Module['downloadedData'] = data;
  var jsDownload = download('/test/test_manual_download_data.js').then((data) => {
    console.log('downloaded js file');
    Module['mainScriptUrlOrBlob'] = new Blob([data], { type: 'application/javascript' });
    addScriptToDom(data);
  });
});
