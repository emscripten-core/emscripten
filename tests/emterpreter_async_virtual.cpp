#include <stdio.h>
#include <emscripten.h>

class BatchFile {
public:
  virtual bool ReadLine(char * line);
};

bool BatchFile::ReadLine(char * line) {
  printf("Sleep-->\n");
  emscripten_sleep(300);
  printf("<--Sleep\n");
  return true;
}

BatchFile b; 

BatchFile *bf = &b;

int main(void) {
  printf("main.\n");
  int cnt = 0;
  while (cnt < 5) {
    printf("main loop %i\n", ++cnt);
    bf->ReadLine(0);
  }
  int result = 1;
  REPORT_RESULT();
}

