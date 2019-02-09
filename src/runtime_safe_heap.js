#if SAFE_HEAP || !MINIMAL_RUNTIME
// In MINIMAL_RUNTIME, setValue() and getValue() are only available when building with safe heap enabled, for heap safety checking.
// In traditional runtime, setValue() and getValue() are always available (although their use is highly discouraged due to perf penalties)

/** @type {function(number, number, string, boolean=)} */
function setValue(ptr, value, type, noSafe) {
  type = type || 'i8';
  if (type.charAt(type.length-1) === '*') type = 'i32'; // pointers are 32-bit
#if SAFE_HEAP
  if (noSafe) {
    switch(type) {
      case 'i1': {{{ makeSetValue('ptr', '0', 'value', 'i1', undefined, undefined, undefined, '1') }}}; break;
      case 'i8': {{{ makeSetValue('ptr', '0', 'value', 'i8', undefined, undefined, undefined, '1') }}}; break;
      case 'i16': {{{ makeSetValue('ptr', '0', 'value', 'i16', undefined, undefined, undefined, '1') }}}; break;
      case 'i32': {{{ makeSetValue('ptr', '0', 'value', 'i32', undefined, undefined, undefined, '1') }}}; break;
      case 'i64': {{{ makeSetValue('ptr', '0', 'value', 'i64', undefined, undefined, undefined, '1') }}}; break;
      case 'float': {{{ makeSetValue('ptr', '0', 'value', 'float', undefined, undefined, undefined, '1') }}}; break;
      case 'double': {{{ makeSetValue('ptr', '0', 'value', 'double', undefined, undefined, undefined, '1') }}}; break;
      default: abort('invalid type for setValue: ' + type);
    }
  } else {
#endif
    switch(type) {
      case 'i1': {{{ makeSetValue('ptr', '0', 'value', 'i1') }}}; break;
      case 'i8': {{{ makeSetValue('ptr', '0', 'value', 'i8') }}}; break;
      case 'i16': {{{ makeSetValue('ptr', '0', 'value', 'i16') }}}; break;
      case 'i32': {{{ makeSetValue('ptr', '0', 'value', 'i32') }}}; break;
      case 'i64': {{{ makeSetValue('ptr', '0', 'value', 'i64') }}}; break;
      case 'float': {{{ makeSetValue('ptr', '0', 'value', 'float') }}}; break;
      case 'double': {{{ makeSetValue('ptr', '0', 'value', 'double') }}}; break;
      default: abort('invalid type for setValue: ' + type);
    }
#if SAFE_HEAP
  }
#endif
}

/** @type {function(number, string, boolean=)} */
function getValue(ptr, type, noSafe) {
  type = type || 'i8';
  if (type.charAt(type.length-1) === '*') type = 'i32'; // pointers are 32-bit
#if SAFE_HEAP
  if (noSafe) {
    switch(type) {
      case 'i1': return {{{ makeGetValue('ptr', '0', 'i1', undefined, undefined, undefined, undefined, '1') }}};
      case 'i8': return {{{ makeGetValue('ptr', '0', 'i8', undefined, undefined, undefined, undefined, '1') }}};
      case 'i16': return {{{ makeGetValue('ptr', '0', 'i16', undefined, undefined, undefined, undefined, '1') }}};
      case 'i32': return {{{ makeGetValue('ptr', '0', 'i32', undefined, undefined, undefined, undefined, '1') }}};
      case 'i64': return {{{ makeGetValue('ptr', '0', 'i64', undefined, undefined, undefined, undefined, '1') }}};
      case 'float': return {{{ makeGetValue('ptr', '0', 'float', undefined, undefined, undefined, undefined, '1') }}};
      case 'double': return {{{ makeGetValue('ptr', '0', 'double', undefined, undefined, undefined, undefined, '1') }}};
      default: abort('invalid type for getValue: ' + type);
    }
  } else {
#endif
    switch(type) {
      case 'i1': return {{{ makeGetValue('ptr', '0', 'i1') }}};
      case 'i8': return {{{ makeGetValue('ptr', '0', 'i8') }}};
      case 'i16': return {{{ makeGetValue('ptr', '0', 'i16') }}};
      case 'i32': return {{{ makeGetValue('ptr', '0', 'i32') }}};
      case 'i64': return {{{ makeGetValue('ptr', '0', 'i64') }}};
      case 'float': return {{{ makeGetValue('ptr', '0', 'float') }}};
      case 'double': return {{{ makeGetValue('ptr', '0', 'double') }}};
      default: abort('invalid type for getValue: ' + type);
    }
#if SAFE_HEAP
  }
#endif
  return null;
}
#endif // SAFE_HEAP || !MINIMAL_RUNTIME


