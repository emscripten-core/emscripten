function save_profile_data() {
  var __write_profile = wasmExports['__write_profile'];
  if (__write_profile) {
    var len = __write_profile({{{ to64('0') }}}, 0);
    var offset = _malloc(len);
    var actualLen = __write_profile({{{ to64('offset') }}}, len);
    var profile_data = HEAPU8.subarray(offset, offset + len);
    if (typeof writeFile !== 'undefined') {
      writeFile('profile.data', profile_data);
    } else if (typeof fs !== 'undefined') {
      fs.writeFileSync('profile.data', profile_data);
    } else {
      console.log(JSON.stringify(Array.from(profile_data)));
    }
    console.log('wrote profile data')
    _free(offset);
  }
}
