if (!ENVIRONMENT_IS_PTHREAD) {
  Module.print = (text) => { console.log(text); }
}
