unsigned char problematic[] = { 0x20, 0x7c, 0x02, 0x07, 0x5f, 0xa0, 0xdf };
int main() {
  unsigned char a, b;
  int result = 0, i, j;
  for (i = 0; i < sizeof(problematic); ++i) {
    a = problematic[i] ^ 32;
    for (j = 0; j < sizeof(problematic); ++j) {
      b = problematic[j] ^ 32;
      if (((const unsigned char)data[a][2*b]) != a ||
          ((const unsigned char)data[a][2*b + 1]) != b) {
        result = 1;
        printf("data[0x%02x][0x%03x]=%x02x\n", a, 2*b, data[a][2*b]);
        printf("data[0x%02x][0x%03x]=%x02x\n", a, 2*b + 1, data[a][2*b + 1]);
      }
    }
  }
  REPORT_RESULT()
}
