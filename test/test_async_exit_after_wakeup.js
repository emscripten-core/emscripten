addToLibrary({
  async_func__async: true,
  async_func: (value) => {
    return Asyncify.handleSleep((wakeUp) => {
      setTimeout(() => wakeUp(42), 0);
    });
  },
});
