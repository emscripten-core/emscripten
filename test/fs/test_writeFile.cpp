// https://github.com/emscripten-core/emscripten/issues/2334

#include <fstream>
#include <iostream>
#include <string>

#include <emscripten/emscripten.h>

int main() {
  EM_ASM(
    const buf = Uint8Array.from('c=3\nd=4\ne=5', x => x.charCodeAt(0));
    FS.writeFile("testfile", "a=1\nb=2\n");
    FS.writeFile("testfile", buf.subarray(4, 7) /* d=4 */, { flags: "a" });
  );

  std::ifstream file("testfile");

  while (!file.eof() && !file.fail()) {
    std::string line;
    getline(file, line);
    std::string key;
    std::string val;

    std::cout << "read " << line << std::endl;

    size_t equalsPos = 1;

    size_t notSpace = line.find_first_not_of(" \t", equalsPos);

    if (notSpace != std::string::npos && notSpace != equalsPos) {
      line.erase(std::remove_if(line.begin(), line.begin() + notSpace, isspace), line.end());

      equalsPos = line.find('=');
    }

    if (equalsPos == std::string::npos) {
        continue;
    }

    key = line.substr(0, equalsPos);
    val = line.substr(equalsPos + 1);

    std::cout << "parsed " << key << "=" << val << std::endl;
  }

  return 0;
}
