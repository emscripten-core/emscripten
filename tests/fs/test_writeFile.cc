// https://github.com/kripken/emscripten/issues/2334

#include <fstream>
#include <iostream>
#include <string>

#include <emscripten/emscripten.h>

int main()
{
    EM_ASM(
        FS.writeFile("testfile", "a=1\nb=2\nc=3");
    );

    std::ifstream file("testfile");

    while(!file.eof() && !file.fail())
    {
        std::string line;
        getline(file, line);
        std::string name;

        std::cout << "read " << line << std::endl;

        size_t equalsPos = 1;

        size_t notSpace = line.find_first_not_of(" \t", equalsPos);

        if(notSpace != std::string::npos && notSpace != equalsPos)
        {
            line.erase(std::remove_if(line.begin(), line.begin() + notSpace, isspace), line.end());

            equalsPos = line.find('=');
        }

        if(equalsPos == std::string::npos)
            continue;

        name = line.substr(0, equalsPos);
    }

    return 0;
}
