function lin() {
  c(1);
  c(2);
  c(3);
  c(4);
  c(5);
  c(6);
  c(7);
  c(8);
  c(9);
  c(10);
  c(11);
  c(12);
  c(13);
  c(14);
  c(15);
  c(16);
  c(17);
  c(18);
  c(19);
  c(20);
}
function lin2() {
  while (1) {
    c(1);
    c(2);
    c(3);
    c(4);
    c(5);
    c(6);
    c(7);
    c(8);
    c(9);
    c(10);
    c(11);
    c(12);
    c(13);
    c(14);
    c(15);
    c(16);
    c(17);
    c(18);
    c(19);
    c(20);
  }
}
function lin3() {
  while (1) {
    c(1);
    c(2);
    c(3);
    c(4);
    c(5);
    c(6);
    c(7);
    c(8);
    c(9);
    c(10);
    c(11);
    c(12);
    c(13);
    c(14);
    c(15);
    c(16);
    c(17);
    c(18);
    c(19);
    c(20);
    return 10;
  }
  return 20;
}
function lin4() {
  while (1) {
    c(1);
    c(2);
    c(3);
    c(4);
    c(5);
    c(6);
    c(7);
    c(8);
    c(9);
    c(10);
    c(11);
    c(12);
    c(13);
    c(14);
    c(15);
    c(16);
    c(17);
    c(18);
    c(19);
    c(20);
    break;
  }
  return 20;
}
function lin5() {
  while (1) {
    c(1);
    c(2);
    c(3);
    c(4);
    c(5);
    c(6);
    c(7);
    c(8);
    c(9);
    c(10);
    c(11);
    c(12);
    c(13);
    c(14);
    c(15);
    c(16);
    c(17);
    c(18);
    c(19);
    c(20);
    continue;
  }
  return 20;
}
function mix() {
  main: while (1) {
    c(1);
    c(2);
    c(3);
    c(4);
    c(5);
    c(6);
    c(7);
    c(8);
    c(9);
    c(10);
    c(11);
    c(12);
    c(13);
    c(14);
    c(15);
    c(16);
    c(17);
    break main;
    c(18);
    break;
    while (1) {
      break; // no need to forward
    }
    inner: while (1) {
      break inner; // no need to forward
    }
    c(19);
    continue;
    c(20);
    continue main;
  }
  return 20;
}
// EMSCRIPTEN_GENERATED_FUNCTIONS
// EXTRA_INFO: { "sizeToOutline": 30 }
