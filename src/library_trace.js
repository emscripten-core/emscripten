// Copyright 2014 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

var LibraryTracing = {
  $EmscriptenTrace__deps: [
    'emscripten_trace_js_configure', 'emscripten_trace_configure_for_google_wtf',
    'emscripten_trace_js_enter_context', 'emscripten_trace_exit_context',
    'emscripten_trace_js_log_message', 'emscripten_trace_js_mark',
    'emscripten_get_now'
  ],
  $EmscriptenTrace__postset: 'EmscriptenTrace.init()',
  $EmscriptenTrace: {
    worker: null,
    collectorEnabled: false,
    googleWTFEnabled: false,
    testingEnabled: false,

    googleWTFData: {
      'scopeStack': [],
      'cachedScopes': {}
    },

    DATA_VERSION: 1,

    EVENT_ALLOCATE: 'allocate',
    EVENT_ANNOTATE_TYPE: 'annotate-type',
    EVENT_APPLICATION_NAME: 'application-name',
    EVENT_ASSOCIATE_STORAGE_SIZE: 'associate-storage-size',
    EVENT_ENTER_CONTEXT: 'enter-context',
    EVENT_EXIT_CONTEXT: 'exit-context',
    EVENT_FRAME_END: 'frame-end',
    EVENT_FRAME_RATE: 'frame-rate',
    EVENT_FRAME_START: 'frame-start',
    EVENT_FREE: 'free',
    EVENT_LOG_MESSAGE: 'log-message',
    EVENT_MEMORY_LAYOUT: 'memory-layout',
    EVENT_OFF_HEAP: 'off-heap',
    EVENT_REALLOCATE: 'reallocate',
    EVENT_REPORT_ERROR: 'report-error',
    EVENT_SESSION_NAME: 'session-name',
    EVENT_TASK_ASSOCIATE_DATA: 'task-associate-data',
    EVENT_TASK_END: 'task-end',
    EVENT_TASK_RESUME: 'task-resume',
    EVENT_TASK_START: 'task-start',
    EVENT_TASK_SUSPEND: 'task-suspend',
    EVENT_USER_NAME: 'user-name',

    init: function() {
      Module['emscripten_trace_configure'] = _emscripten_trace_js_configure;
      Module['emscripten_trace_configure_for_google_wtf'] = _emscripten_trace_configure_for_google_wtf;
      Module['emscripten_trace_enter_context'] = _emscripten_trace_js_enter_context;
      Module['emscripten_trace_exit_context'] = _emscripten_trace_exit_context;
      Module['emscripten_trace_log_message'] = _emscripten_trace_js_log_message;
      Module['emscripten_trace_mark'] = _emscripten_trace_js_mark;
    },

    // Work around CORS issues ...
    loadWorkerViaXHR: function(url, ready, scope) {
      var req = new XMLHttpRequest();
      req.addEventListener('load', function() {
        var blob = new Blob([this.responseText], { type: 'text/javascript' });
        var worker = new Worker(window.URL.createObjectURL(blob));
        if (ready) {
          ready.call(scope, worker);
        }
      }, req);
      req.open("get", url, false);
      req.send();
    },

    configure: function(collector_url, application) {
      EmscriptenTrace.now = _emscripten_get_now;
      var now = new Date();
      var session_id = now.getTime().toString() + '_' +
                          Math.floor((Math.random() * 100) + 1).toString();
      EmscriptenTrace.loadWorkerViaXHR(collector_url + 'worker.js', function (worker) {
        EmscriptenTrace.worker = worker;
        EmscriptenTrace.worker.addEventListener('error', function (e) {
          console.log('TRACE WORKER ERROR:');
          console.log(e);
        }, false);
        EmscriptenTrace.worker.postMessage({ 'cmd': 'configure',
                                             'data_version': EmscriptenTrace.DATA_VERSION,
                                             'session_id': session_id,
                                             'url': collector_url });
        EmscriptenTrace.configured = true;
        EmscriptenTrace.collectorEnabled = true;
        EmscriptenTrace.postEnabled = true;
      });
      EmscriptenTrace.post([EmscriptenTrace.EVENT_APPLICATION_NAME, application]);
      EmscriptenTrace.post([EmscriptenTrace.EVENT_SESSION_NAME, now.toISOString()]);
    },

    configureForTest: function() {
      EmscriptenTrace.postEnabled = true;
      EmscriptenTrace.testingEnabled = true;
      EmscriptenTrace.now = function() { return 0.0; };
    },

    configureForGoogleWTF: function() {
      if (window && window.wtf) {
        EmscriptenTrace.googleWTFEnabled = true;
      } else {
        console.log('GOOGLE WTF NOT AVAILABLE TO ENABLE');
      }
    },

    post: function(entry) {
      if (EmscriptenTrace.postEnabled && EmscriptenTrace.collectorEnabled) {
        EmscriptenTrace.worker.postMessage({ 'cmd': 'post',
                                             'entry': entry });
      } else if (EmscriptenTrace.postEnabled && EmscriptenTrace.testingEnabled) {
        out('Tracing ' + entry);
      }
    },

    googleWTFEnterScope: function(name) {
      var scopeEvent = EmscriptenTrace.googleWTFData['cachedScopes'][name];
      if (!scopeEvent) {
        scopeEvent = window.wtf.trace.events.createScope(name);
        EmscriptenTrace.googleWTFData['cachedScopes'][name] = scopeEvent;
      }
      var scope = scopeEvent();
      EmscriptenTrace.googleWTFData['scopeStack'].push(scope);
    },

    googleWTFExitScope: function() {
      var scope = EmscriptenTrace.googleWTFData['scopeStack'].pop();
      window.wtf.trace.leaveScope(scope);
    }
  },

  emscripten_trace_js_configure: function(collector_url, application) {
    EmscriptenTrace.configure(collector_url, application);
  },

  emscripten_trace_configure: function(collector_url, application) {
    EmscriptenTrace.configure(Pointer_stringify(collector_url),
                              Pointer_stringify(application));
  },

  emscripten_trace_configure_for_test: function() {
    EmscriptenTrace.configureForTest();
  },

  emscripten_trace_configure_for_google_wtf: function() {
    EmscriptenTrace.configureForGoogleWTF();
  },

  emscripten_trace_set_enabled: function(enabled) {
    EmscriptenTrace.postEnabled = !!enabled;
  },

  emscripten_trace_set_session_username: function(username) {
    EmscriptenTrace.post(EmscriptenTrace.EVENT_USER_NAME, Pointer_stringify(username));
  },

  emscripten_trace_record_frame_start: function() {
    if (EmscriptenTrace.postEnabled) {
      var now = EmscriptenTrace.now();
      EmscriptenTrace.post([EmscriptenTrace.EVENT_FRAME_START, now]);
    }
  },

  emscripten_trace_record_frame_end: function() {
    if (EmscriptenTrace.postEnabled) {
      var now = EmscriptenTrace.now();
      EmscriptenTrace.post([EmscriptenTrace.EVENT_FRAME_END, now]);
    }
  },

  emscripten_trace_js_log_message: function(channel, message) {
    if (EmscriptenTrace.postEnabled) {
      var now = EmscriptenTrace.now();
      EmscriptenTrace.post([EmscriptenTrace.EVENT_LOG_MESSAGE, now,
                            channel, message]);
    }
  },

  emscripten_trace_log_message: function(channel, message) {
    if (EmscriptenTrace.postEnabled) {
      var now = EmscriptenTrace.now();
      EmscriptenTrace.post([EmscriptenTrace.EVENT_LOG_MESSAGE, now,
                            Pointer_stringify(channel),
                            Pointer_stringify(message)]);
    }
  },

  emscripten_trace_js_mark: function(message) {
    if (EmscriptenTrace.postEnabled) {
      var now = EmscriptenTrace.now();
      EmscriptenTrace.post([EmscriptenTrace.EVENT_LOG_MESSAGE, now,
                            "MARK", message]);
    }
    if (EmscriptenTrace.googleWTFEnabled) {
      window.wtf.trace.mark(message);
    }
  },

  emscripten_trace_mark: function(message) {
    if (EmscriptenTrace.postEnabled) {
      var now = EmscriptenTrace.now();
      EmscriptenTrace.post([EmscriptenTrace.EVENT_LOG_MESSAGE, now,
                            "MARK", Pointer_stringify(message)]);
    }
    if (EmscriptenTrace.googleWTFEnabled) {
      window.wtf.trace.mark(Pointer_stringify(message));
    }
  },

  emscripten_trace_report_error: function(error) {
    var now = EmscriptenTrace.now();
    var callstack = (new Error).stack;
    EmscriptenTrace.post([EmscriptenTrace.EVENT_REPORT_ERROR, now,
                          Pointer_stringify(error), callstack]);
  },

  emscripten_trace_record_allocation: function(address, size) {
    if (typeof Module['onMalloc'] === 'function') Module['onMalloc'](address, size);
    if (EmscriptenTrace.postEnabled) {
      var now = EmscriptenTrace.now();
      EmscriptenTrace.post([EmscriptenTrace.EVENT_ALLOCATE,
                            now, address, size]);
    }
  },

  emscripten_trace_record_reallocation: function(old_address, new_address, size) {
    if (typeof Module['onRealloc'] === 'function') Module['onRealloc'](old_address, new_address, size);
    if (EmscriptenTrace.postEnabled) {
      var now = EmscriptenTrace.now();
      EmscriptenTrace.post([EmscriptenTrace.EVENT_REALLOCATE,
                            now, old_address, new_address, size]);
    }
  },

  emscripten_trace_record_free: function(address) {
    if (typeof Module['onFree'] === 'function') Module['onFree'](address);
    if (EmscriptenTrace.postEnabled) {
      var now = EmscriptenTrace.now();
      EmscriptenTrace.post([EmscriptenTrace.EVENT_FREE,
                            now, address]);
    }
  },

  emscripten_trace_annotate_address_type: function(address, type_name) {
    if (EmscriptenTrace.postEnabled) {
      EmscriptenTrace.post([EmscriptenTrace.EVENT_ANNOTATE_TYPE, address,
                            Pointer_stringify(type_name)]);
    }
  },

  emscripten_trace_associate_storage_size: function(address, size) {
    if (EmscriptenTrace.postEnabled) {
      EmscriptenTrace.post([EmscriptenTrace.EVENT_ASSOCIATE_STORAGE_SIZE,
                            address, size]);
    }
  },

  emscripten_trace_report_memory_layout: function() {
    if (EmscriptenTrace.postEnabled) {
      var memory_layout = {
        'static_base':  STATIC_BASE,
        'static_top':   STATICTOP,
        'stack_base':   STACK_BASE,
        'stack_top':    STACKTOP,
        'stack_max':    STACK_MAX,
        'dynamic_base': DYNAMIC_BASE,
        'dynamic_top':  HEAP32[DYNAMICTOP_PTR>>2],
        'total_memory': TOTAL_MEMORY
      };
      var now = EmscriptenTrace.now();
      EmscriptenTrace.post([EmscriptenTrace.EVENT_MEMORY_LAYOUT,
                            now, memory_layout]);
    }
  },

  emscripten_trace_report_off_heap_data: function () {
    function openal_audiodata_size() {
      if (typeof AL == 'undefined' || !AL.currentContext) {
        return 0;
      }
      var totalMemory = 0;
      for (var i in AL.currentContext.buf) {
        var buffer = AL.currentContext.buf[i];
        for (var channel = 0; channel < buffer.numberOfChannels; ++channel) {
          totalMemory += buffer.getChannelData(channel).length * 4;
        }
      }
      return totalMemory;
    }
    if (EmscriptenTrace.postEnabled) {
      var off_heap_data = {
        'openal': openal_audiodata_size()
      }
      var now = EmscriptenTrace.now();
      EmscriptenTrace.post([EmscriptenTrace.EVENT_OFF_HEAP, now, off_heap_data]);
    }
  },

  emscripten_trace_js_enter_context: function(name) {
    if (EmscriptenTrace.postEnabled) {
      var now = EmscriptenTrace.now();
      EmscriptenTrace.post([EmscriptenTrace.EVENT_ENTER_CONTEXT,
                            now, name]);
    }
    if (EmscriptenTrace.googleWTFEnabled) {
      EmscriptenTrace.googleWTFEnterScope(name);
    }
  },

  emscripten_trace_enter_context: function(name) {
    if (EmscriptenTrace.postEnabled) {
      var now = EmscriptenTrace.now();
      EmscriptenTrace.post([EmscriptenTrace.EVENT_ENTER_CONTEXT,
                            now, Pointer_stringify(name)]);
    }
    if (EmscriptenTrace.googleWTFEnabled) {
      EmscriptenTrace.googleWTFEnterScope(Pointer_stringify(name));
    }
  },

  emscripten_trace_exit_context: function() {
    if (EmscriptenTrace.postEnabled) {
      var now = EmscriptenTrace.now();
      EmscriptenTrace.post([EmscriptenTrace.EVENT_EXIT_CONTEXT, now]);
    }
    if (EmscriptenTrace.googleWTFEnabled) {
      EmscriptenTrace.googleWTFExitScope();
    }
  },

  emscripten_trace_task_start: function(task_id, name) {
    if (EmscriptenTrace.postEnabled) {
      var now = EmscriptenTrace.now();
      EmscriptenTrace.post([EmscriptenTrace.EVENT_TASK_START,
                            now, task_id, Pointer_stringify(name)]);
    }
  },

  emscripten_trace_task_associate_data: function(key, value) {
    if (EmscriptenTrace.postEnabled) {
      EmscriptenTrace.post([EmscriptenTrace.EVENT_TASK_ASSOCIATE_DATA,
                            Pointer_stringify(key),
                            Pointer_stringify(value)]);
    }
  },

  emscripten_trace_task_suspend: function(explanation) {
    if (EmscriptenTrace.postEnabled) {
      var now = EmscriptenTrace.now();
      EmscriptenTrace.post([EmscriptenTrace.EVENT_TASK_SUSPEND,
                            now, Pointer_stringify(explanation)]);
    }
  },

  emscripten_trace_task_resume: function(task_id, explanation) {
    if (EmscriptenTrace.postEnabled) {
      var now = EmscriptenTrace.now();
      EmscriptenTrace.post([EmscriptenTrace.EVENT_TASK_RESUME,
                            now, task_id, Pointer_stringify(explanation)]);
    }
  },

  emscripten_trace_task_end: function() {
    if (EmscriptenTrace.postEnabled) {
      var now = EmscriptenTrace.now();
      EmscriptenTrace.post([EmscriptenTrace.EVENT_TASK_END, now]);
    }
  },

  emscripten_trace_close: function() {
    EmscriptenTrace.collectorEnabled = false;
    EmscriptenTrace.googleWTFEnabled = false;
    EmscriptenTrace.postEnabled = false;
    EmscriptenTrace.testingEnabled = false;
    EmscriptenTrace.worker.postMessage({ 'cmd': 'close' });
    EmscriptenTrace.worker = null;
  },
};

autoAddDeps(LibraryTracing, '$EmscriptenTrace');
mergeInto(LibraryManager.library, LibraryTracing);
