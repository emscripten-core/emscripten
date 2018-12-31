extern "C" {
  void lib_function();
  void lib_function2();
}

int cpp_library_function();

int main()
{
  lib_function();
  lib_function2();
  cpp_library_function();
}
