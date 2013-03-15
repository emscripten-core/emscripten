
var EmJVM = {
  debug: false,

  nextId: 0,
  objects: {},
  classNames: {}, // class name => singleton object

  addObject: function(o) {
    var ret = EmJVM.nextId++;
    EmJVM.objects[ret] = o;
    o.id = ret;
    o.refs = 1;
    o.nextMethodId = 0;
    // XXX Module.print('add object ' + JSON.stringify(o).substr(0, 80) + (ret > 5285 ? new Error().stack : ''));
    return ret;
  },

  addSingletonObject: function(o) {
    EmJVM.classNames[o.name] = o;
    return EmJVM.addObject(o);
  },

  createString: function(data) {
    return EmJVM.addObject({ name: 'string', value: data });
  },

  createByteArray: function(data) {
    return EmJVM.addObject({ name: 'byteArray', value: data });
  },
};

function widecharToString(ptr, len) {
  var nullTerminated = typeof(len) == "undefined";
  var ret = "";
  var i = 0;
  var t;
  while (1) {
    t = getValue(ptr + 2 * i, 'i16');
    if (nullTerminated && t == 0) break;
    if (t != 0) {
      ret += String.fromCharCode(t);
    }
    ++i;
    if (!nullTerminated && i == len) break;
  };
  return ret;
}

function _emjvm_newString(chars, len) {
  return EmJVM.createString(widecharToString(chars, len));
}

function _emjvm_getStringUTFChars(jniEnv, string, isCopy) {
  var obj = EmJVM.objects[string];
  assert(obj.name == 'string');
  if (isCopy) setValue(isCopy, 'i8', 1);
  var buffer = _malloc(obj.value.length+1);
  writeStringToMemory(obj.value, buffer);
  return buffer;
}

function _emjvm_getStringUTFLength(jniEnv, string) {
  var obj = EmJVM.objects[string];
  if (obj.value) {
    return obj.value.length;
  }
  return 0;
}

function _emjvm_releaseStringUTFChars(jniEnv, string, utf) {
}

function _emjvm_getObjectClass(env, jobject) {
  if (EmJVM.debug) {
    console.log('EMJVM_GetObjectClass+AddLocalRef: ' + [jobject]);
  }
  var obj = EmJVM.objects[jobject];
  obj.refs++;
  return jobject;
}

function _emjvm_getMethodID(jclass, name, sig) {
  if (EmJVM.debug) {
    console.log('EMJVM_GetMethodID: ' + [jclass, Pointer_stringify(name), Pointer_stringify(sig)]);
    console.log('EMJVM_GetMethodID: ' + [EmJVM.objects[jclass].name]);
  }
  // assumes class <--> object, just called on singletons
  name = Pointer_stringify(name);
  var obj = EmJVM.objects[jclass];
  if (!obj[name]) {
    throw 'missing implementation for ' + obj.name + '::' + name + ' : ' + new Error().stack;
  }
  if (!obj[name + '__methodId']) {
    var methodId = obj.nextMethodId++;
    obj[name + '__methodId'] = methodId;
    obj['method__' + methodId] = obj[name];
    obj['methodName__' + methodId] = name;
  }
  return obj[name + '__methodId'];
}

function _emjvm_getStaticMethodID(jniEnv, jclass, name, sig) {
  // Pretend this to be the same as looking up a non-static method
  return _emjvm_getMethodID(jclass, name, sig);
}

function _emjvm_callObjectMethod(jniEnv, jobject, methodId, varargs) {
  if (EmJVM.debug) {
    console.log('EMJVM_CallObjectMethod: ' + [jobject, EmJVM.objects[jobject].name, methodId, EmJVM.objects[jobject]['methodName__' + methodId]]);
  }
  return EmJVM.objects[jobject]['method__' + methodId](varargs);
}

function _emjvm_callStaticObjectMethod(jniEnv, jclass, methodId, varargs) {
  // Pretend this to be the same as calling a non-static method
  return _emjvm_callObjectMethod(jniEnv, jclass, methodId, varargs);
}

function _emjvm_callStaticBooleanMethod(jniEnv, jclass, methodId, varargs) {
  // Only differs in return type
  return _emjvm_callStaticObjectMethod(jniEnv, jclass, methodId, varargs);
}

function _emjvm_callBooleanMethod(jniEnv, jobject, methodId, varargs) {
  // Pretend this to be the same as calling a non-static method
  return _emjvm_callStaticBooleanMethod(jniEnv, jobject, methodId, varargs);
}

function _emjvm_callVoidMethod(jniEnv, jobject, methodId, varargs) {
  _emjvm_callObjectMethod(jniEnv, jobject, methodId, varargs);
}

function _emjvm_callIntMethod(jniEnv, jobject, methodId, varargs) {
  return _emjvm_callObjectMethod(jniEnv, jobject, methodId, varargs);
}

function _emjvm_deleteLocalRef(jniEnv, jobject) {
  if (EmJVM.debug) {
    console.log('EMJVM_DeleteLocalRef: ' + [jobject]);
  }
  var obj = EmJVM.objects[jobject];
  obj.refs--;
  if (obj.refs == 0) {
    if (EmJVM.debug) {
      console.log('EMJVM_DeleteLocalRef: remove ' + obj.name);
    }
    delete EmJVM.objects[jobject];
  }
}

function _emjvm_getArrayLength(jniEnv, jobject) {
  var obj = EmJVM.objects[jobject];
  assert(obj.name == 'byteArray');
  return obj.value.length;
}

function _emjvm_getByteArrayRegion(jniEnv, jobject, start, len, buf) {
  var obj = EmJVM.objects[jobject];
  assert(obj.name == 'byteArray');
  assert(obj.value); // we set this to null below and assume we are never called again
  if (EmJVM.debug) {
    console.log('emjvm_getByteArrayRegion: ' + [jobject, obj.value.length, start, len, buf]);
  }
  assert(start + len <= obj.value.length);
  assert(len == obj.value.length); // we assume users read it all, and we can now copy it all with set() and then free it
  HEAPU8.set(obj.value, buf);
  obj.value = null; // XXX assume byte arrays are one-shot
}

function _emjvm_findClass(env, name) {
  name = Pointer_stringify(name);
  if (EmJVM.debug) {
    console.log('emjvm_findClass: ' + [name]);
  }
  var obj = EmJVM.classNames[name];
  assert(obj);
  return obj.id;
}

