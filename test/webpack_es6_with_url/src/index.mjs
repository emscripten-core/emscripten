var params_url = {
  print: (function() {
    var element = document.getElementById('output');
    return function(text) {
      console.log(text);
      element.innerHTML += text.replace('\n', '<br>', 'g') + '<br>';
    };
  })(),
  locateFile: function(path, prefix) {
    return path;
  },
  canvas: document.getElementById('canvas'),
  mainScriptUrlOrBlob: 'hello.mjs'
};

params_url.print('testing...');
import(/* webpackIgnore: true */ 'hello.mjs').then((factory) => {
  factory.default(params_url).then((instance) => {
    console.log('loaded by url');
  });
});
