extern "C" {
  int js_library_function();
}

int cpp_library_function()
{
	return js_library_function();
}
