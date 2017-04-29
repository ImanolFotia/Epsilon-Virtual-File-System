#include <iostream>
#include <zlib.h>
#include <cstdio>
#include <string>
#include <bitset>
#include <iomanip>
#include <map>
#include <vector>
using namespace std;

#pragma pack(push, 1)
struct LocalFileHeader {
    /** offset 0*/  uint32_t        signature; /// 0x04034b50

    /** offset 4*/  uint16_t        minversion;
    /** offset 6*/  uint16_t        flag;

    /** offset 8*/  uint16_t        compmethod;
    /** offset 10*/ uint32_t        lastmodtime;

    /** offset 14*/ uint32_t        CRC32;

    /** offset 20*/ uint32_t        compsize;

    /** offset 24*/ uint32_t        uncompsize;

    /** offset 28*/ uint16_t        filenamelength;
    /** offset 30*/ uint16_t        extrafieldlength;
};

struct DataDescriptor {
    uint32_t     signature; /// 0x08074b50
    uint32_t     CRC32;
    uint32_t     compsize;
    uint32_t     uncompsize;
};

struct CentralDirectoryHeader {
    /** offset 0*/ uint32_t     signature; ///0x02014b50

    /** offset 4*/ uint16_t   version;
    /** offset 6*/ uint16_t   minversion;

    /** offset 8*/ uint16_t   flag;
    /** offset 10*/ uint16_t   commethod;

    /** offset 12*/ uint16_t   lastmodtimel;
    /** offset 14*/ uint16_t   lastmoddate;

    /** offset 16*/ uint32_t     CRC32;

    /** offset 20*/ uint32_t     compsize;

    /** offset 24*/ uint32_t     uncomsize;

    /** offset 28*/ uint16_t   filenamelength;
    /** offset 30*/ uint16_t   extrafieldlength;

    /** offset 32*/ uint16_t   filecommentlength;
    /** offset 34*/ uint16_t   filestartsdisk;

    /** offset 36*/ uint16_t   intatt;

    /** offset 38*/ uint32_t     extatt;

    /** offset 42*/ uint32_t     offsettolocalheader;
    /*
    char*   filename;
    char*   extrafield;
    char*   comment;
    */
};

struct EndOfCentralDirectory {
    uint32_t     signature; ///0x06054b50
    uint16_t   disknum;
    uint16_t   diskcendirstarts;
    uint16_t   numcdr;
    uint16_t   totalcdr;
    uint32_t     cdsize;
    uint32_t     offsetcd;
    uint16_t   commentlength;
    //char*   comment;
};

std::map<int, string> Compression_Method;

LocalFileHeader* getLocalHeader(gzFile infile, int curroffset) {
    LocalFileHeader* lfh = new LocalFileHeader;
    gzseek(infile, curroffset, SEEK_SET);
    gzread(infile, lfh, sizeof(char) * 30);

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

    char* buffer = new char[lfh->filenamelength];
    gzread(infile, buffer, sizeof(char) * lfh->filenamelength);

    cout << "Filename:" << endl;
    for(int i=0; i < lfh->filenamelength; i++)
        cout << (char)buffer[i];
    cout << endl << endl;

    delete buffer;
    buffer = new char[lfh->uncompsize];
    gzread(infile, buffer, sizeof(char) * lfh->uncompsize);
    cout << "Content: " << endl;
    for(int i=0; i < lfh->uncompsize; i++)
        cout << (char)buffer[i];
    cout << endl;

    return lfh;
}


int main() {

    Compression_Method[0] = "Stored";
    Compression_Method[1] = "Shrunk";
    Compression_Method[2] = "Reduced with compression factor 1";
    Compression_Method[3] = "Reduced with compression factor 2";
    Compression_Method[4] = "Reduced with compression factor 3";
    Compression_Method[5] = "Reduced with compression factor 4";
    Compression_Method[6] = "Imploded";
    Compression_Method[7] = "Tokenized";
    Compression_Method[8] = "Deflated";
    Compression_Method[9] = "Deflate64";
    Compression_Method[10] = "IBM TERSE (old)";
    Compression_Method[11] = "PKWARE";
    Compression_Method[12] = "BZIP2";
    Compression_Method[13] = "PKWARE";
    Compression_Method[14] = "LZMA (EFS)";
    Compression_Method[15] = "PKWARE";
    Compression_Method[16] = "PKWARE";
    Compression_Method[17] = "PKWARE";
    Compression_Method[18] = "IBM TERSE (new)";
    Compression_Method[19] = "IBM LZ77";
    Compression_Method[97] = "WavPack";
    Compression_Method[98] = "PPMd version I, Rev 1";

    int END = 0;

    gzFile infile = gzopen("hello.zip", "rb");
    if (!infile) return -1;

    std::vector<LocalFileHeader*> Headers;
    size_t offset = 0;

    do {

        uint32_t buffer = 0;
        gzseek(infile, offset, SEEK_SET);
        gzread(infile, &buffer, sizeof(uint32_t));
        //return 1;
        if((uint32_t)buffer == 0x04034b50) {
            Headers.push_back(getLocalHeader(infile, offset));
            offset +=    Headers.back()->filenamelength  +
                         Headers.back()->compsize        +
                         Headers.back()->extrafieldlength+
                         30;
            cout << "Offset: " << offset << endl;
        } else if((uint32_t)buffer == 0x02014b50) {
            ///Central directory stuff
            cout << "Central Directory Header" << endl;
            CentralDirectoryHeader* cdh = new CentralDirectoryHeader();

            gzseek(infile, offset, SEEK_SET);
            gzread(infile, cdh, sizeof(char) * 46);
            offset += cdh->extrafieldlength + cdh->filecommentlength + cdh->filenamelength + 46;
            cout << "Offset: " << offset << endl;
        } else if((uint32_t)buffer == 0x06054b50) {
            ///end stuff
            cout << "End header" << endl;
            END = 1;
        }


    } while(END == 0);

    gzclose(infile);
    //fclose(outfile);
    return 0;
}
