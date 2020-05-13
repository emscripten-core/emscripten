class Foo {
public:
	Foo() : counter(5) {}
	int counter;
};

int testfunc() {
  // libcxxabi will emit some checks for proper static initialization, which
  // can call abort_message()
	static Foo foo;
	return ++foo.counter;
}

int main() {
	testfunc();
}
