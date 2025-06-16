addToLibrary({
  getUrlOrigin__sig: 'vpp',
  getUrlOrigin: function (ptr, len) {
    try {
      stringToUTF8(location.origin, ptr, len);
    } catch (e) {
      console.warn(e);
    }
  }
});
