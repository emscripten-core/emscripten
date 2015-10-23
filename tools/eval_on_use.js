/*
This code loads an asm.js module, and compresses it so each large-enough function is just a thunk,
that calls eval to compile itself from the full string text kept on the side. This lets only
actually called methods be parsed and optimized.
*/

var originalXHR = new XMLHttpRequest();
originalXHR.open('GET', 'a.asm.js', true);
originalXHR.onload = function() {
  var code = originalXHR.responseText;
  originalXHR = null;
  Module.code = code;
  // compress large-enough methods
  var startTime = Date.now();
  var compressed = new Uint8Array(code.length); // will be shorter
  var i = 0, c = 0;
  function writeRange(start, end) {
    for (var j = start; j < end; j++) {
      compressed[c++] = code.charCodeAt(j);
    }
  }
  function writeString(str) {
    for (var j = 0; j < str.length; j++) {
      compressed[c++] = str.charCodeAt(j);
    }
  }
  while (1) {
    var funcStart = code.indexOf('function ', i);
    if (funcStart < 0) {
      // all done
      writeRange(i, code.length);
      break;
    }
    // copy all the stuff until the function
    writeRange(i, funcStart);
    var funcEnd = funcStart;
    var nest = 0;
    while (1) {
      var curr = code[funcEnd++];
      if (curr === '{') {
        nest++;
      } else if (curr === '}') {
        nest--;
        if (nest === 0) {
          break;
        }
      }
    }
    // function is at [funcStart..funcEnd)
    i = funcEnd; // prepare for next iteration
    var size = funcEnd - funcStart;
    var paren = code.indexOf('(', funcStart);
    var paren2 = code.indexOf(')', funcStart);
    var name = code.substring(funcStart + 9, paren);
    var args = code.substring(paren + 1, paren2);
    var thunk = 'function ' + name + '(' + args + '){if(!' + name + '.Q){' + name + '=eval("(" + Module.code.substring(' + funcStart + ',' + funcEnd + ') + ")");' + name + '.Q=1}return ' + name + '(' + args + ')}';
    if (size < thunk.length + 100) {
      // not worth it
      writeRange(funcStart, funcEnd);
      continue;
    }
    // compressible
    writeString(thunk);
    continue;
  }
  if (!(c < code.length)) throw 'horrible ' + [c, code.length];
  // remove the  use asm  string
  var useAsm = code.lastIndexOf('"use asm"', 1024);
  if (useAsm < 0) {
    useAsm = code.lastIndexOf("'use asm'", 1024);
  }
  if (useAsm > 0) {
    compressed[useAsm + 1] = 'N'.charCodeAt(0);
    compressed[useAsm + 2] = 'O'.charCodeAt(0);
    compressed[useAsm + 3] = '!'.charCodeAt(0);
  }
  // finish
  compressed = new Uint8Array(compressed.subarray(0, c)); // we do a copy here, in order to be able to throw away memory
  console.log('compressed asm from ' + (code.length/(1024*1024)).toFixed(2) + ' MB to ' + (compressed.length/(1024*1024)).toFixed(2) + ' MB, in ' + ((Date.now() - startTime)/1000).toFixed(2) + ' seconds');

  /* debug
  var test = '';
  for (var i = 0; i < compressed.length; i++) {
    test += String.fromCharCode(compressed[i]);
  }
  Module.print(test);
  */

  var asmScript = document.createElement('script');
  asmScript.onload = function() {
    URL.revokeObjectURL(asmScript.src);
    // load the rest
    var mainScript = document.createElement('script');
    mainScript.src = "a.js";
    document.body.appendChild(mainScript);
    //
  };
  asmScript.src = URL.createObjectURL(new Blob([compressed], { type: 'text/javascript' }));
  //asmScript.innerHTML = test;
  document.body.appendChild(asmScript);
};
originalXHR.send(null);

