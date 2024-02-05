addToLibrary({
  sync_tunnel: function(value) {
    return Asyncify.handleSleep((wakeUp) => {
      setTimeout(function() {
        wakeUp(value + 1);
      }, 1);
    });
  },
  sync_tunnel_bool: function(value) {
    return Asyncify.handleSleep((wakeUp) => {
      setTimeout(function() {
        wakeUp(!value);
      }, 1);
    });
  }
});
