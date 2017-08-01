#include <iostream>
#include <string>
#include <memory>

#include <zip.h>
#include <filesystem.h>

#ifndef __cplusplus > 201402L
#error c++ 14 not available
#endif // __cplusplus

using namespace std;

int main() {

    Filesystem::Mount("hello.dat");

    std::shared_ptr<File> file = Filesystem::open("hello.txt");

    std::cout << std::string(&file->Data.get()[0], &file->Data.get()[file->FileSizeUncomp]) << endl;

    return 0;
}
