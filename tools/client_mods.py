
class PreciseF32:
  name = 'PRECISE_F32 == 2'

  @staticmethod
  def get(settings, minified):
    if settings.PRECISE_F32 == 2:
      # Potentially-modifiable code, load as text, modify, then execute. This lets you
      # patch the code on the client machine right before it is executed, perhaps based
      # on information about the client.
      mod = '''
console.log('optimizing out Math.fround calls');
code = code.replace("'use asm'", "'almost asm'").replace('"use asm"', '"almost asm"');
'''
      if not minified:
        # simple dumb replace
        mod += "code = code.replace(/Math_fround\(/g, '(')\n";
      else:
        # minified, not quite so simple
        mod += '''
try {
  console.log('optimizing out Math.fround calls');
  var m = /var ([^=]+)=global\.Math\.fround;/.exec(code);
  var minified = m[1];
  if (!minified) throw 'fail';
  do {
    var moar = false; // we need to re-do, as x(x( will not be fixed
    code = code.replace(new RegExp('[^a-zA-Z0-9\\\\$\\\\_]' + minified + '\\\\(', 'g'), function(s) { moar = true; return s[0] + '(' });
  } while (moar);
} catch(e) { console.log('failed to optimize out Math.fround calls ' + e) }
'''
      return ['if (!Math.fround) { ' + mod + ' }']
    return []

