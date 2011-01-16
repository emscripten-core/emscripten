#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
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

  fwrite("texto\n", 1, 6, stdout);
  fwrite("texte\n", 1, 6, stderr);

  return 0;
}

