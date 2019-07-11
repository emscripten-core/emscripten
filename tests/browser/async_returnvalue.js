mergeInto(LibraryManager.library, {
  sync_tunnel: function(value) {
    return Bysyncify.handleSleep(function(wakeUp) {
      setTimeout(function() {
        wakeUp(value + 1);
      }, 1);
    });
  }
});

