#pragma once
#include <memory>
#include <string>

class File{
public:
    File(){}
    ~File(){}

    void PrintFileInfo(){}

public:
    uint32_t FileSizeUncomp;
    uint32_t FileSizeComp;
    uint32_t FileHeaderIndex;
    std::string FileName;
    std::shared_ptr<unsigned char> Data;

};
