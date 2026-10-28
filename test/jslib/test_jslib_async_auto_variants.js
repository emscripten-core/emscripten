addToLibrary({
  answer__sig: 'pip',
  answer__async: {{{ FORMS }}},
  answer__proxy: 'sync',
  answer: (ms, value, sync) => {
    if (!ms) return value;
    if (sync) return -1;
    return new Promise((resolve, reject) =>
      setTimeout(() => value ? resolve(value) : reject(new Error('zero')), ms));
  },
  wait_ms__sig: 'vi',
  wait_ms__async: {{{ FORMS }}},
  wait_ms__proxy: 'sync',
  wait_ms: (ms, sync) => {
    if (ms && !sync) return new Promise((resolve) => setTimeout(resolve, ms));
  },
});
