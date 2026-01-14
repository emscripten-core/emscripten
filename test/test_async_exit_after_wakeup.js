addToLibrary({
  async_func__async: true,
  async_func: (value) => {
    return Asyncify.handleSleep((wakeUp) => {
      // Currently with -sASYNCIFY the wakeUp call needs to be wrapped in
      // callUserCallback, otherwise things like `exit()` won't work from
      // the continuation.
      // TODO(https://github.com/emscripten-core/emscripten/issues/26093)
      setTimeout(() => callUserCallback(() => wakeUp(42)), 0);
    });
  },
});
