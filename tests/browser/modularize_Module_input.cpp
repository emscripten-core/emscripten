int main() {
  EM_ASM({
    if (!Module.someProperty) {
      alert("Must see a property on Module, that arrived from outside");
      abort();
    }
  });
  REPORT_RESULT(0);
  return 0;
}

