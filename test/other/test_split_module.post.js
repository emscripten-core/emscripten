#preprocess

function saveProfileData() {
  var __write_profile = wasmExports['__write_profile'];
  if (__write_profile) {
    var len = __write_profile({{{ to64('0') }}}, 0);
    var offset = _malloc(len);
    var actualLen = __write_profile({{{ to64('offset') }}}, len);
    var profile_data = HEAPU8.subarray(offset, offset + len);
    if (typeof writeFile !== 'undefined') {
      console.log('using writeFile')
      writeFile('profile.data', profile_data);
    } else if (typeof fs !== 'undefined') {
      console.log('using fs.writeFileSync')
      fs.writeFileSync('profile.data', profile_data);
    } else {
      console.log(JSON.stringify(Array.from(profile_data)));
    }
    console.log('profile size is', actualLen, 'bytes (allocated', len, 'bytes)');
    console.log('wrote profile data')
    _free(offset);
  }

  // Say hello *after* recording the profile so that all functions are deferred.
  var result = _say_hello();
  if (typeof Asyncify !== 'undefined') {
    console.log((result instanceof Promise) ? 'result is promise' : '');
  }
}

addOnPostRun(saveProfileData);
