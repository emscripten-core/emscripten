// Copyright 2021 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

// Original Source:
// https://github.com/WebAssembly/binaryen/blob/main/src/support/command-line.h

//
// Command line helpers.
//

#pragma once

#include <functional>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace wasmfs {

class Options {
public:
  using Action = std::function<void(Options*, const std::string&)>;

  enum class Arguments {
    // No arguments.
    Zero,
    // One argument, in the form  --flag A  or  --flag=A
    One,
    // Multiple arguments, in the form --flag A B C
    N,
    // An optional single argument, in the form --flag=A (we disallow --flag A
    // as that would be ambiguous regarding whether A is another flag, or an
    // argument to us).
    Optional
  };

  bool debug;
  std::map<std::string, std::string> extra;

  Options(const std::string& command, const std::string& description);
  ~Options();
  Options& add(const std::string& longName,
               const std::string& shortName,
               const std::string& description,
               Arguments arguments,
               const Action& action,
               bool hidden = false);
  Options& add_positional(const std::string& name,
                          Arguments arguments,
                          const Action& action);
  void parse(int argc, const char* argv[]);

private:
  struct Option {
    std::string longName;
    std::string shortName;
    std::string description;
    Arguments arguments;
    Action action;
    bool hidden;
    size_t seen;
  };
  std::vector<Option> options;
  Arguments positional;
  std::string positionalName;
  Action positionalAction;
};

} // namespace wasmfs
