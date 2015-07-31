#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
  // Reading

  FILE *file = fopen("somefile.binary", "rb");
  assert(file);

  fseek(file, 0, SEEK_END);
  int size = ftell(file);
  rewind (file);
  printf("size: %d\n", size);

  char *buffer = (char*) malloc (sizeof(char)*size);
  assert(buffer);

  size_t read = fread(buffer, 1, size, file);
  assert(read == size);

  printf("data: %d", buffer[0]);
  for (int i = 1; i < size; i++)
    printf(",%d", buffer[i]);
  printf("\n");

  fclose (file);
  free (buffer);

  // Do it again, with a loop on feof

  printf("loop: ");
  file = fopen("somefile.binary", "rb");
  assert(file);
  while (!feof(file)) {
    char c = fgetc(file);
    if (c != EOF) printf("%d ", c);
  }
  fclose (file);
  printf("\n");

  // Standard streams

  printf("input:%s\n", gets((char*)malloc(1024)));
  fwrite("texto\n", 1, 6, stdout);
  fwrite("texte\n", 1, 6, stderr);
  putchar('$');
  putc('\n', stdout);

  // Writing

  char data[5] = { 10, 30, 20, 11, 88 };
  FILE *outf = fopen("go.out", "wb");
  fwrite(data, 1, 5, outf);
  fclose(outf);

  FILE *devNull = fopen("/dev/null", "rb");
  assert(devNull);

  char data2[10];
  FILE *inf = fopen("go.out", "rb");
  int num = fread(data2, 1, 10, inf);
  fclose(inf);
  printf("%d : %d,%d,%d,%d,%d\n", num, data2[0], data2[1], data2[2], data2[3], data2[4]);

  // Test reading a file that has not been cached
  
  FILE *other = fopen("test.file", "r");
  assert(other);

  char otherData[1000];
  num = fread(otherData, 1, 9, other);
  otherData[num] = 0;
  printf("other=%s.\n", otherData);

  // Seeking

  fseek(other, 2, SEEK_SET);
  num = fread(otherData, 1, 5, other);
  otherData[num] = 0;
  printf("seeked=%s.\n", otherData);

  fseek(other, -1, SEEK_CUR);
  num = fread(otherData, 1, 3, other);
  otherData[num] = 0;
  printf("seeked=%s.\n", otherData);

  fseek(other, -2, SEEK_END);
  num = fread(otherData, 1, 2, other);
  otherData[num] = 0;
  printf("seeked=%s.\n", otherData);

  fclose(other);

  // fscanf

  outf = fopen("fscan.f", "w");
  fprintf(outf, "10 hello");
  fclose(outf);

  int number;
  char text[100];
  inf = fopen("fscan.f", "r");
  num = fscanf(inf, "%d %s", &number, text);
  fclose(inf);
  printf("fscanfed: %d - %s\n", number, text);

  // temp files
  const char *tname = "file_XXXXXX";
  char tname1[100];
  char tname2[100];
  strcpy(tname1, tname);
  strcpy(tname2, tname);
  assert(!strcmp(tname1, tname2)); // equal
  int f1 = mkstemp(tname1);
  int f2 = mkstemp(tname2);
  assert(f1 != f2);
  //printf("%d,%d,%s,%s\n", f1, f2, tname1, tname2);
  assert(strcmp(tname1, tname2)); // not equal
  assert(fopen(tname1, "r"));
  assert(fopen(tname2, "r"));
  assert(!fopen(tname2+1, "r")); // sanity check that we can't open just anything

  {
    FILE* f = tmpfile();
    assert(f);
    fclose(f);

    char* str = tmpnam(NULL);
    //printf("temp: %s\n", str);
    assert(strncmp("/tmp/", str, 5) == 0);
  }

  FILE *n = fopen("/dev/null", "w");
  printf("5 bytes to dev/null: %d\n", fwrite(data, 1, 5, n));
  fclose(n);

  printf("ok.\n");

  return 0;
}

