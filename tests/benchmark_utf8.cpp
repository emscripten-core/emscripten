// Copyright 2016 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <iostream>
#include <cassert>
#include <emscripten.h>

double test(const char *str) {
  double res = EM_ASM_DOUBLE({
    var t0 = _emscripten_get_now();
    var str = Module.UTF8ToString($0);
    var t1 = _emscripten_get_now();
//    out('t: ' + (t1 - t0) + ', len(result): ' + str.length + ', result: ' + str.slice(0, 100));
    return (t1-t0);
  }, str);
  return res;
}

char *utf8_corpus = 0;
long utf8_corpus_length = 0;

char *randomString(int len) {
  if (!utf8_corpus) {
//    FILE *handle = fopen("ascii_corpus.txt", "rb");
    FILE *handle = fopen("utf8_corpus.txt", "rb");
    fseek(handle, 0, SEEK_END);
    utf8_corpus_length = ftell(handle);
    assert(utf8_corpus_length > 0);
    utf8_corpus = new char[utf8_corpus_length+1];
    fseek(handle, 0, SEEK_SET);
    fread(utf8_corpus, 1, utf8_corpus_length, handle);
    fclose(handle);
    utf8_corpus[utf8_corpus_length] = '\0';
  }
  int startIdx = rand() % (utf8_corpus_length - len);
  while(((unsigned char)utf8_corpus[startIdx] & 0xC0) == 0x80) {
    ++startIdx;
    if (startIdx + len > utf8_corpus_length) len = utf8_corpus_length - startIdx;
  }
  assert(len > 0);
  char *s = new char[len+1];
  memcpy(s, utf8_corpus + startIdx, len);
  s[len] = '\0';
  while(((unsigned char)s[len-1] & 0xC0) == 0x80) { s[--len] = '\0'; }
  while(((unsigned char)s[len-1] & 0xC0) == 0xC0) { s[--len] = '\0'; }
  assert(len >= 0);
  return s;
}

int main() {
  srand(time(NULL));
  double t = 0;
  double t2 = emscripten_get_now();
  for(int i = 0; i < 100000; ++i) {
    // FF Nightly: Already on small strings of 64 bytes in length, TextDecoder trumps in performance.
    char *str = randomString(8);
    t += test(str);
    delete [] str;
  }
  double t3 = emscripten_get_now();
  printf("OK. Time: %f (%f).\n", t, t3-t2);

#ifdef REPORT_RESULT
  REPORT_RESULT(0);
#endif
}
