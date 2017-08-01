#pragma once
#include <memory>
#include <string>
#include <fstream>
class File {
public:
    File() {
        m_isopen = true;
    }
    ~File() {}

    void PrintFileInfo() {}

public:
    uint32_t FileSizeUncomp;
    uint32_t FileSizeComp;
    uint32_t FileHeaderIndex;
    std::string FileName;
    std::shared_ptr<unsigned char> Data;
    std::ifstream realfile;
    uint32_t m_charposition;
    bool m_isopen;
    bool m_isvirtualfile;

    template <typename T>
    int seekg(T pos) {
        return 0;
    }

    template <typename T>
    int seekg(T off, std::ios_base::seekdir dir) {
        return 0;
    }

    int read(...) {}
    int open() {}
    int is_open(){
        return m_isopen;
    }

    int close() {
        m_isopen = false;
    }

    bool eof() {
        if(m_charposition >= FileSizeUncomp)
            return true;
        else
            return false;
    }

};
