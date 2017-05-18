#include <iostream>
#include <zlib.h>
#include <cstdio>
#include <string>
#include <bitset>
#include <iomanip>
#include <map>
#include <vector>
#include <algorithm>
#include <zip.h>
#include <memory>
#include <fstream>

#ifndef __cplusplus > 201402L
#error c++ 14 not availble
#endif // __cplusplus

using namespace std;

std::shared_ptr<LocalFileHeader<uint16_t, uint32_t> > getLocalHeader(std::shared_ptr<std::ifstream> infile, uint32_t curroffset) {

    std::shared_ptr<LocalFileHeader<uint16_t, uint32_t> > lfh =
    (std::shared_ptr<LocalFileHeader<uint16_t, uint32_t> >)
    new LocalFileHeader<uint16_t, uint32_t> ();

    infile->seekg(curroffset, infile->beg);
    infile->read(reinterpret_cast<char*>(lfh.get()), sizeof(char)* 30);

    std::cout << "Signature: " << std::hex << lfh->signature << std::dec << endl; /// 0x04034b50
    std::cout << "Minimum version required: " << lfh->minversion << endl;
    std::cout << "General flag: " << std::bitset<16>(lfh->flag) << endl;
    std::cout << "\t" << "Encripted: " << std::bitset<16>(lfh->flag)[0] << endl;
    std::cout << "\t" << "Sliding dictinary used: " << (std::bitset<16>(lfh->flag)[1] == 0 ? "4K" : "8K") << endl;
    std::cout << "\t" << "Shannon-Fano trees: " << (std::bitset<16>(lfh->flag)[2] == 0 ? "2" : "3") << endl;
    std::cout << "\t" << "Strong encryption: " << std::bitset<16>(lfh->flag)[6] << endl << endl;
    std::cout << "Compression Method: " << Compression_Method[lfh->compmethod] << endl;
    std::cout << "Last Modified Time: " << lfh->lastmodtime << endl;
    std::cout << "CRC-32: " << std::hex << (lfh->CRC32) << std::dec << endl;
    std::cout << "Compressed Size: " <<lfh->compsize << endl;
    std::cout << "Uncompressed Size: " <<lfh->uncompsize << endl;
    std::cout << "Filename Length: " << lfh->filenamelength << endl;
    std::cout << "Extra field Length: " <<lfh->extrafieldlength << endl;

    std::shared_ptr<char> buffer = (std::shared_ptr<char>) new char[lfh->filenamelength];
    infile->read(reinterpret_cast<char*>(buffer.get()), sizeof(char)* lfh->filenamelength);

    cout << "Filename:" << endl;

    for(int i=0; i < lfh->filenamelength; i++)
        cout << (char)buffer.get()[i];

    cout << endl << endl;

    buffer.reset();

    buffer = (std::shared_ptr<char>) new char[lfh->uncompsize];
    infile->read(reinterpret_cast<char*>(buffer.get()), sizeof(char) * lfh->uncompsize);
    cout << "Content: " << endl;
    for(int i=0; i < lfh->uncompsize; i++)
        cout << (char)buffer.get()[i];
    cout << endl;

    return lfh;
}

int main() {

    int END = 0;

    std::shared_ptr<std::ifstream> infile = (std::shared_ptr<std::ifstream>) new std::ifstream("hello.zip", std::ifstream::binary);
    if (!infile) return -1;

    std::vector<std::shared_ptr<LocalFileHeader<uint16_t, uint32_t> > > headers;
    size_t offset = 0;

    Zip zip("hello.zip");

    do {

        uint32_t buffer = 0;
        infile->seekg(offset, infile->beg);
        infile->read((char*)&buffer, sizeof(uint32_t));

        if((uint32_t)buffer == 0x04034b50) {
            headers.push_back(getLocalHeader(infile, offset));
            offset +=    headers.back()->filenamelength  +
                         headers.back()->compsize        +
                         headers.back()->extrafieldlength+
                         30;
            cout << "Offset: " << offset << endl;
        } else if((uint32_t)buffer == 0x02014b50) {
            ///Central directory stuff
            cout << "Central Directory Header" << endl;
            std::shared_ptr<CentralDirectoryHeader<uint16_t, uint32_t> > cdh = (std::shared_ptr<CentralDirectoryHeader<uint16_t, uint32_t> >) new CentralDirectoryHeader<uint16_t, uint32_t> ();

            infile->seekg(offset, infile->beg);
            infile->read(reinterpret_cast<char*>(cdh.get()), sizeof(char) * 46);
            offset += cdh->extrafieldlength + cdh->filecommentlength + cdh->filenamelength + 46;
            cout << "Offset: " << offset << endl;
        } else if((uint32_t)buffer == 0x06054b50) {
            ///end stuff
            cout << "End header" << endl;
            END = 1;
        }
        else
        {
            cout << "Not a Zip file" << endl;
            return -1;
        }


    } while(END == 0);

    infile->close();
    return 0;
}
