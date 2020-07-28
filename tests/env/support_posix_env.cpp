#include <string>
#include <iostream>
#include <sstream>

void foo()
{
	std::stringstream ss;
}

int bar()
{
	return 13;
}

int main()
{
	if (bar() == 14) foo();
}
