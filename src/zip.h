#ifndef ZIP_H_INCLUDED
#define ZIP_H_INCLUDED

#include <iostream>
#include <zlib.h>
#include <cstdio>
#include <string>
#include <bitset>
#include <iomanip>
#include <map>
#include <vector>
#include <algorithm>
#include <memory>
#include <fstream>
#include <type_traits>
#include <cassert>

constexpr uint32_t LOCAL_FILE_HEADER_SIGNATURE = 0x04034b50;
constexpr uint32_t DATA_DESCRIPTOR_HEADER_SIGNATURE = 0x08074b50;
constexpr uint32_t CENTRAL_DIRECTORY_HEADER_SIGNATURE = 0x02014b50;
constexpr uint32_t END_OF_CENTRAL_DIRECTORY_HEADER_SIGNATURE = 0x06054b50;

#pragma pack(push, 1)
template <typename U16, typename U32>
struct LocalFileHeader {
    /** offset 0*/  U32        signature; /// 0x04034b50

    /** offset 4*/  U16        minversion;
    /** offset 6*/  U16        flag;

    /** offset 8*/  U16        compmethod;
    /** offset 10*/ U32        lastmodtime;

    /** offset 14*/ U32        CRC32;

    /** offset 20*/ U32        compsize;

    /** offset 24*/ U32        uncompsize;

    /** offset 28*/ U16        filenamelength;
    /** offset 30*/ U16        extrafieldlength;
};

#pragma pack(push, 1)
template <typename U32>
struct DataDescriptor {
    /** offset 0*/  U32        signature; /// 0x08074b50
    /** offset 4*/  U32        CRC32;
    /** offset 8*/  U32        compsize;
    /** offset 12*/ U32        uncompsize;
};

#pragma pack(push, 1)
template <typename U16, typename U32>
struct CentralDirectoryHeader {
    /** offset 0*/  U32        signature; ///0x02014b50

    /** offset 4*/  U16        version;
    /** offset 6*/  U16        minversion;

    /** offset 8*/  U16        flag;
    /** offset 10*/ U16        commethod;

    /** offset 12*/ U16        lastmodtimel;
    /** offset 14*/ U16        lastmoddate;

    /** offset 16*/ U32        CRC32;

    /** offset 20*/ U32        compsize;

    /** offset 24*/ U32        uncomsize;

    /** offset 28*/ U16        filenamelength;
    /** offset 30*/ U16        extrafieldlength;

    /** offset 32*/ U16        filecommentlength;
    /** offset 34*/ U16        filestartsdisk;

    /** offset 36*/ U16        intatt;

    /** offset 38*/ U32        extatt;

    /** offset 42*/ U32        offsettolocalheader;
};

#pragma pack(push, 1)
template <typename U16, typename U32>
struct EndOfCentralDirectory {
    /** offset 0*/  U32        signature; ///0x06054b50
    /** offset 4*/  U16        disknum;
    /** offset 6*/  U16        diskcendirstarts;
    /** offset 8*/  U16        numcdr;
    /** offset 10*/ U16        totalcdr;
    /** offset 12*/ U32        cdsize;
    /** offset 16*/ U32        offsetcd;
    /** offset 20*/ U16        commentlength;
};

char * Compression_Method[20] = {
    "Stored",
    "Shrunk",
    "Reduced with compression factor 1",
    "Reduced with compression factor 2",
    "Reduced with compression factor 3",
    "Reduced with compression factor 4",
    "Imploded",
    "Tokenized",
    "Deflated",
    "Deflate64",
    "IBM TERSE (old)",
    "PKWARE",
    "BZIP2",
    "PKWARE",
    "LZMA (EFS)",
    "PKWARE",
    "PKWARE",
    "PKWARE",
    "IBM TERSE (new)",
    "IBM LZ77"
};



class Zip {
public:
    Zip(std::string path) {

        m_Infile = (std::shared_ptr<std::ifstream>) new std::ifstream(path, std::ifstream::binary);

        auto isFileaccesible = [&]()-> bool {
            return (m_Infile->is_open()) ? true : false;
        };

        assert(isFileaccesible());

        size_t currentOffset;

        auto checkSignature = [&](uint32_t signature) -> size_t {
            switch(signature) {
            case LOCAL_FILE_HEADER_SIGNATURE:
                return this->getLocalHeader(currentOffset);
                break;

            case DATA_DESCRIPTOR_HEADER_SIGNATURE:
                return true;
                break;

            case CENTRAL_DIRECTORY_HEADER_SIGNATURE:
                return this->getCentralDirectoryHeader(currentOffset);
                return true;
                break;

            case END_OF_CENTRAL_DIRECTORY_HEADER_SIGNATURE:
                return true;
                break;

            default:
                return false;
                break;

            }
        };

        auto getSignature = [&](uint32_t offset)-> uint32_t {
            if(!isFileaccesible()) {
                return false;
            }

            uint32_t buffer = 0;
            this->m_Infile->seekg(offset, this->m_Infile->beg);
            this->m_Infile->read((char*)&buffer, sizeof(uint32_t));

            return buffer;
        };

        do {
            uint32_t signature = getSignature(currentOffset);

            currentOffset += checkSignature(signature);

        } while(
            !m_Infile->eof()       ||
            m_EndCentralDirectory != nullptr
        ); /**End while loop*/

        }

private:

    size_t getLocalHeader(size_t curroffset) {

        std::shared_ptr<LocalFileHeader<uint16_t, uint32_t> > localheader = (std::shared_ptr<LocalFileHeader<uint16_t, uint32_t> >)
                new LocalFileHeader<uint16_t, uint32_t> ();

        m_Infile->seekg(curroffset, m_Infile->beg);
        m_Infile->read(reinterpret_cast<char*>(localheader.get()), sizeof(char)* 30);

        m_LocalFileHeaders.push_back(localheader);

        curroffset +=    localheader->filenamelength     +
                         localheader->compsize           +
                         localheader->extrafieldlength   +
                         30;

        return curroffset;

    }

    size_t getCentralDirectoryHeader(size_t curroffset) {
        std::shared_ptr<CentralDirectoryHeader<uint16_t, uint32_t> > cdh = (std::shared_ptr<CentralDirectoryHeader<uint16_t, uint32_t> >)
                new CentralDirectoryHeader<uint16_t, uint32_t> ();

        m_Infile->seekg(curroffset, m_Infile->beg);
        m_Infile->read(reinterpret_cast<char*>(cdh.get()), sizeof(char) * 46);
        curroffset += cdh->extrafieldlength + cdh->filecommentlength + cdh->filenamelength + 46;

        return curroffset;
    }

private:

    std::shared_ptr<std::ifstream> m_Infile;
    std::vector<std::shared_ptr<LocalFileHeader<uint16_t, uint32_t> > > m_LocalFileHeaders;
    std::vector<std::shared_ptr<DataDescriptor<uint32_t> > > m_DataDescriptors;
    std::vector<std::shared_ptr<CentralDirectoryHeader<uint16_t, uint32_t> > > m_CentralDirectoryHeaders;
    std::shared_ptr<EndOfCentralDirectory<uint16_t, uint32_t> > m_EndCentralDirectory;

protected:

};

#endif // ZIP_H_INCLUDED
