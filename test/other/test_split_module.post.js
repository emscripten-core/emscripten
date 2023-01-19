function saveProfileData() {
  var __write_profile = Module['asm']['__write_profile'];
  if (__write_profile) {
    var len = __write_profile(0, 0);
    var offset = _malloc(len);
    var actualLen = __write_profile(offset, len);
    var profile_data = HEAPU8.subarray(offset, offset + len);
    fs.writeFileSync('profile.data', profile_data);
    console.log('profile size is', actualLen, 'bytes (allocated', len, 'bytes)');
    console.log('wrote profile data')
    _free(offset);
  }

  // Say hello *after* recording the profile so that all functions are deferred.
  _say_hello();
}

addOnPostRun(saveProfileData);
