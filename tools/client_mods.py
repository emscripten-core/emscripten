
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

  // The minified JS variable for Math.fround might contain the '$' sign, so this must be escaped to \$ to be used as a search pattern.
  minified = minified.replace(/\$/g, "\\\\$$");

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
      return ['''if (typeof SharedArrayBuffer === "undefined") {
try {
  console.log('This browser does not support SharedArrayBuffer/Atomics/pthreads! Patching out SharedArrayBuffer usage...');
  var t0 = performance.now();

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
  var atomics_exchange = /var\s+([^=]+?)\s*=\s*global\.Atomics\.exchange;/.exec(code)[1];
  var atomics_compareExchange = /var\s+([^=]+?)\s*=\s*global\.Atomics\.compareExchange;/.exec(code)[1];
  var atomics_add = /var\s+([^=]+?)\s*=\s*global\.Atomics\.add;/.exec(code)[1];
  var atomics_sub = /var\s+([^=]+?)\s*=\s*global\.Atomics\.sub;/.exec(code)[1];
  var atomics_and = /var\s+([^=]+?)\s*=\s*global\.Atomics\.and;/.exec(code)[1];
  var atomics_or = /var\s+([^=]+?)\s*=\s*global\.Atomics\.or;/.exec(code)[1];
  var atomics_xor = /var\s+([^=]+?)\s*=\s*global\.Atomics\.xor;/.exec(code)[1];

  // JS variables may contain the '$' sign, so these must be escaped. However,
  // the '$' sign needs to be escaped differently depending on whether it's on the
  // string to search for side (espace by '\\'), or the value to replace
  // with side (escape by '$').
  function escapeDollarForRegexSearch(str) { return str.replace(/\$/g, "\\\\$$"); }
  function escapeDollarForRegexValue(str) { return str.replace(/\$/g, "$$$$"); }

  var wb = '([^\\\\w\\\\$])'; // word break (one character, which is backinserted)

  var s_heap8 = escapeDollarForRegexSearch(heap8);
  var s_heap16 = escapeDollarForRegexSearch(heap16);
  var s_heap32 = escapeDollarForRegexSearch(heap32);
  var s_heapf32 = escapeDollarForRegexSearch(heapf32);
  var s_heapf64 = escapeDollarForRegexSearch(heapf64);

  // The Atomics built-ins take as first parameter the heap object, however when replacing those with
  // polyfill versions, it is not possible to pass a heap object as the first parameter. Therefore
  // route each call to Atomics to a polyfill function for each type, e.g. "Atomics_add(HEAP32, index, val)" -> "Atomics_add_32(index, val)"
  var s_atomics_load = escapeDollarForRegexSearch(atomics_load);
  var v_atomics_load = escapeDollarForRegexValue(atomics_load);
  code = code.replace(new RegExp(wb + s_atomics_load + '\\\\('+s_heap8+',', 'g'), '$1' + v_atomics_load + "_8(");
  code = code.replace(new RegExp(wb + s_atomics_load + '\\\\('+s_heap16+',', 'g'), '$1' + v_atomics_load + "_16(");
  code = code.replace(new RegExp(wb + s_atomics_load + '\\\\('+s_heap32+',', 'g'), '$1' + v_atomics_load + "_32(");
  code = code.replace(new RegExp(wb + s_atomics_load + '\\\\('+s_heapf32+',', 'g'), '$1' + v_atomics_load + "_f32(");
  code = code.replace(new RegExp(wb + s_atomics_load + '\\\\('+s_heapf64+',', 'g'), '$1' + v_atomics_load + "_f64(");

  var s_atomics_store = escapeDollarForRegexSearch(atomics_store);
  var v_atomics_store = escapeDollarForRegexValue(atomics_store);
  code = code.replace(new RegExp(wb + s_atomics_store + '\\\\('+s_heap8+',', 'g'), '$1' + v_atomics_store + "_8(");
  code = code.replace(new RegExp(wb + s_atomics_store + '\\\\('+s_heap16+',', 'g'), '$1' + v_atomics_store + "_16(");
  code = code.replace(new RegExp(wb + s_atomics_store + '\\\\('+s_heap32+',', 'g'), '$1' + v_atomics_store + "_32(");
  code = code.replace(new RegExp(wb + s_atomics_store + '\\\\('+s_heapf32+',', 'g'), '$1' + v_atomics_store + "_f32(");
  code = code.replace(new RegExp(wb + s_atomics_store + '\\\\('+s_heapf64+',', 'g'), '$1' + v_atomics_store + "_f64(");

  var s_atomics_add = escapeDollarForRegexSearch(atomics_add);
  var v_atomics_add = escapeDollarForRegexValue(atomics_add);
  code = code.replace(new RegExp(wb + s_atomics_add + '\\\\('+s_heap8+',', 'g'), '$1' + v_atomics_add + "_8(");
  code = code.replace(new RegExp(wb + s_atomics_add + '\\\\('+s_heap16+',', 'g'), '$1' + v_atomics_add + "_16(");
  code = code.replace(new RegExp(wb + s_atomics_add + '\\\\('+s_heap32+',', 'g'), '$1' + v_atomics_add + "_32(");

  var s_atomics_sub = escapeDollarForRegexSearch(atomics_sub);
  var v_atomics_sub = escapeDollarForRegexValue(atomics_sub);
  code = code.replace(new RegExp(wb + s_atomics_sub + '\\\\('+s_heap8+',', 'g'), '$1' + v_atomics_sub + "_8(");
  code = code.replace(new RegExp(wb + s_atomics_sub + '\\\\('+s_heap16+',', 'g'), '$1' + v_atomics_sub + "_16(");
  code = code.replace(new RegExp(wb + s_atomics_sub + '\\\\('+s_heap32+',', 'g'), '$1' + v_atomics_sub + "_32(");

  var s_atomics_and = escapeDollarForRegexSearch(atomics_and);
  var v_atomics_and = escapeDollarForRegexValue(atomics_and);
  code = code.replace(new RegExp(wb + s_atomics_and + '\\\\('+s_heap8+',', 'g'), '$1' + v_atomics_and + "_8(");
  code = code.replace(new RegExp(wb + s_atomics_and + '\\\\('+s_heap16+',', 'g'), '$1' + v_atomics_and + "_16(");
  code = code.replace(new RegExp(wb + s_atomics_and + '\\\\('+s_heap32+',', 'g'), '$1' + v_atomics_and + "_32(");

  var s_atomics_or = escapeDollarForRegexSearch(atomics_or);
  var v_atomics_or = escapeDollarForRegexValue(atomics_or);
  code = code.replace(new RegExp(wb + s_atomics_or + '\\\\('+s_heap8+',', 'g'), '$1' + v_atomics_or + "_8(");
  code = code.replace(new RegExp(wb + s_atomics_or + '\\\\('+s_heap16+',', 'g'), '$1' + v_atomics_or + "_16(");
  code = code.replace(new RegExp(wb + s_atomics_or + '\\\\('+s_heap32+',', 'g'), '$1' + v_atomics_or + "_32(");

  var s_atomics_xor = escapeDollarForRegexSearch(atomics_xor);
  var v_atomics_xor = escapeDollarForRegexValue(atomics_xor);
  code = code.replace(new RegExp(wb + s_atomics_xor + '\\\\('+s_heap8+',', 'g'), '$1' + v_atomics_xor + "_8(");
  code = code.replace(new RegExp(wb + s_atomics_xor + '\\\\('+s_heap16+',', 'g'), '$1' + v_atomics_xor + "_16(");
  code = code.replace(new RegExp(wb + s_atomics_xor + '\\\\('+s_heap32+',', 'g'), '$1' + v_atomics_xor + "_32(");

  var s_atomics_exchange = escapeDollarForRegexSearch(atomics_exchange);
  var v_atomics_exchange = escapeDollarForRegexValue(atomics_exchange);
  code = code.replace(new RegExp(wb + s_atomics_exchange + '\\\\('+s_heap8+',', 'g'), '$1' + v_atomics_exchange + "_8(");
  code = code.replace(new RegExp(wb + s_atomics_exchange + '\\\\('+s_heap16+',', 'g'), '$1' + v_atomics_exchange + "_16(");
  code = code.replace(new RegExp(wb + s_atomics_exchange + '\\\\('+s_heap32+',', 'g'), '$1' + v_atomics_exchange + "_32(");

  var s_atomics_compareExchange = escapeDollarForRegexSearch(atomics_compareExchange);
  var v_atomics_compareExchange = escapeDollarForRegexValue(atomics_compareExchange);
  code = code.replace(new RegExp(wb + s_atomics_compareExchange + '\\\\('+s_heap8+',', 'g'), '$1' + v_atomics_compareExchange + "_8(");
  code = code.replace(new RegExp(wb + s_atomics_compareExchange + '\\\\('+s_heap16+',', 'g'), '$1' + v_atomics_compareExchange + "_16(");
  code = code.replace(new RegExp(wb + s_atomics_compareExchange + '\\\\('+s_heap32+',', 'g'), '$1' + v_atomics_compareExchange + "_32(");

  // Remove the import statements of Atomics built-ins.
  code = code.replace(new RegExp("var " + s_atomics_load + "\\\\s*=\\\\s*global\\.Atomics\\.load;"), "");
  code = code.replace(new RegExp("var " + s_atomics_store + "\\\\s*=\\\\s*global\\.Atomics\\.store;"), "");
  code = code.replace(new RegExp("var " + s_atomics_exchange + "\\\\s*=\\\\s*global\\.Atomics\\.exchange;"), "");
  code = code.replace(new RegExp("var " + s_atomics_compareExchange + "\\\\s*=\\\\s*global\\.Atomics\\.compareExchange;"), "");
  code = code.replace(new RegExp("var " + s_atomics_add + "\\\\s*=\\\\s*global\\.Atomics\\.add;"), "");
  code = code.replace(new RegExp("var " + s_atomics_sub + "\\\\s*=\\\\s*global\\.Atomics\\.sub;"), "");
  code = code.replace(new RegExp("var " + s_atomics_and + "\\\\s*=\\\\s*global\\.Atomics\\.and;"), "");
  code = code.replace(new RegExp("var " + s_atomics_or + "\\\\s*=\\\\s*global\\.Atomics\\.or;"), "");
  code = code.replace(new RegExp("var " + s_atomics_xor + "\\\\s*=\\\\s*global\\.Atomics\\.xor;"), "");

  // Implement polyfill versions of Atomics intrinsics inside the asm.js scope.
  code = code.replace("// EMSCRIPTEN_START_FUNCS", "// EMSCRIPTEN_START_FUNCS\\n"
    + "function " + atomics_load + "_8(i) { i=i|0; return "+heap8+"[i>>0]|0; }\\n"
    + "function " + atomics_load + "_16(i) { i=i|0; return "+heap16+"[i<<1>>1]|0; }\\n"
    + "function " + atomics_load + "_32(i) { i=i|0; return "+heap32+"[i<<2>>2]|0; }\\n"
    + "function " + atomics_load + "_f32(i) { i=i|0; return "+math_fround+heapf32+"[i<<2>>2]"+cp+"; }\\n"
    + "function " + atomics_load + "_f64(i) { i=i|0; return +"+heapf64+"[i<<3>>3]; }\\n"

    + "function " + atomics_store + "_8(i,v) { i=i|0; v=v|0; "+heap8+"[i>>0]=v; return v|0; }\\n"
    + "function " + atomics_store + "_16(i,v) { i=i|0; v=v|0; "+heap16+"[i<<1>>1]=v; return v|0; }\\n"
    + "function " + atomics_store + "_32(i,v) { i=i|0; v=v|0; "+heap32+"[i<<2>>2]=v; return v|0; }\\n"
    + "function " + atomics_store + "_f32(i,v) { i=i|0; v="+math_fround+"v"+cp+"; "+heapf32+"[i<<2>>2]=v; return "+math_fround+"v"+cp+"; }\\n"
    + "function " + atomics_store + "_f64(i,v) { i=i|0; v=+v;" +heapf64+"[i<<3>>3]=v; return +v; }\\n"

    + "function " + atomics_add + "_8(i,v) { i=i|0; v=v|0; var w=0; w="+heap8+"[i>>0]|0; "+heap8+"[i>>0]=("+heap8+"[i>>0]|0)+(v|0); return w|0; }\\n"
    + "function " + atomics_add + "_16(i,v) { i=i|0; v=v|0; var w=0; w="+heap16+"[i<<1>>1]|0; "+heap16+"[i<<1>>1]=("+heap16+"[i<<1>>1]|0)+(v|0); return w|0; }\\n"
    + "function " + atomics_add + "_32(i,v) { i=i|0; v=v|0; var w=0; w="+heap32+"[i<<2>>2]|0; "+heap32+"[i<<2>>2]=("+heap32+"[i<<2>>2]|0)+(v|0); return w|0; }\\n"

    + "function " + atomics_sub + "_8(i,v) { i=i|0; v=v|0; var w=0; w="+heap8+"[i>>0]|0; "+heap8+"[i>>0]=("+heap8+"[i>>0]|0)-(v|0); return w|0; }\\n"
    + "function " + atomics_sub + "_16(i,v) { i=i|0; v=v|0; var w=0; w="+heap16+"[i<<1>>1]|0; "+heap16+"[i<<1>>1]=("+heap16+"[i<<1>>1]|0)-(v|0); return w|0; }\\n"
    + "function " + atomics_sub + "_32(i,v) { i=i|0; v=v|0; var w=0; w="+heap32+"[i<<2>>2]|0; "+heap32+"[i<<2>>2]=("+heap32+"[i<<2>>2]|0)-(v|0); return w|0; }\\n"

    + "function " + atomics_and + "_8(i,v) { i=i|0; v=v|0; var w=0; w="+heap8+"[i>>0]|0; "+heap8+"[i>>0]=("+heap8+"[i>>0]|0)&(v|0); return w|0; }\\n"
    + "function " + atomics_and + "_16(i,v) { i=i|0; v=v|0; var w=0; w="+heap16+"[i<<1>>1]|0; "+heap16+"[i<<1>>1]=("+heap16+"[i<<1>>1]|0)&(v|0); return w|0; }\\n"
    + "function " + atomics_and + "_32(i,v) { i=i|0; v=v|0; var w=0; w="+heap32+"[i<<2>>2]|0; "+heap32+"[i<<2>>2]=("+heap32+"[i<<2>>2]|0)&(v|0); return w|0; }\\n"

    + "function " + atomics_or + "_8(i,v) { i=i|0; v=v|0; var w=0; w="+heap8+"[i>>0]|0; "+heap8+"[i>>0]=("+heap8+"[i>>0]|0)|(v|0); return w|0; }\\n"
    + "function " + atomics_or + "_16(i,v) { i=i|0; v=v|0; var w=0; w="+heap16+"[i<<1>>1]|0; "+heap16+"[i<<1>>1]=("+heap16+"[i<<1>>1]|0)|(v|0); return w|0; }\\n"
    + "function " + atomics_or + "_32(i,v) { i=i|0; v=v|0; var w=0; w="+heap32+"[i<<2>>2]|0; "+heap32+"[i<<2>>2]=("+heap32+"[i<<2>>2]|0)|(v|0); return w|0; }\\n"

    + "function " + atomics_xor + "_8(i,v) { i=i|0; v=v|0; var w=0; w="+heap8+"[i>>0]|0; "+heap8+"[i>>0]=("+heap8+"[i>>0]|0)^(v|0); return w|0; }\\n"
    + "function " + atomics_xor + "_16(i,v) { i=i|0; v=v|0; var w=0; w="+heap16+"[i<<1>>1]|0; "+heap16+"[i<<1>>1]=("+heap16+"[i<<1>>1]|0)^(v|0); return w|0; }\\n"
    + "function " + atomics_xor + "_32(i,v) { i=i|0; v=v|0; var w=0; w="+heap32+"[i<<2>>2]|0; "+heap32+"[i<<2>>2]=("+heap32+"[i<<2>>2]|0)^(v|0); return w|0; }\\n"

    + "function " + atomics_exchange + "_8(i,e,r) { i=i|0; e=e|0; r=r|0; var w=0; w="+heap8+"[i>>0]|0; "+heap8+"[i>>0]=r; return w|0; }\\n"
    + "function " + atomics_exchange + "_16(i,e,r) { i=i|0; e=e|0; r=r|0; var w=0; w="+heap16+"[i<<1>>1]|0; "+heap16+"[i<<1>>1]=r; return w|0; }\\n"
    + "function " + atomics_exchange + "_32(i,e,r) { i=i|0; e=e|0; r=r|0; var w=0; w="+heap32+"[i<<2>>2]|0; "+heap32+"[i<<2>>2]=r; return w|0; }\\n"

    + "function " + atomics_compareExchange + "_8(i,e,r) { i=i|0; e=e|0; r=r|0; var w=0; w="+heap8+"[i>>0]|0; if ((w|0) == (e|0)) "+heap8+"[i>>0]=r; return w|0; }\\n"
    + "function " + atomics_compareExchange + "_16(i,e,r) { i=i|0; e=e|0; r=r|0; var w=0; w="+heap16+"[i<<1>>1]|0; if ((w|0) == (e|0)) "+heap16+"[i<<1>>1]=r; return w|0; }\\n"
    + "function " + atomics_compareExchange + "_32(i,e,r) { i=i|0; e=e|0; r=r|0; var w=0; w="+heap32+"[i<<2>>2]|0; if ((w|0) == (e|0)) "+heap32+"[i<<2>>2]=r; return w|0; }\\n"
    );

  var t1 = performance.now();
  console.log('SAB+Atomics removed in ' + (t1-t0) + ' msecs.');
} catch(e) { console.log('Failed to optimize out SharedArrayBuffer calls ' + e); }
} // if no SharedArrayBuffer
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

