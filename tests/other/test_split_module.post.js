function saveProfileData() {
  var __write_profile = Module['asm']['__write_profile'];
  if (__write_profile) {
    var len = __write_profile(0, 0);
    var offset = _malloc(len);
    var actualLen = __write_profile(offset, len);
    var profile_data = new Uint8Array(buffer, offset, len);
    nodeFS.writeFileSync('profile.data', profile_data);
    console.log('wrote profile of', actualLen, 'bytes (allocated', len, 'bytes)');
    _free(offset);
  }

  // Say hello *after* recording the profile so that all functions are deferred.
  _say_hello();
}

addOnPostRun(saveProfileData);
