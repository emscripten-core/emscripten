// TODO also with >> 1 and >> 3
//      also HEAP*U*, and HEAP8, 16
function shifty($id2) {
  // $id is a non-ssa, $id2 is a param. both should be replaced with a shifted version
  q(HEAP32[$id >> 2]);
  q(HEAP32[($id + 40) >> 2]);
  q(HEAP32[($id + 80 | 0) >> 2]);
  q(HEAP32[(unknown1 + unknown2 + $id) >> 2]);
  q(HEAP32[(unknown1 + $id + unknown2) >> 2]); // unknowns should be shifted together
  var localUnchanged1 = get(1), localUnchanged2 = get(1);
  q(HEAP32[(localUnchanged1 + $id + localUnchanged2) >> 2]); // unknowns should be shifted together
  q($id >> _something_); // non-fixed shift
  $id = q('..');
  q($id << _somethingElse_); // non-fixed shift
  pause(-1);
  q(HEAP32[$id2 >> 1]);
  q(HEAP32[$id2 >> 1]);
  q(HEAP32[$id2 >> 1]);
  q(HEAP32[$id2 >> 1]);
  q(HEAP32[($id2 + 40) >> 1]);
  q(HEAP32[($id2 + 80 | 0) >> 1]);
  var $id3 = get(74);
  q(HEAP32[$id3 >> 3]);
  q(HEAP32[($id3 + 40) >> 3]);
  q(HEAP32[($id3 + 80 | 0) >> 3]);
  q($id3);
  pause(0);
  // similar, but inside another HEAP
  var _idents = get('abc');
  q(HEAP32[(HEAP32[_idents >> 2] + 8 | 0) >> 2]);
  q(HEAP32[(HEAP32[_idents >> 2] + 8 | 0) >> 2]);
  q(HEAP32[(HEAP32[_idents >> 2] + 8 | 0) >> 2]);
  pause(1);
  // $i's shifts should consolidate (the last should be 0..?
  // since we may have had |0 in the middle!)
  var $sn_addr = get(12), $a_addr = get(999);
  var $i = get(112233);
  q(HEAP32[($a_addr + ((($sn_addr - 1 << 1) + 1 | 0) << 2) | 0) >> 2]);
  q(HEAP32[($a_addr + ((($i - 1 << 1) + 1 | 0) << 2) | 0) >> 2]);
  q(HEAP32[($a_addr + (($i << 1 | 0) << 2) | 0) >> 2]);
  q(HEAP32[($a_addr + ($i << 2)) >> 2]);
  q($a_addr >> 2, z($a_addr >> 2));
  pause(2);
  var $level = HEAP[get(322) >> 2]; // ignore this
  var _dwt_norms_real = get('a'), $orient = get('cheez');
  q(HEAP32[(_dwt_norms_real + $orient * 80 + ($level << 3) | 0) >> 2]);
  q(HEAP32[(_dwt_norms_real + $orient * 80 + ($level << 3) + 4 | 0) >> 2]);
  q(HEAP32[(_dwt_norms_real + $orient * 80 + ($level << 3) + 8 | 0) >> 2]);
  pause(3);
  // reuse $a_addr here
  var $wavelet38 = get(38);
  $k = $a_addr;
  q(HEAPF32[(HEAP32[$wavelet38 >> 2] + ($k << 4) + 8 | 0) >> 2]);
  q(HEAPF32[(HEAP32[$wavelet38 >> 2] + ($k << 4) + 12 | 0) >> 2]);
  q(HEAPF32[(HEAP32[$wavelet38 >> 2] + ($k << 4) + 400 | 0) >> 2]);
  pause(4);
  // reuse $k, which already reuses $a_addr
  var $p = $k, $parameters_addr = get('burger')
  q(HEAP32[($parameters_addr + 5624 + ($p << 2) | 0) >> 2]);
  q(HEAP32[($parameters_addr + 5644 + ($p << 2) | 0) >> 2]);
  q(HEAP32[($parameters_addr + 5664 + ($p << 2) | 0) >> 2]);
  pause(5);
  // loops count as more uses!
  var $res_spec242 = get($real), $cp = get('b'), $tileno = arguments[2];
  q(HEAP32[($parameters_addr + 5624 + (($res_spec242 - 1 | 0) << 2) | 0) >> 2]);
  q(HEAP32[(HEAP32[($cp + 108 | 0) >> 2] + $tileno * 5588 + 420 | 0) >> 2]);
  pause(6);
  q($idx << 1 << 2);
  q(1 << $idx << 1); // Do not turn this into the slower 1 << $idx + 1 (which is identical though)
  print(INDENT + "Entering: _main" + "hi"); // this order should not be modified
  pause(7);
  var $tp = get('tp');
  q($tp >> 2);
  q($tp >> 2);
  q($tp >> 2);
  HEAP32[$H400] = $tp;
  HEAP32[$tp] = 5;
  HEAP32[$tp >> 2] = 5;
  HEAP32[HEAP[$tp >> 2]] = 5;
  HEAP32[HEAP[$tp >> 2] >> 2] = 5;
  pause(7);
  q(go() >> 1 << 1);
  q(go() << 1 >> 1);
  q(go() >> 1 >> 1);
  q(go() << 1 << 1);
  q(go() >> 8 << 8);
  q(go() << 8 >> 8);
  q(go() >> 8 >> 8);
  q(go() << 8 << 8);
  q((go() + 2) >> 2); // the 2 >> 2 can't be simplified
}
// EMSCRIPTEN_GENERATED_FUNCTIONS: ["shifty"]
