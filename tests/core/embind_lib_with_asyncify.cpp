#include <assert.h>

#include <string>

#include <emscripten.h>
#include <emscripten/bind.h>

using namespace emscripten;

EM_JS(int, sleep_and_return, (int x), {
	return Asyncify.handleSleep(wakeUp => setTimeout(wakeUp, 10, x));
});

void delayed_throw() {
	sleep_and_return(0);
	EM_ASM({ throw new Error('my message'); });
}

int foo() {
	return sleep_and_return(10);
}

class Bar {
public:
	Bar(): x(sleep_and_return(20)) {}

	int getX() const {
		return x;
	}

	int method() {
		return sleep_and_return(30);
	}

	int method_without_async_calls() {
		return 40;
	}

	void void_method_without_async_calls() {}

	void void_method_with_async_calls() {
		sleep_and_return(0);
	}

	static int static_method() {
		return sleep_and_return(50);
	}

private:
	int x;
};

EMSCRIPTEN_BINDINGS(embind_async) {
	function("delayed_throw", &delayed_throw);
	function("foo", &foo);

	class_<Bar>("Bar")
		.constructor<>()
		.function("method", &Bar::method)
		.function("method_without_async_calls", &Bar::method_without_async_calls)
		.function("void_method_without_async_calls", &Bar::void_method_without_async_calls)
		.function("void_method_with_async_calls", &Bar::void_method_with_async_calls)
		.class_function("static_method", &Bar::static_method)
		.property("x", &Bar::getX)
		;
}
