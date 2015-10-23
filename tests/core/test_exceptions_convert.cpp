#include <ios>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace
{
  struct TestEnum
  {
    enum type
    {
      Zero,
      One
    };
  };

  // An input operator a-la-boost date_time.  This input operator will catch
  // anything and rethrow if the exception mask for the input stream is set to
  // throw on failure.
  std::istream& operator>>(std::istream& in, TestEnum::type& value)
  {
    try {
      std::string raw;
      if (not (in >> raw)) { return in; }
      if (raw == "Zero")  { value = TestEnum::Zero; return in; }
      if (raw == "One")   { value = TestEnum::One; return in; }

      // The boost input operator uses it's own facet for input which can
      // throw, so we simulate something failing by just throwing an exception
      // directly.
      throw std::exception();
    }
    catch (...) {
      const std::ios_base::iostate exception_mask = in.exceptions();
      if (std::ios_base::failbit & exception_mask) {
        try { in.setstate(std::ios_base::failbit); }
        catch(std::ios_base::failure&) {}
        throw; // rethrow original exception
      }
      else {
        in.setstate(std::ios_base::failbit);
      }
    }
    return in;
  }
}

int main()
{
  try {
    // Show that the input operator works.
    std::istringstream iss("One");
    TestEnum::type value = TestEnum::Zero;

    // We expect this to work.
    iss >> value;
    if (iss.fail()) {
      std::cout
        << "Failed to convert 'One' to TestEnum::type... fail"
        << std::endl;
    }
    else {
      std::cout
        << "Successfully converted 'One' to TestEnum::type: " << value
        << "... ok" << std::endl;
    }
  }
  catch (...) {
    std::cout
      << "Unknown exception caught converting 'One' to TestEnum... fail"
      << std::endl;
  }

  try {
    // Show that invalid input set the fail bit on the input stream and no
    // exception is thrown, since we did not enable them on the stream.
    std::istringstream iss("Two");
    TestEnum::type value = TestEnum::Zero;

    // We expect this to fail.
    iss >> value;
    if (iss.fail()) {
      std::cout
        << "Failed to convert 'Two' to TestEnum::type... ok"
        << std::endl;
    }
    else {
      std::cout
        << "Successfully converted 'Two' to TestEnum::type: " << value
        << "... fail" << std::endl;
    }
  }
  catch (...) {
    std::cout
      << "Unknown exception caught converting 'Two' to TestEnum... fail"
      << std::endl;
  }

  try {
    // Show that setting the input stream to throw on failure currently
    // results in a JS exception being emitted.
    std::istringstream iss("Three");
    TestEnum::type value = TestEnum::Zero;

    // Tell the stream to throw on failure.
    iss.exceptions(std::ios_base::failbit);

    // We expect this to fail.
    iss >> value;
    if (iss.fail()) {
      std::cout
        << "No exception thrown; Failed to convert 'Three' to TestEnum::type..."
        "fail" << std::endl;
    }
    else {
      std::cout
        << "Successfully converted 'Three' to TestEnum::type: " << value
        << "... fail" << std::endl;
    }
  }
  catch(const std::ios_base::failure& ex) {
    std::cout << "Caught exception: " << ex.what() << "... ok" << std::endl;
  }
  catch (...) {
    std::cout
      << "Unknown exception caught converting 'Three' to TestEnum... fail"
      << std::endl;
  }

  return 0;
}

