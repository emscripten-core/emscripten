function shifty($id) {
  $id >>= 2;
  q(HEAP32[$id]);
  q(HEAP32[$id + 10]);
  q(HEAP32[$id + 20]);
  q(HEAP32[(unknown2 + unknown1 >> 2) + $id]);
  q(HEAP32[(unknown2 + unknown1 >> 2) + $id]);
  var localUnchanged1 = get(1), localUnchanged2 = get(1);
  q(HEAP32[(localUnchanged2 + localUnchanged1 >> 2) + $id]);
  q($id >> _something_ - 2);
  q($id << _somethingElse_ + 2);
  pause(-1);
  var $id2;
  $id2 = get(54) >> 1;
  q(HEAP32[$id2]);
  q(HEAP32[$id2 + 20]);
  q(HEAP32[$id2 + 40]);
}
// EMSCRIPTEN_GENERATED_FUNCTIONS: ["shifty"]
