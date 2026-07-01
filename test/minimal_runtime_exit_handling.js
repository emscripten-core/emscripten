if (typeof process != 'undefined') {
  function onerror(e) {
    // MINIMAL_RUNTIME + EXIT_RUNTIME throws `exit(status)` when exit() is called
    // or when main is done.
    // Unlike the normal runtime it doesn't call process.exit.
    var message = e.message || e;
    var offset = message.indexOf('exit(');
    if (offset != -1) {
      var status = message.substring(offset + 5);
      offset = status.indexOf(')')
      status = status.substr(0, offset)
      process.exitCode = status;
    } else {
      console.error('top-level-onerror:', e)
      process.exitCode = 1;
    }
  }

  process.on('uncaughtException', onerror);
  process.on('unhandledRejection', onerror);
}
