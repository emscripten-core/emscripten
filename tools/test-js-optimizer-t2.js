// TODO also with >> 1 and >> 3
//      also HEAP*U*, and HEAP8, 16
function shifty($id) {
  // $id is a param, $id2 is a local. both should be replaced with a shifted version
  q(HEAP32[$id >> 2]);
  q(HEAP32[($id + 40) >> 2]);
  q(HEAP32[($id + 80 | 0) >> 2]);
  q(HEAP32[(unknown1 + unknown2 + $id) >> 2]);
  q(HEAP32[(unknown1 + $id + unknown2) >> 2]); // unknowns should be shifted together
  var localUnchanged1 = get(1), localUnchanged2 = get(1);
  q(HEAP32[(localUnchanged1 + $id + localUnchanged2) >> 2]); // unknowns should be shifted together
  q($id >> _something_); // non-fixed shift
  q($id << _somethingElse_); // non-fixed shift
  pause(-1);
  var $id2;
  $id2 = get(54);
  q(HEAP32[$id2 >> 1]);
  q(HEAP32[($id2 + 40) >> 1]);
  q(HEAP32[($id2 + 80 | 0) >> 1]);
}
// EMSCRIPTEN_GENERATED_FUNCTIONS: ["shifty"]
