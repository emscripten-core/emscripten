function makePrintProfile(profileName) {
  function printProfile() {
    console.log('in printProfile (' + profileName + ') on thread', _pthread_self());
    var __write_profile = Module['asm']['__write_profile'];
    if (__write_profile) {
      if (typeof nodeFS == 'undefined') {
        var nodeFS = require('fs');
      }
      var len = __write_profile(0, 0);
      var offset = _malloc(len);
      var actualLen = __write_profile(offset, len);
      var profile_data = new Uint8Array(buffer, offset, len);
      nodeFS.writeFileSync(profileName, profile_data);
      console.log('wrote profile of', actualLen, 'bytes (allocated', len, 'bytes)');
      _free(offset);
    }
  }
  return printProfile;
}

Module['afterThreadExit'] = makePrintProfile('thread.profile');

Module['loadSplitModule'] = function(deferred, imports, prop) {
  console.log('Loading deferred module');
  console.log(new Error().stack);
  return instantiateSync(deferred, imports);
}
