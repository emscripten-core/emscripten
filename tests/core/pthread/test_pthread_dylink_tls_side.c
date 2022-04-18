__thread int side_tls = 42;
__thread int side_tls2 = 43;

int get_side_tls() {
  return side_tls;
}

int get_side_tls2() {
  return side_tls2;
}

int* get_side_tls_address() {
  return &side_tls;
}

int* get_side_tls_address2() {
  return &side_tls2;
}
