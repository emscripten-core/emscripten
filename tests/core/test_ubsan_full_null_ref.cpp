int main(void) {
  int *p = nullptr;
  int &a = *p;
  auto &b = *p;
  auto &&c = *p;
}
