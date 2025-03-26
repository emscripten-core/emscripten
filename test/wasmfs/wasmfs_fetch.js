addToLibrary({
  getUrlOrigin__sig: 'vpp',
  getUrlOrigin: function (ptr, len) {
    try {
      var orig = self.location.origin;
      var nb = lengthBytesUTF8(orig) + 1;
      if (nb <= len)
        stringToUTF8(orig, ptr, len);
    } catch (e) {
      console.warn(e);
    }
  }
});
