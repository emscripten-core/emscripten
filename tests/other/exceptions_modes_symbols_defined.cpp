int main () {
  try {
    throw 42;
  } catch (int e) {
    return e;
  }
  return 0;
}
