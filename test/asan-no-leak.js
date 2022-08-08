if (Module != undefined) {
  Module.ASAN_OPTIONS = 'detect_leaks=0';
} else {
  Module = {ASAN_OPTIONS: 'detect_leaks=0'};
}
