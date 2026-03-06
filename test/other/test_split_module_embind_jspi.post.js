#preprocess
#include "save_profile_data.js"

async function saveProfileData() {
  console.log('primary_function: ' + Module.primary_function());
  save_profile_data();
  // deferred_function *after* recording the profile so that all functions are deferred.
  var result = Module.deferred_function();
  console.log('deferred_function: ' + result);
  console.log('deferred_function await: ' + await result);
}

addOnPostRun(saveProfileData);
