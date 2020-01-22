mergeInto(LibraryManager.library, {
  sync_tunnel: function(value) {
    return Asyncify.handleSleep(function(wakeUp) {
      setTimeout(function() {
        wakeUp(value + 1);
      }, 1);
    });
  }
});

