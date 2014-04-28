int add (int x, int y) {
	return x+y;
}

void concat (const char* str1, const char* str2, char *result) {
  int i=0, j=0;
  while (str1[i] != '\0') {
    result[i] = str1[i];
    i++;
  }
  while (str2[j] != '\0') {
    result[i+j] = str2[j];
    j++;
  }
  result[i+j] = '\0';
}