class Pthreads:
  name = 'USE_PTHREADS == 2'

  @staticmethod
  def get(settings, minified):
    if settings.USE_PTHREADS == 2:
      return ['''if (typeof SharedInt8Array === "undefined") {
try {
  console.log('This browser does not support SharedArrayBuffer/Atomics/pthreads! Patching out SharedArrayBuffer usage...');
  var t0 = performance.now();
  // "new global.SharedInt8Array(buffer)" -> "new global.Int8Array(buffer)" and likewise for other buffer types.
  code = code.replace(/new\s+global\.Shared(.*?)Array\(buffer\);/g, "new global.$1Array(buffer);");

  // In minified builds the interesting symbol names are mangled, so we have to first find what they are.
  var math_fround = /var\s+([^=]+?)\s*=\s*global\.Math\.fround;/.exec(code);
  var cp;
  var zero;
  if (math_fround && math_fround.length >= 2) {
    math_fround = math_fround[1] + '(';
    cp = ')';
    zero = math_fround + '0)';
  } else {
    math_fround = "+";
    cp = '';
    zero = '0.0';
  }
  var heap8 = /var\s+([^=]+?)\s*=\s*new global\.Int8Array\(buffer\);/.exec(code)[1];
  var heap16 = /var\s+([^=]+?)\s*=\s*new global\.Int16Array\(buffer\);/.exec(code)[1];
  var heap32 = /var\s+([^=]+?)\s*=\s*new global\.Int32Array\(buffer\);/.exec(code)[1];
  var heapf32 = /var\s+([^=]+?)\s*=\s*new global\.Float32Array\(buffer\);/.exec(code)[1];
  var heapf64 = /var\s+([^=]+?)\s*=\s*new global\.Float64Array\(buffer\);/.exec(code)[1];
  var atomics_load = /var\s+([^=]+?)\s*=\s*global\.Atomics\.load;/.exec(code)[1];
  var atomics_store = /var\s+([^=]+?)\s*=\s*global\.Atomics\.store;/.exec(code)[1];
  var atomics_compareExchange = /var\s+([^=]+?)\s*=\s*global\.Atomics\.compareExchange;/.exec(code)[1];
  var atomics_add = /var\s+([^=]+?)\s*=\s*global\.Atomics\.add;/.exec(code)[1];
  var atomics_sub = /var\s+([^=]+?)\s*=\s*global\.Atomics\.sub;/.exec(code)[1];
  var atomics_and = /var\s+([^=]+?)\s*=\s*global\.Atomics\.and;/.exec(code)[1];
  var atomics_or = /var\s+([^=]+?)\s*=\s*global\.Atomics\.or;/.exec(code)[1];
  var atomics_xor = /var\s+([^=]+?)\s*=\s*global\.Atomics\.xor;/.exec(code)[1];
  var atomics_fence = /var\s+([^=]+?)\s*=\s*global\.Atomics\.fence;/.exec(code)[1];

  // "Atomics_load(HEAP32, index)" -> "HEAP32[index]|0" and same for other heap types.
  code = code.replace(new RegExp('\\\\b' + atomics_load + '\\\\((' + heap8 + ')\w*,(.*?)\\\\\)', 'g'), "($1[$2]|0)");
  code = code.replace(new RegExp('\\\\b' + atomics_load + '\\\\((' + heap16 + ')\w*,(.*?)\\\\\)', 'g'), "($1[$2]|0)");
  code = code.replace(new RegExp('\\\\b' + atomics_load + '\\\\((' + heap32 + ')\w*,(.*?)\\\\\)', 'g'), "($1[$2]|0)");
  code = code.replace(new RegExp('\\\\b' + atomics_load + '\\\\((' + heapf32 + ')\w*,(.*?)\\\\\)', 'g'), math_fround + "$1[$2]"+cp);
  code = code.replace(new RegExp('\\\\b' + atomics_load + '\\\\((' + heapf64 + ')\w*,(.*?)\\\\\)', 'g'), "(+$1[$2])");

  // "Atomics_store(HEAP32, index, value)" -> "HEAP32[index] = value" and same for other heap types.
  code = code.replace(new RegExp('\\\\b' + atomics_store + '\\\\((.*?),(.*?),(.*?)\\\\\)', 'g'), "($1[$2] = $3)");

  // The Atomics built-ins take as first parameter the heap object, however when replacing those with
  // polyfill versions, it is not possible to pass a heap object as the first parameter. Therefore
  // route each call to Atomics to a polyfill function for each type, e.g. "Atomics_add(HEAP32, index, val)" -> "Atomics_add_32(index, val)"
  code = code.replace(new RegExp('\\\\b' + atomics_add + '\\\\('+heap8+',(.*?),(.*?)\\\\\)', 'g'), '((' + atomics_add + "_8($1,$2)|0)|0)");
  code = code.replace(new RegExp('\\\\b' + atomics_add + '\\\\('+heap16+',(.*?),(.*?)\\\\\)', 'g'), '((' + atomics_add + "_16($1,$2)|0)|0)");
  code = code.replace(new RegExp('\\\\b' + atomics_add + '\\\\('+heap32+',(.*?),(.*?)\\\\\)', 'g'), '((' + atomics_add + "_32($1,$2)|0)|0)");
  code = code.replace(new RegExp('\\\\b' + atomics_add + '\\\\('+heapf32+',(.*?),(.*?)\\\\\)', 'g'), '(' + atomics_add + "_f32($1,$2)|0)");
  code = code.replace(new RegExp('\\\\b' + atomics_add + '\\\\('+heapf64+',(.*?),(.*?)\\\\\)', 'g'), '(' + atomics_add + "_f64($1,$2)|0)");

  code = code.replace(new RegExp('\\\\b' + atomics_sub + '\\\\('+heap8+',(.*?),(.*?)\\\\\)', 'g'), '((' + atomics_sub + "_8($1,$2)|0)|0)");
  code = code.replace(new RegExp('\\\\b' + atomics_sub + '\\\\('+heap16+',(.*?),(.*?)\\\\\)', 'g'), '((' + atomics_sub + "_16($1,$2)|0)|0)");
  code = code.replace(new RegExp('\\\\b' + atomics_sub + '\\\\('+heap32+',(.*?),(.*?)\\\\\)', 'g'), '((' + atomics_sub + "_32($1,$2)|0)|0)");
  code = code.replace(new RegExp('\\\\b' + atomics_sub + '\\\\('+heapf32+',(.*?),(.*?)\\\\\)', 'g'), '(' + atomics_sub + "_f32($1,$2)|0)");
  code = code.replace(new RegExp('\\\\b' + atomics_sub + '\\\\('+heapf64+',(.*?),(.*?)\\\\\)', 'g'), '(' + atomics_sub + "_f64($1,$2)|0)");

  code = code.replace(new RegExp('\\\\b' + atomics_and + '\\\\('+heap8+',(.*?),(.*?)\\\\\)', 'g'), '((' + atomics_and + "_8($1,$2)|0)|0)");
  code = code.replace(new RegExp('\\\\b' + atomics_and + '\\\\('+heap16+',(.*?),(.*?)\\\\\)', 'g'), '((' + atomics_and + "_16($1,$2)|0)|0)");
  code = code.replace(new RegExp('\\\\b' + atomics_and + '\\\\('+heap32+',(.*?),(.*?)\\\\\)', 'g'), '((' + atomics_and + "_32($1,$2)|0)|0)");

  code = code.replace(new RegExp('\\\\b' + atomics_or + '\\\\('+heap8+',(.*?),(.*?)\\\\\)', 'g'), '((' + atomics_or + "_8($1,$2)|0)|0)");
  code = code.replace(new RegExp('\\\\b' + atomics_or + '\\\\('+heap16+',(.*?),(.*?)\\\\\)', 'g'), '((' + atomics_or + "_16($1,$2)|0)|0)");
  code = code.replace(new RegExp('\\\\b' + atomics_or + '\\\\('+heap32+',(.*?),(.*?)\\\\\)', 'g'), '((' + atomics_or + "_32($1,$2)|0)|0)");

  code = code.replace(new RegExp('\\\\b' + atomics_xor + '\\\\('+heap8+',(.*?),(.*?)\\\\\)', 'g'), '((' + atomics_xor + "_8($1,$2)|0)|0)");
  code = code.replace(new RegExp('\\\\b' + atomics_xor + '\\\\('+heap16+',(.*?),(.*?)\\\\\)', 'g'), '((' + atomics_xor + "_16($1,$2)|0)|0)");
  code = code.replace(new RegExp('\\\\b' + atomics_xor + '\\\\('+heap32+',(.*?),(.*?)\\\\\)', 'g'), '((' + atomics_xor + "_32($1,$2)|0)|0)");

  code = code.replace(new RegExp('\\\\b' + atomics_compareExchange + '\\\\('+heap8+',(.*?),(.*?),(.*?)\\\\\)', 'g'), '(' + atomics_compareExchange + "_8($1,$2,$3)|0)");
  code = code.replace(new RegExp('\\\\b' + atomics_compareExchange + '\\\\('+heap16+',(.*?),(.*?),(.*?)\\\\\)', 'g'), '(' + atomics_compareExchange + "_16($1,$2,$3)|0)");
  code = code.replace(new RegExp('\\\\b' + atomics_compareExchange + '\\\\('+heap32+',(.*?),(.*?),(.*?)\\\\\)', 'g'), '(' + atomics_compareExchange + "_32($1,$2,$3)|0)");
  code = code.replace(new RegExp('\\\\b' + atomics_compareExchange + '\\\\('+heapf32+',(.*?),(.*?),(.*?)\\\\\)', 'g'), '(' + atomics_compareExchange + "_f32($1,$2,$3)|0)");
  code = code.replace(new RegExp('\\\\b' + atomics_compareExchange + '\\\\('+heapf64+',(.*?),(.*?),(.*?)\\\\\)', 'g'), '(' + atomics_compareExchange + "_f64($1,$2,$3)|0)");

  // Remove the import statements of Atomics built-ins.
  code = code.replace(new RegExp("var " + atomics_load + "\\\\s*=\\\\s*global\\.Atomics\\.load;"), "");
  code = code.replace(new RegExp("var " + atomics_store + "\\\\s*=\\\\s*global\\.Atomics\\.store;"), "");
  code = code.replace(new RegExp("var " + atomics_compareExchange + "\\\\s*=\\\\s*global\\.Atomics\\.compareExchange;"), "");
  code = code.replace(new RegExp("var " + atomics_add + "\\\\s*=\\\\s*global\\.Atomics\\.add;"), "");
  code = code.replace(new RegExp("var " + atomics_sub + "\\\\s*=\\\\s*global\\.Atomics\\.sub;"), "");
  code = code.replace(new RegExp("var " + atomics_and + "\\\\s*=\\\\s*global\\.Atomics\\.and;"), "");
  code = code.replace(new RegExp("var " + atomics_or + "\\\\s*=\\\\s*global\\.Atomics\\.or;"), "");
  code = code.replace(new RegExp("var " + atomics_xor + "\\\\s*=\\\\s*global\\.Atomics\\.xor;"), "");
  code = code.replace(new RegExp("var " + atomics_fence + "\\\\s*=\\\\s*global\\.Atomics\\.fence;"), "");

  // Implement polyfill versions of Atomics intrinsics inside the asm.js scope.
  code = code.replace("// EMSCRIPTEN_START_FUNCS", "// EMSCRIPTEN_START_FUNCS\\n"
    + "function " + atomics_add + "_8(i,v) { i=i|0; v=v|0; var w=0; w="+heap8+"[i>>0]|0; "+heap8+"[i>>0]=("+heap8+"[i>>0]|0)+(v|0); return w|0; }\\n"
    + "function " + atomics_add + "_16(i,v) { i=i|0; v=v|0; var w=0; w="+heap16+"[i<<1>>1]|0; "+heap16+"[i<<1>>1]=("+heap16+"[i<<1>>1]|0)+(v|0); return w|0; }\\n"
    + "function " + atomics_add + "_32(i,v) { i=i|0; v=v|0; var w=0; w="+heap32+"[i<<2>>2]|0; "+heap32+"[i<<2>>2]=("+heap32+"[i<<2>>2]|0)+(v|0); return w|0; }\\n"
    + "function " + atomics_add + "_f32(i,v) { i=i|0; v="+math_fround+"v"+cp+"; var w="+zero+"; w="+math_fround+heapf32+"[i<<2>>2]"+cp+"; "+heapf32+"[i<<2>>2]="+math_fround+heapf32+"[i<<2>>2]"+cp+"+v; return w; }\\n"
    + "function " + atomics_add + "_f64(i,v) { i=i|0; v=+v; var w=0.0; w=+"+heapf64+"[i<<3>>3]; "+heapf64+"[i<<3>>3]="+heapf64+"[i<<3>>3]+v; return w; }\\n"

    + "function " + atomics_sub + "_8(i,v) { i=i|0; v=v|0; var w=0; w="+heap8+"[i>>0]|0; "+heap8+"[i>>0]=("+heap8+"[i>>0]|0)-(v|0); return w|0; }\\n"
    + "function " + atomics_sub + "_16(i,v) { i=i|0; v=v|0; var w=0; w="+heap16+"[i<<1>>1]|0; "+heap16+"[i<<1>>1]=("+heap16+"[i<<1>>1]|0)-(v|0); return w|0; }\\n"
    + "function " + atomics_sub + "_32(i,v) { i=i|0; v=v|0; var w=0; w="+heap32+"[i<<2>>2]|0; "+heap32+"[i<<2>>2]=("+heap32+"[i<<2>>2]|0)-(v|0); return w|0; }\\n"
    + "function " + atomics_sub + "_f32(i,v) { i=i|0; v="+math_fround+"v"+cp+"; var w="+zero+"; w="+math_fround+heapf32+"[i<<2>>2]"+cp+"; "+heapf32+"[i<<2>>2]="+math_fround+heapf32+"[i<<2>>2]"+cp+"-v; return w; }\\n"
    + "function " + atomics_sub + "_f64(i,v) { i=i|0; v=+v; var w=0.0; w=+"+heapf64+"[i<<3>>3]; "+heapf64+"[i<<3>>3]="+heapf64+"[i<<3>>3]-v; return w; }\\n"

    + "function " + atomics_and + "_8(i,v) { i=i|0; v=v|0; var w=0; w="+heap8+"[i>>0]|0; "+heap8+"[i>>0]=("+heap8+"[i>>0]|0)&(v|0); return w|0; }\\n"
    + "function " + atomics_and + "_16(i,v) { i=i|0; v=v|0; var w=0; w="+heap16+"[i<<1>>1]|0; "+heap16+"[i<<1>>1]=("+heap16+"[i<<1>>1]|0)&(v|0); return w|0; }\\n"
    + "function " + atomics_and + "_32(i,v) { i=i|0; v=v|0; var w=0; w="+heap32+"[i<<2>>2]|0; "+heap32+"[i<<2>>2]=("+heap32+"[i<<2>>2]|0)&(v|0); return w|0; }\\n"

    + "function " + atomics_or + "_8(i,v) { i=i|0; v=v|0; var w=0; w="+heap8+"[i>>0]|0; "+heap8+"[i>>0]=("+heap8+"[i>>0]|0)|(v|0); return w|0; }\\n"
    + "function " + atomics_or + "_16(i,v) { i=i|0; v=v|0; var w=0; w="+heap16+"[i<<1>>1]|0; "+heap16+"[i<<1>>1]=("+heap16+"[i<<1>>1]|0)|(v|0); return w|0; }\\n"
    + "function " + atomics_or + "_32(i,v) { i=i|0; v=v|0; var w=0; w="+heap32+"[i<<2>>2]|0; "+heap32+"[i<<2>>2]=("+heap32+"[i<<2>>2]|0)|(v|0); return w|0; }\\n"

    + "function " + atomics_xor + "_8(i,v) { i=i|0; v=v|0; var w=0; w="+heap8+"[i>>0]|0; "+heap8+"[i>>0]=("+heap8+"[i>>0]|0)^(v|0); return w|0; }\\n"
    + "function " + atomics_xor + "_16(i,v) { i=i|0; v=v|0; var w=0; w="+heap16+"[i<<1>>1]|0; "+heap16+"[i<<1>>1]=("+heap16+"[i<<1>>1]|0)^(v|0); return w|0; }\\n"
    + "function " + atomics_xor + "_32(i,v) { i=i|0; v=v|0; var w=0; w="+heap32+"[i<<2>>2]|0; "+heap32+"[i<<2>>2]=("+heap32+"[i<<2>>2]|0)^(v|0); return w|0; }\\n"

    + "function " + atomics_compareExchange + "_8(i,e,r) { i=i|0; e=e|0; r=r|0; var w=0; w="+heap8+"[i>>0]|0; if ((w|0) == (e|0)) "+heap8+"[i>>0]=r; return w|0; }\\n"
    + "function " + atomics_compareExchange + "_16(i,e,r) { i=i|0; e=e|0; r=r|0; var w=0; w="+heap16+"[i<<1>>1]|0; if ((w|0) == (e|0)) "+heap16+"[i<<1>>1]=r; return w|0; }\\n"
    + "function " + atomics_compareExchange + "_32(i,e,r) { i=i|0; e=e|0; r=r|0; var w=0; w="+heap32+"[i<<2>>2]|0; if ((w|0) == (e|0)) "+heap32+"[i<<2>>2]=r; return w|0; }\\n"
    + "function " + atomics_compareExchange + "_f32(i,e,r) { i=i|0; e="+math_fround+"e"+cp+"; r="+math_fround+"r"+cp+"; var w="+zero+"; w="+math_fround+heapf32+"[i<<2>>2]"+cp+"; if (w == e) "+heapf32+"[i<<2>>2]=r; return w; }\\n"
    + "function " + atomics_compareExchange + "_f64(i,e,r) { i=i|0; e=+e; r=+r; var w=0.0; w=+"+heapf64+"[i<<3>>3]; if (+w == +e) "+heapf64+"[i<<3>>3]=r; return w; }\\n"

    + "function " + atomics_fence + "() {}\\n"
    );

  var t1 = performance.now();
  console.log('SAB+Atomics removed in ' + (t1-t0) + ' msecs.');
} catch(e) { console.log('Failed to optimize out SharedArrayBuffer calls ' + e); }
} // if no SharedInt8Array
''']
    return []

# Handlers

handlers = [PreciseF32, Pthreads]

# client-side asm code modification

def get_mods(settings, minified, separate_asm):
  ret = []
  for handler in handlers:
    curr = handler.get(settings, minified)
    if curr:
      assert separate_asm, 'options that modify code on the client, like ' + handler.name + ', require --separate-asm'
      ret = ret + curr
  return ret

