#ifndef ZIP_H_INCLUDED
#define ZIP_H_INCLUDED

#include <iostream>
#include <zlib.h>
#include <cstdio>
#include <string>
#include <bitset>
#include <iomanip>
#include <map>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <memory>
#include <fstream>
#include <type_traits>
#include <cassert>
#include <File.h>

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

class File;

class Zip {
public:
    Zip(std::string path) {

        m_Infile = (std::shared_ptr<std::ifstream>) new std::ifstream(path, std::ifstream::binary);

        auto isFileaccesible = [&]()-> bool {
            return (this->m_Infile->is_open()) ? true : false;
        };

        assert(isFileaccesible());

        size_t currentOffset = 0;
        m_EndCentralDirectory = nullptr;

        auto checkSignature = [&](uint32_t signature) -> size_t {
            switch(signature) {
            case LOCAL_FILE_HEADER_SIGNATURE:
                std::cout << "Found Local Header" << std::endl;
                return this->getLocalHeader(currentOffset);
                break;

            case DATA_DESCRIPTOR_HEADER_SIGNATURE:
                std::cout << "Found Data Descriptor" << std::endl;
                return this->getDataDescriptor(currentOffset);
                return true;
                break;

            case CENTRAL_DIRECTORY_HEADER_SIGNATURE:
                std::cout << "Found Central Directory" << std::endl;
                return this->getCentralDirectoryHeader(currentOffset);
                return true;
                break;

            case END_OF_CENTRAL_DIRECTORY_HEADER_SIGNATURE:
                std::cout << "Found End Of central Directory" << std::endl;
                return this->getEndOfCentralDirectory(currentOffset);
                return true;
                break;

            default:
                return currentOffset;
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
            std::cout << std::hex << buffer << std::dec << std::endl;
            std::cout << "Offset: " << offset << std::endl;

            return buffer;
        };

        do {
            uint32_t signature = getSignature(currentOffset);

            currentOffset = checkSignature(signature);

        } while(
            !m_Infile->eof() &&
             m_EndCentralDirectory == nullptr
        ); /**End while loop*/

        m_Infile->close();
    }

    std::shared_ptr<File> getFileByName(const std::string FileName){

        std::shared_ptr<File> tmpFile = nullptr;

        tmpFile = m_FileCollection[FileName];

        struct CompressionInfo{
            uint16_t CompMethod;
            uint32_t CompressedSize;
            uint32_t UncompressedSize;
        };

        CompressionInfo compInfo;

        auto RetrieveCompression = [&]()->void {
                                compInfo.CompMethod = m_LocalFileHeaders.at( tmpFile->FileHeaderIndex )->compmethod;
                                compInfo.CompressedSize = tmpFile->FileSizeComp;
                                compInfo.UncompressedSize = tmpFile->FileSizeUncomp;
        };

        RetrieveCompression();

        std::shared_ptr<unsigned char> fileContent;

        switch(compInfo.CompMethod){
            case 0: /**Content is stored*/
                /**Nothing to do here, the content is just stored*/
            break;
            case 8: /**Compressed using Deflate*/
                fileContent = (std::shared_ptr<unsigned char>) new unsigned char[tmpFile->FileSizeUncomp];
                this->InflateFile(tmpFile->Data.get(), tmpFile->FileSizeComp, fileContent.get(), tmpFile->FileSizeUncomp, -MAX_WBITS);
                tmpFile->Data = fileContent;
            break;
            default:
                std::cout << "This compression method is not supported yet." << std::endl;
                break;
        }


        return tmpFile;

    }

private:

    size_t getLocalHeader(size_t curroffset) {

        std::shared_ptr<LocalFileHeader<uint16_t, uint32_t> > localheader = (std::shared_ptr<LocalFileHeader<uint16_t, uint32_t> >)
                new LocalFileHeader<uint16_t, uint32_t> ();

        std::shared_ptr<File> tmpFile = (std::shared_ptr<File>) new File();

        m_Infile->seekg(curroffset, m_Infile->beg);
        m_Infile->read(reinterpret_cast<char*>(localheader.get()), sizeof(char)* 30);

        std::shared_ptr<unsigned char> buffer = (std::shared_ptr<unsigned char>) new unsigned char[localheader->filenamelength];
        m_Infile->read(reinterpret_cast<char*>(buffer.get()), sizeof(unsigned char)* localheader->filenamelength);

        tmpFile->FileName = std::string(&buffer.get()[0], &buffer.get()[localheader->filenamelength]);


        buffer.reset();
        buffer = (std::shared_ptr<unsigned char>) new unsigned char[localheader->uncompsize];
        m_Infile->read(reinterpret_cast<char*>(buffer.get()), sizeof(unsigned char) * localheader->uncompsize);

        tmpFile->Data = buffer;
        tmpFile->FileSizeComp = localheader->compsize;
        tmpFile->FileSizeUncomp = localheader->uncompsize;
        tmpFile->Data = buffer;
        tmpFile->FileHeaderIndex = m_LocalFileHeaders.size();
        std::cout << "Name: " << tmpFile->FileName  << std::endl;
        std::cout << "Index for this file: " << tmpFile->FileHeaderIndex  << std::endl;

        m_FileCollection[tmpFile->FileName] = tmpFile;

        m_LocalFileHeaders.push_back(localheader);

        curroffset +=    localheader->filenamelength      +
                         localheader->compsize            +
                         localheader->extrafieldlength    +
                         30;

        return curroffset;

    }

    size_t getCentralDirectoryHeader(size_t curroffset) {
        std::shared_ptr<CentralDirectoryHeader<uint16_t, uint32_t> > cdh = (std::shared_ptr<CentralDirectoryHeader<uint16_t, uint32_t> >)
                new CentralDirectoryHeader<uint16_t, uint32_t> ();

        m_Infile->seekg(curroffset, m_Infile->beg);
        m_Infile->read(reinterpret_cast<char*>(cdh.get()), sizeof(char) * 46);

        m_CentralDirectoryHeaders.push_back(cdh);

        curroffset += cdh->extrafieldlength + cdh->filecommentlength + cdh->filenamelength + 46;

        return curroffset;
    }

    size_t getEndOfCentralDirectory(size_t curroffset) {
        std::shared_ptr<EndOfCentralDirectory<uint16_t, uint32_t> > eocd = (std::shared_ptr<EndOfCentralDirectory<uint16_t, uint32_t> >)
                new EndOfCentralDirectory<uint16_t, uint32_t> ();

        m_Infile->seekg(curroffset, m_Infile->beg);
        m_Infile->read(reinterpret_cast<char*>(eocd.get()), sizeof(char) * 20);

        m_EndCentralDirectory = eocd;

        curroffset += eocd->commentlength + 20;

        return curroffset;
    }

    size_t getDataDescriptor(size_t curroffset) {
        std::shared_ptr<DataDescriptor<uint32_t> > DD = (std::shared_ptr<DataDescriptor<uint32_t> >)
                new DataDescriptor<uint32_t> ();

        m_Infile->seekg(curroffset, m_Infile->beg);
        m_Infile->read(reinterpret_cast<char*>(DD.get()), sizeof(char) * 20);

        m_DataDescriptors.push_back(DD);

        curroffset += 16;

        return curroffset;
    }

    size_t InflateFile(unsigned char* inputFileStream, uInt inputStreamSize, unsigned char* outputFileStream, uInt outputstreamSize, size_t windowBits) {
        z_stream infstream;
        unsigned char *c = new Bytef[outputstreamSize];
        infstream.zalloc = Z_NULL;
        infstream.zfree = Z_NULL;
        infstream.opaque = Z_NULL;
        // setup "b" as the input and "c" as the compressed output
        infstream.avail_in = (uInt)(inputStreamSize); // size of input
        infstream.next_in = (Bytef *)inputFileStream; // input char array
        infstream.avail_out = (uInt)outputstreamSize; // size of output
        infstream.next_out = (Bytef *)outputFileStream; // output char array

        size_t error = 0;

        auto checkError = [](size_t error)->size_t{
                switch (error) {
                    case Z_NEED_DICT:
                        std::cout << "Need Dictionary" << std::endl;
                        return error;
                    break;
                    case Z_DATA_ERROR:
                        std::cout << "Data error" << std::endl;
                        return error;
                    break;
                    case Z_MEM_ERROR:
                        std::cout << "Memory error" << std::endl;
                        return error;
                    break;
                    default:
                        return error;
                    break;
                }
        };
        // the actual DE-compression work.
        inflateInit2(&infstream, windowBits);

        error = checkError(inflate(&infstream, Z_NO_FLUSH));

        inflateEnd(&infstream);

        return error;
    }

private:

    std::shared_ptr<std::ifstream> m_Infile;
    std::vector<std::shared_ptr<LocalFileHeader<uint16_t, uint32_t> > > m_LocalFileHeaders;
    std::vector<std::shared_ptr<DataDescriptor<uint32_t> > > m_DataDescriptors;
    std::vector<std::shared_ptr<CentralDirectoryHeader<uint16_t, uint32_t> > > m_CentralDirectoryHeaders;
    std::shared_ptr<EndOfCentralDirectory<uint16_t, uint32_t> > m_EndCentralDirectory = nullptr;


    std::unordered_map<std::string, std::shared_ptr<File> > m_FileCollection;
protected:

};

#endif // ZIP_H_INCLUDED
