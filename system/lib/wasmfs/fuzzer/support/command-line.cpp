// Copyright 2021 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

// Original Source:
// https://github.com/WebAssembly/binaryen/blob/main/src/support/command-line.cpp

#include "command-line.h"
#include <assert.h>
#include <iostream>

using namespace wasmfs;

#ifndef PROJECT_VERSION
#define PROJECT_VERSION 1
#endif

#ifndef SCREEN_WIDTH
#define SCREEN_WIDTH 80
#endif

void printWrap(std::ostream& os, int leftPad, const std::string& content) {
  int len = content.size();
  int space = SCREEN_WIDTH - leftPad;
  std::string nextWord;
  std::string pad(leftPad, ' ');
  for (int i = 0; i <= len; ++i) {
    if (i != len && content[i] != ' ' && content[i] != '\n') {
      nextWord += content[i];
    } else {
      if (static_cast<int>(nextWord.size()) > space) {
        os << '\n' << pad;
        space = SCREEN_WIDTH - leftPad;
      }
      os << nextWord;
      space -= nextWord.size() + 1;
      if (space > 0) {
        os << ' ';
      }
      nextWord.clear();
      if (content[i] == '\n') {
        os << '\n';
        space = SCREEN_WIDTH - leftPad;
      }
    }
  }
}

Options::Options(const std::string& command, const std::string& description)
  : debug(false), positional(Arguments::Zero) {
  add("--version",
      "",
      "Output version information and exit",
      Arguments::Zero,
      [command](Options*, const std::string&) {
        std::cout << command << " version " << PROJECT_VERSION << "\n";
        exit(0);
      });
  add("--help",
      "-h",
      "Show this help message and exit",
      Arguments::Zero,
      [this, command, description](Options* o, const std::string&) {
        std::cout << command;
        if (positional != Arguments::Zero) {
          std::cout << ' ' << positionalName;
        }
        std::cout << "\n\n";
        printWrap(std::cout, 0, description);
        std::cout << "\n\nOptions:\n";
        size_t optionWidth = 0;
        for (const auto& o : options) {
          if (o.hidden) {
            continue;
          }
          optionWidth =
            std::max(optionWidth, o.longName.size() + o.shortName.size());
        }
        for (const auto& o : options) {
          if (o.hidden) {
            continue;
          }
          std::cout << '\n';
          bool long_n_short = o.longName.size() != 0 && o.shortName.size() != 0;
          size_t pad = 1 + optionWidth - o.longName.size() - o.shortName.size();
          std::cout << "  " << o.longName << (long_n_short ? ',' : ' ')
                    << o.shortName << std::string(pad, ' ');
          printWrap(std::cout, optionWidth + 4, o.description);
          std::cout << '\n';
        }
        std::cout << '\n';
        exit(EXIT_SUCCESS);
      });
  add("--debug",
      "-d",
      "Print debug information to stderr",
      Arguments::Optional,
      [&](Options* o, const std::string& arguments) {
        debug = true;
        // setDebugEnabled(arguments.c_str());
      });
}

Options::~Options() {}

Options& Options::add(const std::string& longName,
                      const std::string& shortName,
                      const std::string& description,
                      Arguments arguments,
                      const Action& action,
                      bool hidden) {
  options.push_back(
    {longName, shortName, description, arguments, action, hidden, 0});
  return *this;
}

Options& Options::add_positional(const std::string& name,
                                 Arguments arguments,
                                 const Action& action) {
  positional = arguments;
  positionalName = name;
  positionalAction = action;
  return *this;
}

void Options::parse(int argc, const char* argv[]) {
  assert(argc > 0 && "expect at least program name as an argument");
  size_t positionalsSeen = 0;
  auto dashes = [](const std::string& s) {
    for (size_t i = 0; i < s.size(); ++i) {
      if (s[i] != '-') {
        return i;
      }
    }
    return s.size();
  };
  for (size_t i = 1, e = argc; i != e; ++i) {
    std::string currentOption = argv[i];

    // "-" alone is a positional option
    if (dashes(currentOption) == 0 || currentOption == "-") {
      // Positional.
      switch (positional) {
        case Arguments::Zero:
          // Optional arguments must use --flag=A format, and not separated by
          // spaces (which would be ambiguous).
        case Arguments::Optional:
          std::cerr << "Unexpected positional argument '" << currentOption
                    << "'\n";
          exit(EXIT_FAILURE);
        case Arguments::One:
          if (positionalsSeen) {
            std::cerr << "Unexpected second positional argument '"
                      << currentOption << "' for " << positionalName << '\n';
            exit(EXIT_FAILURE);
          }
          [[fallthrough]];
        case Arguments::N:
          positionalAction(this, currentOption);
          ++positionalsSeen;
          break;
      }
      continue;
    }

    // Non-positional.
    std::string argument;
    auto equal = currentOption.find_first_of('=');
    if (equal != std::string::npos) {
      argument = currentOption.substr(equal + 1);
      currentOption = currentOption.substr(0, equal);
    }
    Option* option = nullptr;
    for (auto& o : options) {
      if (o.longName == currentOption || o.shortName == currentOption) {
        option = &o;
      }
    }
    if (!option) {
      std::cerr << "Unknown option '" << currentOption << "'\n";
      exit(EXIT_FAILURE);
    }
    switch (option->arguments) {
      case Arguments::Zero:
        if (argument.size()) {
          std::cerr << "Unexpected argument '" << argument << "' for option '"
                    << currentOption << "'\n";
          exit(EXIT_FAILURE);
        }
        break;
      case Arguments::One:
        if (option->seen) {
          std::cerr << "Unexpected second argument '" << argument << "' for '"
                    << currentOption << "'\n";
          exit(EXIT_FAILURE);
        }
        [[fallthrough]];
      case Arguments::N:
        if (!argument.size()) {
          if (i + 1 == e) {
            std::cerr << "Couldn't find expected argument for '"
                      << currentOption << "'\n";
            exit(EXIT_FAILURE);
          }
          argument = argv[++i];
        }
        break;
      case Arguments::Optional:
        break;
    }
    option->action(this, argument);
    ++option->seen;
  }
}
