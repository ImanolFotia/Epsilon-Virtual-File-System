#include <iostream>
#include <string>
#include <memory>

#include <zip.h>

#ifndef __cplusplus > 201402L
#error c++ 14 not available
#endif // __cplusplus

using namespace std;

int main() {

    Zip zip("hello.zip");

    std::shared_ptr<File> file = zip.getFileByName("hello.txt");

    std::cout << std::string(&file->Data.get()[0], &file->Data.get()[file->FileSizeUncomp]) << endl;

    return 0;
}
