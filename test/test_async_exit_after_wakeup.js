addToLibrary({
  async_func__async: 'auto',
  async_func: (value) => new Promise((resolve) => setTimeout(() => resolve(42), 0)),
});
