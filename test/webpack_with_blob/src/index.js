fetch('hello.js').then((resp) => resp.blob().then((blob) => {
  var params_blob = {
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
    mainScriptUrlOrBlob: blob
  };

  params_blob.print("testing..");

  var script = document.createElement("script");
  script.src = URL.createObjectURL(blob);
  script.onload = () => {
    Module(params_blob).then((instance) => { console.log("loaded") })
  };
  document.body.appendChild(script);
}))
