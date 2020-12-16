function saveProfileData() {
  var __write_profile = Module['asm']['__write_profile'];
  if (__write_profile) {
    var capacity = 1;
    var offset;
    var len;
    while (true) {
      offset = _malloc(capacity);
      len = __write_profile(offset, capacity);
      if (len <= capacity) {
        console.log('writing profile of size', len, 'with capacity', capacity);
        break;
      }
      _free(offset);
      capacity *= 2;
    }
    var profile_data = new Uint8Array(buffer, offset, len);
    nodeFS.writeFileSync('profile.data', profile_data);
  }

  // Say hello *after* recording the profile so that all functions are deferred.
  _say_hello();
}

addOnPostRun(saveProfileData);
