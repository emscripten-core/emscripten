#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <emscripten/emscripten.h>

EMSCRIPTEN_KEEPALIVE int mywrite1() {
  FILE* out = fopen("/working/hoge.txt","w");
  fprintf(out,"test1\n");
  fclose(out);
  return 0;
}

EMSCRIPTEN_KEEPALIVE int myread1() {
  FILE* in = fopen("/working/hoge.txt","r");
  char buf[1024];
  int len;
  if(in==NULL)
    printf("open failed\n");

  while(! feof(in)){
    if(fgets(buf,sizeof(buf),in)==buf){
      printf("%s",buf);
    }
  }
  fclose(in);
  return 0;
}

EMSCRIPTEN_KEEPALIVE int mywrite2() {
  FILE* out = fopen("/working2/hoge.txt","w");
  fprintf(out,"test2\n");
  fclose(out);
  return 0;
}

EMSCRIPTEN_KEEPALIVE int myread2() {
  FILE* in = fopen("/working2/hoge.txt","r");
  char buf[1024];
  int len;
  if(in==NULL)
    printf("open failed\n");

  while(! feof(in)){
    if(fgets(buf,sizeof(buf),in)==buf){
      printf("%s",buf);
    }
  }
  fclose(in);
  return 0;
}

EMSCRIPTEN_KEEPALIVE int mywrite0(int i) {
  FILE* out = fopen("hoge.txt","w");
  fprintf(out,"test0_%d\n",i);
  fclose(out);
  return 0;
}

EMSCRIPTEN_KEEPALIVE int myread0() {
  FILE* in = fopen("hoge.txt","r");
  char buf[1024];
  int len;
  if(in==NULL)
    printf("open failed\n");

  while(! feof(in)){
    if(fgets(buf,sizeof(buf),in)==buf){
      printf("%s",buf);
    }
  }
  fclose(in);
  return 0;
}

EMSCRIPTEN_KEEPALIVE int myaccess0existing() {
  int canAccess = access("/working/hoge.txt",O_RDONLY);
  printf("access=%d\n", canAccess);
  return 0;
}

EMSCRIPTEN_KEEPALIVE int myaccess0absent() {
  int canAccess = access("/working/nosuchfile",O_RDONLY);
  printf("access=%d\n", canAccess);
  return 0;
}

EMSCRIPTEN_KEEPALIVE int myaccess1existing() {
  int canAccess = access("/hoge.txt",O_RDONLY);
  printf("access=%d\n", canAccess);
  return 0;
}

EMSCRIPTEN_KEEPALIVE int myaccess1absent() {
  int canAccess = access("/nosuchfile",O_RDONLY);
  printf("access=%d\n", canAccess);
  return 0;
}

EMSCRIPTEN_KEEPALIVE int myreade() {
  FILE* in = fopen("proxyfs_embed.txt","r");
  char buf[1024];
  int len;
  if(in==NULL)
    printf("open failed\n");

  while(! feof(in)){
    if(fgets(buf,sizeof(buf),in)==buf){
      printf("%s",buf);
    }
  }
  fclose(in);
  return 0;
}

EMSCRIPTEN_KEEPALIVE int myreadSeekEnd() {
  FILE* in = fopen("/working2/hoge.txt","r");

  fseek(in, 0L, SEEK_END);
  int fileSize = ftell(in);
  fseek(in, 0L, SEEK_SET);
  printf("%d\n", fileSize);

  fclose(in);
  return 0;
}
