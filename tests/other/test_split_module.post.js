function saveProfileData() {
  // Avoid offset 0 so that we don't upset the stack cookie monster.
  var offset = 1024;
  var __write_profile = Module['asm']['__write_profile'];
  if (__write_profile) {
    var len = Module['asm']['__write_profile'](offset, -1);
    var profile_data = new Uint8Array(buffer, offset, len);
    nodeFS.writeFileSync('profile.data', profile_data);
  }
}

addOnPostRun(saveProfileData);
