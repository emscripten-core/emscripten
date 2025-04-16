// just do it, until we have full pthreads
void __synccall(void (*func)(void *), void *ctx) {
  func(ctx);
}

