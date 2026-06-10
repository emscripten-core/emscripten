#include <GLES3/gl3.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define WIDTH    4
#define HEIGHT   4
#define DEPTH    4
#define CHANNELS 4

static void check_pixel(const GLubyte *buf, int x, int y,
                        GLubyte r, GLubyte g, GLubyte b, GLubyte a,
                        const char *label) {
  int i = (y * WIDTH + x) * CHANNELS;
  if (buf[i] == r && buf[i+1] == g && buf[i+2] == b && buf[i+3] == a) {
    printf("PASS: %s — got rgba(%d,%d,%d,%d)\n", label, r, g, b, a);
  } else {
    printf("FAIL: %s — expected rgba(%d,%d,%d,%d), got rgba(%d,%d,%d,%d)\n",
           label, r, g, b, a, buf[i], buf[i+1], buf[i+2], buf[i+3]);
    assert(0);
  }
}

static void read_slice(GLuint fb, GLuint tex, int z, GLubyte *out) {
  glBindFramebuffer(GL_FRAMEBUFFER, fb);
  glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex, 0, z);
  assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
  glReadPixels(0, 0, WIDTH, HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, out);
  assert(glGetError() == GL_NO_ERROR);
}

int main() {
  EmscriptenWebGLContextAttributes attrs;
  emscripten_webgl_init_context_attributes(&attrs);
  attrs.majorVersion = 2;
  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx = emscripten_webgl_create_context("#canvas", &attrs);
  assert(ctx > 0);
  assert(emscripten_webgl_make_context_current(ctx) == EMSCRIPTEN_RESULT_SUCCESS);
  printf("PASS: WebGL2 context acquired\n");

  /* ---- create 3D texture ---- */
  GLuint tex;
  glGenTextures(1, &tex);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_3D, tex);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

  glTexStorage3D(GL_TEXTURE_3D, 1, GL_RGBA8, WIDTH, HEIGHT, DEPTH);
  assert(glGetError() == GL_NO_ERROR);
  printf("PASS: texStorage3D allocated %dx%dx%d RGBA8\n", WIDTH, HEIGHT, DEPTH);

  /* ---- fill and upload full volume ---- */
  int sliceBytes = WIDTH * HEIGHT * CHANNELS;
  int totalBytes = sliceBytes * DEPTH;
  GLubyte *data = (GLubyte *)malloc(totalBytes);
  assert(data);

  for (int z = 0; z < DEPTH; z++)
    for (int y = 0; y < HEIGHT; y++)
      for (int x = 0; x < WIDTH; x++) {
        int i = (z * HEIGHT * WIDTH + y * WIDTH + x) * CHANNELS;
        data[i + 0] = (z * 64) & 0xff;
        data[i + 1] = (y * 64) & 0xff;
        data[i + 2] = (x * 64) & 0xff;
        data[i + 3] = 255;
      }

  glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0,
                  WIDTH, HEIGHT, DEPTH,
                  GL_RGBA, GL_UNSIGNED_BYTE, data);
  assert(glGetError() == GL_NO_ERROR);
  printf("PASS: texSubImage3D uploaded full volume (%d bytes)\n", totalBytes);

  /* ---- overwrite z-slice 1 with solid red ---- */
  GLubyte *sliceData = (GLubyte *)malloc(sliceBytes);
  assert(sliceData);
  for (int i = 0; i < sliceBytes; i += CHANNELS) {
    sliceData[i] = 255; sliceData[i+1] = 0; sliceData[i+2] = 0; sliceData[i+3] = 255;
  }
  glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 1,
                  WIDTH, HEIGHT, 1,
                  GL_RGBA, GL_UNSIGNED_BYTE, sliceData);
  assert(glGetError() == GL_NO_ERROR);
  printf("PASS: texSubImage3D overwrote z-slice 1 with solid red\n");

  /* ---- patch 2x2 green rect at (1,1) in z-slice 2 ---- */
  GLubyte patchData[2 * 2 * CHANNELS];
  for (int i = 0; i < (int)sizeof(patchData); i += CHANNELS) {
    patchData[i] = 0; patchData[i+1] = 255; patchData[i+2] = 0; patchData[i+3] = 255;
  }
  glTexSubImage3D(GL_TEXTURE_3D, 0, 1, 1, 2,
                  2, 2, 1,
                  GL_RGBA, GL_UNSIGNED_BYTE, patchData);
  assert(glGetError() == GL_NO_ERROR);
  printf("PASS: texSubImage3D patched 2x2 green rect at (1,1) in z-slice 2\n");

  /* ---- readback setup ---- */
  GLuint fb;
  glGenFramebuffers(1, &fb);
  GLubyte readBuf[WIDTH * HEIGHT * CHANNELS];

  /* ---- verify slice 0: original ramp data ---- */
  printf("      --- Slice 0 (original data) ---\n");
  read_slice(fb, tex, 0, readBuf);
  check_pixel(readBuf, 0, 0, 0, 0, 0, 255, "slice 0 pixel (0,0)");
  check_pixel(readBuf, 3, 0, 0, 0, 192, 255, "slice 0 pixel (3,0)");

  /* ---- verify slice 1: solid red ---- */
  printf("      --- Slice 1 (solid red) ---\n");
  read_slice(fb, tex, 1, readBuf);
  for (int y = 0; y < HEIGHT; y++)
    for (int x = 0; x < WIDTH; x++)
      check_pixel(readBuf, x, y, 255, 0, 0, 255, "slice 1 red check");

  /* ---- verify slice 2: green patch + original elsewhere ---- */
  printf("      --- Slice 2 (2x2 green patch at 1,1) ---\n");
  read_slice(fb, tex, 2, readBuf);
  check_pixel(readBuf, 1, 1, 0, 255, 0, 255, "slice 2 patch (1,1)");
  check_pixel(readBuf, 2, 1, 0, 255, 0, 255, "slice 2 patch (2,1)");
  check_pixel(readBuf, 1, 2, 0, 255, 0, 255, "slice 2 patch (1,2)");
  check_pixel(readBuf, 2, 2, 0, 255, 0, 255, "slice 2 patch (2,2)");
  check_pixel(readBuf, 0, 0, 128, 0, 0, 255, "slice 2 corner (0,0) original");

  /* ---- verify slice 3: untouched original ---- */
  printf("      --- Slice 3 (original data) ---\n");
  read_slice(fb, tex, 3, readBuf);
  check_pixel(readBuf, 0, 0, 192, 0, 0, 255, "slice 3 pixel (0,0)");

  /* ---- srcOffset test: upload from middle of a larger buffer ---- */
  printf("      --- srcOffset variant (pointer arithmetic) ---\n");
  GLubyte *bigBuf = (GLubyte *)malloc(sliceBytes * 2);
  assert(bigBuf);
  memset(bigBuf, 0, sliceBytes * 2);
  for (int i = sliceBytes; i < sliceBytes * 2; i += CHANNELS) {
    bigBuf[i] = 0; bigBuf[i+1] = 0; bigBuf[i+2] = 255; bigBuf[i+3] = 255;
  }
  glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 3,
                  WIDTH, HEIGHT, 1,
                  GL_RGBA, GL_UNSIGNED_BYTE, bigBuf + sliceBytes);
  assert(glGetError() == GL_NO_ERROR);
  read_slice(fb, tex, 3, readBuf);
  for (int y = 0; y < HEIGHT; y++)
    for (int x = 0; x < WIDTH; x++)
      check_pixel(readBuf, x, y, 0, 0, 255, 255, "slice 3 blue (srcOffset)");
  printf("PASS: srcOffset variant — slice 3 is now solid blue\n");

  /* ---- unpack alignment test ---- */
  printf("      --- Unpack alignment ---\n");
  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
  glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, 0);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  GLubyte tightData[2 * 2 * CHANNELS];
  for (int i = 0; i < (int)sizeof(tightData); i += CHANNELS) {
    tightData[i] = 255; tightData[i+1] = 255; tightData[i+2] = 0; tightData[i+3] = 255;
  }
  glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0,
                  2, 2, 1,
                  GL_RGBA, GL_UNSIGNED_BYTE, tightData);
  assert(glGetError() == GL_NO_ERROR);
  read_slice(fb, tex, 0, readBuf);
  check_pixel(readBuf, 0, 0, 255, 255, 0, 255, "alignment: (0,0) yellow");
  {
    int i = (3 * WIDTH + 3) * CHANNELS;
    int isYellow = (readBuf[i] == 255 && readBuf[i+1] == 255 &&
                    readBuf[i+2] == 0 && readBuf[i+3] == 255);
    assert(!isYellow);
    printf("PASS: alignment: (3,3) not overwritten — rgba(%d,%d,%d,%d)\n",
           readBuf[i], readBuf[i+1], readBuf[i+2], readBuf[i+3]);
  }

  /* ---- cleanup ---- */
  glDeleteFramebuffers(1, &fb);
  glDeleteTextures(1, &tex);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
  free(data);
  free(sliceData);
  free(bigBuf);

  printf("      --- done ---\n");
  return 0;
}