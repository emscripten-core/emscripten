#include <stdio.h>
#include <emscripten.h>

class BatchFile {
public:
  virtual int ReadLine(char * line);
};

int BatchFile::ReadLine(char * line) {
  printf("Sleep-->\n");
  emscripten_sleep(300);
  printf("<--Sleep\n");
  return 1;
}

BatchFile b; 

BatchFile *bf = &b;

int main(void) {
  printf("main.\n");
  int cnt = 0;
  int result = 0;
  while (cnt < 5) {
    printf("main loop %i\n", ++cnt);
    result += bf->ReadLine(0);
  }
  REPORT_RESULT();
}

