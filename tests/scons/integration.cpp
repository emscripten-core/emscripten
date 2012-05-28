
#include <string>
#include <iostream>

int main() {
	std::string output("If you see this - the world is all right!");
	
	auto func = [=](std::string text) -> int {
		std::cout << text << std::endl;
		return 5;
	};
	
	return func(output);
}
