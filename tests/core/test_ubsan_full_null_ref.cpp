int main(int argc, char **argv) {
  int *p = nullptr;
  int &a = *p;
  auto &b = *p;
  auto &&c = *p;
}
