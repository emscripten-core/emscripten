var params_url = {
  print: (function() {
    var element = document.getElementById('output');
    return function(text) {
      console.log(text);
      element.innerHTML += text.replace('\n', '<br>', 'g') + '<br>';
    };
  })(),
  locateFile: function(path, _prefix) {
    return path;
  },
  canvas: document.getElementById('canvas'),
  mainScriptUrlOrBlob: 'hello.js'
};

params_url.print("testing..");

var script = document.createElement("script");
script.src = params_url['mainScriptUrlOrBlob'];
script.onload = () => {
  Module(params_url).then((instance) => { console.log("loaded") })
};
document.body.appendChild(script);
