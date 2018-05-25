
// Extra libc helper functions

char *tzname[2];

void* _get_tzname() {
  return (void*)tzname;
}

int daylight;

int* _get_daylight() {
  return &daylight;
}

long timezone;

long* _get_timezone() {
  return &timezone;
}

