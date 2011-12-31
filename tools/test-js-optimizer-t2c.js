function shifty() {
  $pPage = HEAP32[$pCur_addr + 116 + ($26 << 16 >> 16 << 2) >> 2];
  var $ead_192394 = HEAP32[$pCur_addr + 116 + ($26 << 16 >> 16 << 2) >> 2];
  $pPage2 = HEAP32[$pCur_addr + 116 + ($26 << 16 >> 16 << 2)];
  var $ead_192394b = HEAP32[$pCur_addr + 116 + ($26 << 16 >> 16 << 2)];
  $pPage2 = HEAP32[$pCur_addr + 116 + ($26 << 16 >> 16)];
  var $ead_192394b = HEAP32[$pCur_addr + 116 + ($26 << 16 >> 16)];
  // We prefer to do additions then shifts, so the shift happens last, because the shift output is known to be 32-bit. So these should not change
  q(16 >> 2);
  q($13 + 8 >> 2);
  q(28 + $13 >> 2);
  q(48 + $13 + 12 >> 2);
  q($13 + $15 + 12 >> 2);
  q(HEAPF32[$output + ($j37 << 4) + 4 >> 2]);
  q(5 + $13 + 8 << 2);
  q(((h() | 0) >> 2) << 2); // removing the shifts is dangerous
}
// EMSCRIPTEN_GENERATED_FUNCTIONS: ["shifty"]
