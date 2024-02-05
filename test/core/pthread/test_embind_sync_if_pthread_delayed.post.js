if (!ENVIRONMENT_IS_PTHREAD) {
  console.log(Module['value_from_same_thread']());

  Module['pthreadPoolReady'].then(() => {
    console.log(Module['value_from_other_thread']());
  });
}
