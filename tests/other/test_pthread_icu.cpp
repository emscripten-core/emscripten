#include <thread>
#include <iostream>
#include <unicode/unistr.h>

// Test that code using both pthread and icu compiles.
int main () {
	std::thread([] {
		std::string str;
		icu::UnicodeString ustr("Hello world!");
		ustr.toUTF8String(str);
		std::cout << str << std::endl;
	}).detach();
}