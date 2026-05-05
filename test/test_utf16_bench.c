// Copyright 2016 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <stdio.h>
#include <assert.h>
#include <emscripten.h>

EM_JS_DEPS(deps, "$UTF16ToString");

double test(const uint16_t *str) {
  double res = EM_ASM_DOUBLE({
    var t0 = _emscripten_get_now();
    var str = UTF16ToString($0);
    var t1 = _emscripten_get_now();
    out('t: ' + (t1 - t0) + ', len(result): ' + str.length + ', result: ' + str.slice(0, 100));
    return (t1-t0);
  }, str);
  return res;
}

uint16_t *utf16_corpus = 0;
long utf16_corpus_length = 0;

uint16_t *randomString(int len) {
  if (!utf16_corpus) {
    FILE *handle = fopen("utf16_corpus.txt", "rb");
    fseek(handle, 0, SEEK_END);
    utf16_corpus_length = ftell(handle)/2;
    assert(utf16_corpus_length > 0);
    utf16_corpus = malloc(sizeof(uint16_t) * (utf16_corpus_length+1));
    fseek(handle, 0, SEEK_SET);
    fread(utf16_corpus, 2, utf16_corpus_length, handle);
    fclose(handle);
    utf16_corpus[utf16_corpus_length] = 0;
  }
  int startIdx = rand() % (utf16_corpus_length - len);
  while((utf16_corpus[startIdx] & 0xFF00) == 0xDC00) {
    ++startIdx;
    if (startIdx + len > utf16_corpus_length) len = utf16_corpus_length - startIdx;
  }
  assert(len > 0);
  uint16_t *s = malloc(sizeof(uint16_t) * (len+1));
  memcpy(s, utf16_corpus + startIdx, len*2);
  s[len] = 0;
  while(((uint16_t)s[len-1] & 0xFF00) == 0xD800) { s[--len] = 0; }
  assert(len >= 0);
  return s;
}

int main() {
  double t = 0;
  double t2 = emscripten_get_now();
  for(int i = 0; i < 10; ++i) {
    // FF Nightly: Already on small strings of 64 bytes in length, TextDecoder trumps in performance.
    uint16_t *str = randomString(100);
    t += test(str);
    free(str);
  }
  double t3 = emscripten_get_now();
  printf("OK. Time: %f (%f).\n", t, t3-t2);
  return 0;
}
