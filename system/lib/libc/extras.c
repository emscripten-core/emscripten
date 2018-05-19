
// Extra libc helper functions

char *tzname[2];

void* _get_tzname() {
  return (void*)tzname;
}

