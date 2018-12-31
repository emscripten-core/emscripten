#include <random>
#include <iostream>
#include <exception>

auto main()
  -> int
try
{
  std::random_device rd;
  std::cout << "random was read" << "\n";
}
catch( const std::exception& e )
{
  std::cerr << e.what();
}

