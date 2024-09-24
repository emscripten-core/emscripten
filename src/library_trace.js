/**
 * @license
 * Copyright 2014 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

var LibraryTracing = {
  $EmscriptenTrace__deps: [
    '$traceConfigure', 'emscripten_trace_configure_for_google_wtf',
    '$traceEnterContext', 'emscripten_trace_exit_context',
    '$traceLogMessage', '$traceMark',
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

    init: () => {
      Module['emscripten_trace_configure'] = traceConfigure;
      Module['emscripten_trace_configure_for_google_wtf'] = _emscripten_trace_configure_for_google_wtf;
      Module['emscripten_trace_enter_context'] = traceEnterContext;
      Module['emscripten_trace_exit_context'] = _emscripten_trace_exit_context;
      Module['emscripten_trace_log_message'] = traceLogMessage;
      Module['emscripten_trace_mark'] = traceMark;
    },

    // Work around CORS issues ...
    fetchBlob: (url) => {
      return fetch(url).then((rsp) => rsp.blob());
    },

    configure: (collector_url, application) => {
      EmscriptenTrace.now = _emscripten_get_now;
      var now = new Date();
      var session_id = now.getTime().toString() + '_' +
                          Math.floor((Math.random() * 100) + 1).toString();
      EmscriptenTrace.fetchBlob(collector_url + 'worker.js').then((blob) => {
        EmscriptenTrace.worker = new Worker(window.URL.createObjectURL(blob));
        EmscriptenTrace.worker.addEventListener('error', (e) => {
          out('TRACE WORKER ERROR:');
          out(e);
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

    configureForTest: () => {
      EmscriptenTrace.postEnabled = true;
      EmscriptenTrace.testingEnabled = true;
      EmscriptenTrace.now = () => 0.0;
    },

    configureForGoogleWTF: () => {
      if (window && window['wtf']) {
        EmscriptenTrace.googleWTFEnabled = true;
      } else {
        out('GOOGLE WTF NOT AVAILABLE TO ENABLE');
      }
    },

    post: (entry) => {
      if (EmscriptenTrace.postEnabled && EmscriptenTrace.collectorEnabled) {
        EmscriptenTrace.worker.postMessage({ 'cmd': 'post',
                                             'entry': entry });
      } else if (EmscriptenTrace.postEnabled && EmscriptenTrace.testingEnabled) {
        out('Tracing ' + entry);
      }
    },

    googleWTFEnterScope: (name) => {
      var scopeEvent = EmscriptenTrace.googleWTFData['cachedScopes'][name];
      if (!scopeEvent) {
        scopeEvent = window['wtf'].trace.events.createScope(name);
        EmscriptenTrace.googleWTFData['cachedScopes'][name] = scopeEvent;
      }
      var scope = scopeEvent();
      EmscriptenTrace.googleWTFData['scopeStack'].push(scope);
    },

    googleWTFExitScope: () => {
      var scope = EmscriptenTrace.googleWTFData['scopeStack'].pop();
      window['wtf'].trace.leaveScope(scope);
    }
  },

  $traceConfigure: (collector_url, application) => {
    EmscriptenTrace.configure(collector_url, application);
  },

  emscripten_trace_configure: (collector_url, application) => {
    EmscriptenTrace.configure(UTF8ToString(collector_url),
                              UTF8ToString(application));
  },

  emscripten_trace_configure_for_test: () => {
    EmscriptenTrace.configureForTest();
  },

  emscripten_trace_configure_for_google_wtf: () => {
    EmscriptenTrace.configureForGoogleWTF();
  },

  emscripten_trace_set_enabled: (enabled) => {
    EmscriptenTrace.postEnabled = !!enabled;
  },

  emscripten_trace_set_session_username: (username) => {
    EmscriptenTrace.post(EmscriptenTrace.EVENT_USER_NAME, UTF8ToString(username));
  },

  emscripten_trace_record_frame_start: () => {
    if (EmscriptenTrace.postEnabled) {
      var now = EmscriptenTrace.now();
      EmscriptenTrace.post([EmscriptenTrace.EVENT_FRAME_START, now]);
    }
  },

  emscripten_trace_record_frame_end: () => {
    if (EmscriptenTrace.postEnabled) {
      var now = EmscriptenTrace.now();
      EmscriptenTrace.post([EmscriptenTrace.EVENT_FRAME_END, now]);
    }
  },

  $traceLogMessage: (channel, message) => {
    if (EmscriptenTrace.postEnabled) {
      var now = EmscriptenTrace.now();
      EmscriptenTrace.post([EmscriptenTrace.EVENT_LOG_MESSAGE, now,
                            channel, message]);
    }
  },

  emscripten_trace_log_message: (channel, message) => {
    if (EmscriptenTrace.postEnabled) {
      var now = EmscriptenTrace.now();
      EmscriptenTrace.post([EmscriptenTrace.EVENT_LOG_MESSAGE, now,
                            UTF8ToString(channel),
                            UTF8ToString(message)]);
    }
  },

  $traceMark: (message) => {
    if (EmscriptenTrace.postEnabled) {
      var now = EmscriptenTrace.now();
      EmscriptenTrace.post([EmscriptenTrace.EVENT_LOG_MESSAGE, now,
                            "MARK", message]);
    }
    if (EmscriptenTrace.googleWTFEnabled) {
      window['wtf'].trace.mark(message);
    }
  },

  emscripten_trace_mark: (message) => {
    if (EmscriptenTrace.postEnabled) {
      var now = EmscriptenTrace.now();
      EmscriptenTrace.post([EmscriptenTrace.EVENT_LOG_MESSAGE, now,
                            "MARK", UTF8ToString(message)]);
    }
    if (EmscriptenTrace.googleWTFEnabled) {
      window['wtf'].trace.mark(UTF8ToString(message));
    }
  },

  emscripten_trace_report_error: (error) => {
    var now = EmscriptenTrace.now();
    var callstack = (new Error).stack;
    EmscriptenTrace.post([EmscriptenTrace.EVENT_REPORT_ERROR, now,
                          UTF8ToString(error), callstack]);
  },

  emscripten_trace_record_allocation: (address, size) => {
    if (typeof Module['onMalloc'] == 'function') Module['onMalloc'](address, size);
    if (EmscriptenTrace.postEnabled) {
      var now = EmscriptenTrace.now();
      EmscriptenTrace.post([EmscriptenTrace.EVENT_ALLOCATE,
                            now, address, size]);
    }
  },

  emscripten_trace_record_reallocation: (old_address, new_address, size) => {
    if (typeof Module['onRealloc'] == 'function') Module['onRealloc'](old_address, new_address, size);
    if (EmscriptenTrace.postEnabled) {
      var now = EmscriptenTrace.now();
      EmscriptenTrace.post([EmscriptenTrace.EVENT_REALLOCATE,
                            now, old_address, new_address, size]);
    }
  },

  emscripten_trace_record_free: (address) => {
    if (typeof Module['onFree'] == 'function') Module['onFree'](address);
    if (EmscriptenTrace.postEnabled) {
      var now = EmscriptenTrace.now();
      EmscriptenTrace.post([EmscriptenTrace.EVENT_FREE,
                            now, address]);
    }
  },

  emscripten_trace_annotate_address_type: (address, type_name) => {
    if (EmscriptenTrace.postEnabled) {
      EmscriptenTrace.post([EmscriptenTrace.EVENT_ANNOTATE_TYPE, address,
                            UTF8ToString(type_name)]);
    }
  },

  emscripten_trace_associate_storage_size: (address, size) => {
    if (EmscriptenTrace.postEnabled) {
      EmscriptenTrace.post([EmscriptenTrace.EVENT_ASSOCIATE_STORAGE_SIZE,
                            address, size]);
    }
  },

  emscripten_trace_report_memory_layout: () => {
    if (EmscriptenTrace.postEnabled) {
      var memory_layout = {
        'static_base':  {{{ GLOBAL_BASE }}},
        'stack_base':   _emscripten_stack_get_base(),
        'stack_top':    _emscripten_stack_get_current(),
        'stack_max':    _emscripten_stack_get_end(),
        'dynamic_top':  _sbrk(0),
        'total_memory': HEAP8.length
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

  $traceEnterContext: (name) => {
    if (EmscriptenTrace.postEnabled) {
      var now = EmscriptenTrace.now();
      EmscriptenTrace.post([EmscriptenTrace.EVENT_ENTER_CONTEXT,
                            now, name]);
    }
    if (EmscriptenTrace.googleWTFEnabled) {
      EmscriptenTrace.googleWTFEnterScope(name);
    }
  },

  emscripten_trace_enter_context: (name) => {
    if (EmscriptenTrace.postEnabled) {
      var now = EmscriptenTrace.now();
      EmscriptenTrace.post([EmscriptenTrace.EVENT_ENTER_CONTEXT,
                            now, UTF8ToString(name)]);
    }
    if (EmscriptenTrace.googleWTFEnabled) {
      EmscriptenTrace.googleWTFEnterScope(UTF8ToString(name));
    }
  },

  emscripten_trace_exit_context: () => {
    if (EmscriptenTrace.postEnabled) {
      var now = EmscriptenTrace.now();
      EmscriptenTrace.post([EmscriptenTrace.EVENT_EXIT_CONTEXT, now]);
    }
    if (EmscriptenTrace.googleWTFEnabled) {
      EmscriptenTrace.googleWTFExitScope();
    }
  },

  emscripten_trace_task_start: (task_id, name) => {
    if (EmscriptenTrace.postEnabled) {
      var now = EmscriptenTrace.now();
      EmscriptenTrace.post([EmscriptenTrace.EVENT_TASK_START,
                            now, task_id, UTF8ToString(name)]);
    }
  },

  emscripten_trace_task_associate_data: (key, value) => {
    if (EmscriptenTrace.postEnabled) {
      EmscriptenTrace.post([EmscriptenTrace.EVENT_TASK_ASSOCIATE_DATA,
                            UTF8ToString(key),
                            UTF8ToString(value)]);
    }
  },

  emscripten_trace_task_suspend: (explanation) => {
    if (EmscriptenTrace.postEnabled) {
      var now = EmscriptenTrace.now();
      EmscriptenTrace.post([EmscriptenTrace.EVENT_TASK_SUSPEND,
                            now, UTF8ToString(explanation)]);
    }
  },

  emscripten_trace_task_resume: (task_id, explanation) => {
    if (EmscriptenTrace.postEnabled) {
      var now = EmscriptenTrace.now();
      EmscriptenTrace.post([EmscriptenTrace.EVENT_TASK_RESUME,
                            now, task_id, UTF8ToString(explanation)]);
    }
  },

  emscripten_trace_task_end: () => {
    if (EmscriptenTrace.postEnabled) {
      var now = EmscriptenTrace.now();
      EmscriptenTrace.post([EmscriptenTrace.EVENT_TASK_END, now]);
    }
  },

  emscripten_trace_close: () => {
    EmscriptenTrace.collectorEnabled = false;
    EmscriptenTrace.googleWTFEnabled = false;
    EmscriptenTrace.postEnabled = false;
    EmscriptenTrace.testingEnabled = false;
    EmscriptenTrace.worker.postMessage({ 'cmd': 'close' });
    EmscriptenTrace.worker = null;
  },
};

autoAddDeps(LibraryTracing, '$EmscriptenTrace');
addToLibrary(LibraryTracing);
