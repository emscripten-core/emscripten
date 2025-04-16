// Copyright 2014 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <string>
#include <typeinfo>
#include <stdexcept>

using namespace std;

class MathError: public std::runtime_error {
    public:
        explicit MathError(const std::string& what) : std::runtime_error(what) { }
};

std::string _latest_err_msg;
int _result;

int cpp_adder(int a, int b)
{
  if (a < b)
    throw MathError ("a cannot be less than b.");

  return a + b;
}

extern "C" {

  const char* latest_err_msg() {
    return _latest_err_msg.c_str();
  }

  int result() { return _result; }

  int c_adder(int a, int b)
  {
/*_1*/  try {
/*_2*/      try {
/*_3*/          try {
                _result = cpp_adder(a, b);
/*+3a*/         } catch (MathError& e) {
                if (string(e.what()).find("not-found-here") == string::npos)
                    throw MathError(string("Special exception caught: ") + typeid(e).name() + ": " + e.what());
                else
                    throw;
/*+3b*/         } catch (exception& e) {
                throw;
            }
/*+2*/      } catch (exception& e) {
            throw;
        }
        return 0;
/*+1*/  } catch (exception& e) {
        _latest_err_msg = string(typeid(e).name()) + ": " + e.what();
        return 1;
    }
  }
}

int main() {
  int rc;

  rc = c_adder(20, 35);
  if (rc == 0)
    printf("2nd call, result = %d\n", result());
  else
    printf("2nd call, sadface: %s\n", latest_err_msg());

  return 0;
}

