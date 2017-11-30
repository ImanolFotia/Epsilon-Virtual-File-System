#include <iostream>
#include <string>
#include <memory>

#include <filesystem.h>

/*
using namespace std;

int main() {

    Filesystem::Mount("secondfile.rar");
    Filesystem::Mount("hello.dat");

    std::shared_ptr<File> file = Filesystem::open("hello.txt");

    char* bytes = new char[file->FileSizeUncomp];

    file->seekg(0);
    file->read(bytes, file->FileSizeUncomp);

    std::cout << std::string(bytes, file->FileSizeUncomp) << std::endl;

    Filesystem::close(file);

    return 0;
}
*/
