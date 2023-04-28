// Copyright 2016 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <assert.h>
#include <emscripten.h>
#include <time.h>

EM_JS_DEPS(deps, "$UTF8ToString,emscripten_get_now");

double test(const char *str) {
  double res = EM_ASM_DOUBLE({
    var t0 = _emscripten_get_now();
    var str = UTF8ToString($0);
    var t1 = _emscripten_get_now();
    // out('t: ' + (t1 - t0) + ', len(result): ' + str.length + ', result: ' + str.slice(0, 100));
    return (t1-t0);
  }, str);
  return res;
}

char *utf8_corpus = 0;
long utf8_corpus_length = 0;

char *randomString(int len) {
  if (!utf8_corpus) {
    FILE *handle = fopen("utf8_corpus.txt", "rb");
    fseek(handle, 0, SEEK_END);
    utf8_corpus_length = ftell(handle);
    assert(utf8_corpus_length > 0);
    utf8_corpus = malloc(utf8_corpus_length+1);
    fseek(handle, 0, SEEK_SET);
    fread(utf8_corpus, 1, utf8_corpus_length, handle);
    fclose(handle);
    utf8_corpus[utf8_corpus_length] = '\0';
  }
  int startIdx = rand() % (utf8_corpus_length - len);
  while (((unsigned char)utf8_corpus[startIdx] & 0xC0) == 0x80) {
    ++startIdx;
    if (startIdx + len > utf8_corpus_length) len = utf8_corpus_length - startIdx;
  }
  assert(len > 0);
  char *s = malloc(len+1);
  memcpy(s, utf8_corpus + startIdx, len);
  s[len] = '\0';
  while (len > 0 && ((unsigned char)s[len-1] & 0xC0) == 0x80) { s[--len] = '\0'; }
  while (len > 0 && ((unsigned char)s[len-1] & 0xC0) == 0xC0) { s[--len] = '\0'; }
  assert(len >= 0);
  return s;
}

int main() {
  time_t seed = time(NULL);
  printf("Random seed: %lld\n", seed);
  srand(seed);
  double t = 0;
  double t2 = emscripten_get_now();
  for (int i = 0; i < 100000; ++i) {
    // Create strings of lengths 1-32, because the internals of text decoding
    // have a cutoff of 16 for when to use TextDecoder, and we wish to test both
    // (see UTF8ArrayToString).
    char *str = randomString((i % 32) + 1);
    t += test(str);
    free(str);
  }
  double t3 = emscripten_get_now();
  printf("OK. Time: %f (%f).\n", t, t3-t2);
  return 0;
}