#if SAFE_HEAP
function getSafeHeapType(bytes, isFloat) {
  switch (bytes) {
    case 1: return 'i8';
    case 2: return 'i16';
    case 4: return isFloat ? 'float' : 'i32';
    case 8: return 'double';
    default: assert(0);
  }
}

#if SAFE_HEAP_LOG
var SAFE_HEAP_COUNTER = 0;
#endif

function SAFE_HEAP_STORE(dest, value, bytes, isFloat) {
#if SAFE_HEAP_LOG
  out('SAFE_HEAP store: ' + [dest, value, bytes, isFloat, SAFE_HEAP_COUNTER++]);
#endif
  if (dest <= 0) abort('segmentation fault storing ' + bytes + ' bytes to address ' + dest);
  if (dest % bytes !== 0) abort('alignment error storing to address ' + dest + ', which was expected to be aligned to a multiple of ' + bytes);
  if (dest + bytes > HEAP32[DYNAMICTOP_PTR>>2]) abort('segmentation fault, exceeded the top of the available dynamic heap when storing ' + bytes + ' bytes to address ' + dest + '. DYNAMICTOP=' + HEAP32[DYNAMICTOP_PTR>>2]);
  assert(DYNAMICTOP_PTR);
  assert(HEAP32[DYNAMICTOP_PTR>>2] <= HEAP8.length);
  setValue(dest, value, getSafeHeapType(bytes, isFloat), 1);
}
function SAFE_HEAP_STORE_D(dest, value, bytes) {
  SAFE_HEAP_STORE(dest, value, bytes, true);
}

function SAFE_HEAP_LOAD(dest, bytes, unsigned, isFloat) {
  if (dest <= 0) abort('segmentation fault loading ' + bytes + ' bytes from address ' + dest);
  if (dest % bytes !== 0) abort('alignment error loading from address ' + dest + ', which was expected to be aligned to a multiple of ' + bytes);
  if (dest + bytes > HEAP32[DYNAMICTOP_PTR>>2]) abort('segmentation fault, exceeded the top of the available dynamic heap when loading ' + bytes + ' bytes from address ' + dest + '. DYNAMICTOP=' + HEAP32[DYNAMICTOP_PTR>>2]);
  assert(DYNAMICTOP_PTR);
  assert(HEAP32[DYNAMICTOP_PTR>>2] <= HEAP8.length);
  var type = getSafeHeapType(bytes, isFloat);
  var ret = getValue(dest, type, 1);
  if (unsigned) ret = unSign(ret, parseInt(type.substr(1)), 1);
#if SAFE_HEAP_LOG
  out('SAFE_HEAP load: ' + [dest, ret, bytes, isFloat, unsigned, SAFE_HEAP_COUNTER++]);
#endif
  return ret;
}
function SAFE_HEAP_LOAD_D(dest, bytes, unsigned) {
  return SAFE_HEAP_LOAD(dest, bytes, unsigned, true);
}

function SAFE_FT_MASK(value, mask) {
  var ret = value & mask;
  if (ret !== value) {
    abort('Function table mask error: function pointer is ' + value + ' which is masked by ' + mask + ', the likely cause of this is that the function pointer is being called by the wrong type.');
  }
  return ret;
}

function segfault() {
  abort('segmentation fault');
}
function alignfault() {
  abort('alignment fault');
}
function ftfault() {
  abort('Function table mask error');
}
#endif
