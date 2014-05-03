#define SIZE 50
char result[SIZE];

int add (int x, int y) {
	return x+y;
}

char* concat (const char* str1, const char* str2) {
  int i, j;
  for (i = 0; str1[i] != '\0'; i++) result[i] = str1[i];
  for (j = 0; str2[j] != '\0' && i+j<SIZE; j++) result[i+j] = str2[j];
  result[i+j] = '\0';
  return result;
}
