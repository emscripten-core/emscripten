addToLibrary({
  sync_tunnel: (value) => {
    return Asyncify.handleSleep((wakeUp) => {
      setTimeout(() => wakeUp(value + 1), 1);
    });
  },
  sync_tunnel_bool: (value) => {
    return Asyncify.handleSleep((wakeUp) => {
      setTimeout(() => wakeUp(!value), 1);
    });
  },
});
