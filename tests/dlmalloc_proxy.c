// Emscripten tests

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <dlfcn.h>

typedef void *(*mallocer)(int n);
typedef void (*freeer)(void *p);

void *lib_handle;
int handles = 0;
mallocer mallocproxy = NULL;
freeer freeproxy = NULL;

void get_lib() {
  //printf("get lib\n");
  lib_handle = dlopen("liblib.so", RTLD_NOW);
  assert(lib_handle != NULL);
  handles++;

  mallocproxy = (mallocer)dlsym(lib_handle, "mallocproxy");
  assert(mallocproxy!= NULL);
  freeproxy = (freeer)dlsym(lib_handle, "freeproxy");
  assert(freeproxy!= NULL);
}

void unget_lib() {
  //printf("unget lib\n");
  assert(lib_handle);
  dlclose(lib_handle);
  handles--;
  if (handles == 0) lib_handle = NULL;
}

int main() {
  int n = 0, total = 0, l = 0;
  void *allocs[50];
  allocs[10] = malloc(10); // pull in real malloc
  for (int i = 0; i < 1000; i++) {
    //printf("%d: total ever %d MB, current MB %d, total libs %d\n", i, total, n, l);
    if (i % 5 == 0) {
      if (handles < 10) {
        get_lib();
        l++;
      }
    }
    if (i % 7 == 0) {
      if (handles > 0) unget_lib();
    }
    if (i % 3 == 0) {
      if (handles > 0) {
        if (n < 10) {
          if (i % 2 == 0) {
            //printf("alloc\n");
            allocs[n++] = mallocproxy(1024*1024);
          } else {
            //printf("real alloc\n");
            allocs[n++] = malloc(1024*1024);
          }
          total++;
        } else {
          //printf("real free\n");
          free(allocs[--n]); // real free
        }
      }
    }
    if (i % 4 == 0) {
      if (handles > 0 && n > 0) {
        //printf("free\n");
        if (i % 2 == 0) {
          //printf("free\n");
          freeproxy(allocs[--n]);
        } else {
          //printf("real free\n");
          free(allocs[--n]);
        }
      }
    }
  }
  while (n > 0) free(allocs[--n]); // real free
  while (handles > 0) unget_lib();
  printf("*%d,%d*\n", total, l);
}

