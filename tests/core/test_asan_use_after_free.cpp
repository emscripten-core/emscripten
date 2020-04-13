int main() {
  int *x = new int[10];
  delete [] x;
  return x[5];
}
