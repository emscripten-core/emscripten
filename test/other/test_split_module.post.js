#preprocess
#include "save_profile_data.js"

function saveProfileData() {
  save_profile_data();

  // Say hello *after* recording the profile so that all functions are deferred.
  var result = _say_hello();
  if (typeof Asyncify !== 'undefined') {
    console.log((result instanceof Promise) ? 'result is promise' : '');
  }
}

addOnPostRun(saveProfileData);
