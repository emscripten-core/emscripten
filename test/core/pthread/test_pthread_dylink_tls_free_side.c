_Thread_local char side_tls[4096];

int touch_side_tls(int value) {
  side_tls[0] = value;
  return side_tls[0];
}
