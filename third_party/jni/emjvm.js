mergeInto(LibraryManager.library, {
  $EmJVM: {
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

    // utils

    widecharToString: function(ptr, len) {
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
    },
  },

  emjvm_newString__deps: ['$EmJVM'],
  emjvm_newString: function(chars, len) {
    return EmJVM.createString(EmJVM.widecharToString(chars, len));
  },

  emjvm_getStringUTFChars: function(jniEnv, string, isCopy) {
    var obj = EmJVM.objects[string];
    assert(obj.name == 'string');
    if (isCopy) setValue(isCopy, 'i8', 1);
    var buffer = _malloc(obj.value.length+1);
    writeStringToMemory(obj.value, buffer);
    return buffer;
  },

  emjvm_getStringUTFLength: function(jniEnv, string) {
    var obj = EmJVM.objects[string];
    if (obj.value) {
      return obj.value.length;
    }
    return 0;
  },

  emjvm_releaseStringUTFChars: function(jniEnv, string, utf) {
  },

  emjvm_getObjectClass__deps: ['$EmJVM'],
  emjvm_getObjectClass: function(env, jobject) {
    if (EmJVM.debug) {
      console.log('EMJVM_GetObjectClass+AddLocalRef: ' + [jobject]);
    }
    var obj = EmJVM.objects[jobject];
    obj.refs++;
    return jobject;
  },

  emjvm_getMethodID: function(jclass, name, sig) {
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
  },

  emjvm_getStaticMethodID: function(jniEnv, jclass, name, sig) {
    // Pretend this to be the same as looking up a non-static method
    return _emjvm_getMethodID(jclass, name, sig);
  },

  emjvm_callObjectMethod: function(jniEnv, jobject, methodId, varargs) {
    if (EmJVM.debug) {
      console.log('EMJVM_CallObjectMethod: ' + [jobject, EmJVM.objects[jobject].name, methodId, EmJVM.objects[jobject]['methodName__' + methodId]]);
    }
    return EmJVM.objects[jobject]['method__' + methodId](varargs);
  },

  emjvm_callStaticObjectMethod: function(jniEnv, jclass, methodId, varargs) {
    // Pretend this to be the same as calling a non-static method
    return _emjvm_callObjectMethod(jniEnv, jclass, methodId, varargs);
  },

  emjvm_callStaticBooleanMethod: function(jniEnv, jclass, methodId, varargs) {
    // Only differs in return type
    return _emjvm_callStaticObjectMethod(jniEnv, jclass, methodId, varargs);
  },

  emjvm_callBooleanMethod: function(jniEnv, jobject, methodId, varargs) {
    // Pretend this to be the same as calling a non-static method
    return _emjvm_callStaticBooleanMethod(jniEnv, jobject, methodId, varargs);
  },

  emjvm_callVoidMethod: function(jniEnv, jobject, methodId, varargs) {
    _emjvm_callObjectMethod(jniEnv, jobject, methodId, varargs);
  },

  emjvm_callIntMethod: function(jniEnv, jobject, methodId, varargs) {
    return _emjvm_callObjectMethod(jniEnv, jobject, methodId, varargs);
  },

  emjvm_deleteLocalRef: function(jniEnv, jobject) {
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
  },

  emjvm_getArrayLength: function(jniEnv, jobject) {
    var obj = EmJVM.objects[jobject];
    assert(obj.name == 'byteArray');
    return obj.value.length;
  },

  emjvm_getByteArrayRegion: function(jniEnv, jobject, start, len, buf) {
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
  },

  emjvm_findClass: function(env, name) {
    name = Pointer_stringify(name);
    if (EmJVM.debug) {
      console.log('emjvm_findClass: ' + [name]);
    }
    var obj = EmJVM.classNames[name];
    assert(obj);
    return obj.id;
  },
});

