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
      std::ios_base::failure failz("<input failure>");
      std::cout
        << "Throwing std::ios_base::failure: |" << failz.what() << "|..."
        << std::endl;
      throw failz;
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
    // Show that setting the input stream to throw on failure does not
    // preserve the exception type.
    std::istringstream iss("Three");
    TestEnum::type value = TestEnum::Zero;

    // Tell the stream to throw on failure.
    iss.exceptions(std::ios_base::failbit);

    // We expect this to fail.
    iss >> value;
    if (iss.fail()) {
      std::cout
        << "No exception thrown; Failed to convert 'Three' to "
        "TestEnum::type... fail" << std::endl;
    }
    else {
      std::cout
        << "Successfully converted 'Three' to TestEnum::type: " << value
        << "... fail" << std::endl;
    }
  }
  catch(const std::ios_base::failure& ex) {
    // This is what we expect to catch.
    std::cout
      << "Caught std::ios_base::failure: |" << ex.what() << "|... ok"
      << std::endl;
  }
  catch(const std::exception& ex) {
    std::cout << "Caught exception: " << ex.what() << "... fail" << std::endl;
  }
  catch (...) {
    // This is what is actually caught.
    std::cout
      << "Unknown exception caught converting 'Three' to TestEnum... fail"
      << std::endl;
  }

  return 0;
}

