static __thread int mytls = 42;

int get_side_tls() {
  return mytls;
}

int* get_side_tls_address() {
  return &mytls;
}
