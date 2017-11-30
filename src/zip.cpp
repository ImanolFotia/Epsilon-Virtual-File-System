#include <zip.h>

Zip::Zip(std::string path) {

    m_InfilePath = path;
    m_Infile = (std::shared_ptr<std::ifstream>) new std::ifstream(m_InfilePath, std::ifstream::binary);

    auto isFileaccesible = [&]()-> bool {
        return (this->m_Infile->is_open()) ? true : false;
    };

    assert(isFileaccesible());

    char ZipIdentifier[2];
    this->m_Infile->seekg(0, this->m_Infile->beg);
    this->m_Infile->read((char*)&ZipIdentifier, 2*sizeof(char));

    if(ZipIdentifier[0] != 'P' && ZipIdentifier[1] != 'K'){
        std::cout << "Attempted to mount a not zip file... aborting." << std::endl;
        return;
    }

    size_t currentOffset = 0;
    m_EndCentralDirectory = nullptr;

    auto checkSignature = [&](uint32_t signature) -> size_t {
        switch(signature) {
        case LOCAL_FILE_HEADER_SIGNATURE:
            //std::cout << "Found Local Header" << std::endl;
            return this->getLocalHeader(currentOffset);
            break;

        case DATA_DESCRIPTOR_HEADER_SIGNATURE:
            //std::cout << "Found Data Descriptor" << std::endl;
            return this->getDataDescriptor(currentOffset);
            return true;
            break;

        case CENTRAL_DIRECTORY_HEADER_SIGNATURE:
            //std::cout << "Found Central Directory" << std::endl;
            return this->getCentralDirectoryHeader(currentOffset);
            return true;
            break;

        case END_OF_CENTRAL_DIRECTORY_HEADER_SIGNATURE:
            //std::cout << "Found End Of central Directory" << std::endl;
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
        //std::cout << std::hex << buffer << std::dec << std::endl;
        //std::cout << "Offset: " << offset << std::endl;

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

std::shared_ptr<File> Zip::getFileByName(const std::string FileName) {

    std::shared_ptr<File> tmpFile = nullptr;

    tmpFile = m_FileCollection[FileName];


    if(tmpFile == nullptr) {
        std::shared_ptr<File> oFile = (std::shared_ptr<File>) new File();
        oFile->m_isValid = false;
        return oFile;
    }

    CompressionInfo compInfo;

    auto RetrieveCompression = [&]()->void {
        compInfo.CompMethod = m_LocalFileHeaders.at( tmpFile->FileHeaderIndex )->compmethod;
        compInfo.CompressedSize = tmpFile->FileSizeComp;
        compInfo.UncompressedSize = tmpFile->FileSizeUncomp;
    };

    RetrieveCompression();

    tmpFile->compInfo = compInfo;

    return tmpFile;

}

size_t Zip::InflateFile(unsigned char* inputFileStream, uInt inputStreamSize, unsigned char* outputFileStream, uInt outputstreamSize, size_t windowBits) {
    z_stream infstream;
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


size_t Zip::getLocalHeader(size_t curroffset) {

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

    tmpFile->OffsetInContainer = 30 + sizeof(unsigned char)* localheader->filenamelength + curroffset;
    tmpFile->FileSizeComp = localheader->compsize;
    tmpFile->FileSizeUncomp = localheader->uncompsize;
    tmpFile->ContainerName = this->m_InfilePath;
    tmpFile->FileHeaderIndex = m_LocalFileHeaders.size();
    tmpFile->m_isValid = true;
    //std::cout << "Name: " << tmpFile->FileName  << std::endl;
    //std::cout << "Index for this file: " << tmpFile->FileHeaderIndex  << std::endl;

    m_FileCollection[tmpFile->FileName] = tmpFile;

    m_LocalFileHeaders.push_back(localheader);

    curroffset +=    localheader->filenamelength      +
                     localheader->compsize            +
                     localheader->extrafieldlength    +
                     30;

    return curroffset;

}

size_t Zip::getCentralDirectoryHeader(size_t curroffset) {
    std::shared_ptr<CentralDirectoryHeader<uint16_t, uint32_t> > cdh = (std::shared_ptr<CentralDirectoryHeader<uint16_t, uint32_t> >)
            new CentralDirectoryHeader<uint16_t, uint32_t> ();

    m_Infile->seekg(curroffset, m_Infile->beg);
    m_Infile->read(reinterpret_cast<char*>(cdh.get()), sizeof(char) * 46);

    m_CentralDirectoryHeaders.push_back(cdh);

    curroffset += cdh->extrafieldlength + cdh->filecommentlength + cdh->filenamelength + 46;

    return curroffset;
}

size_t Zip::getEndOfCentralDirectory(size_t curroffset) {
    std::shared_ptr<EndOfCentralDirectory<uint16_t, uint32_t> > eocd = (std::shared_ptr<EndOfCentralDirectory<uint16_t, uint32_t> >)
            new EndOfCentralDirectory<uint16_t, uint32_t> ();

    m_Infile->seekg(curroffset, m_Infile->beg);
    m_Infile->read(reinterpret_cast<char*>(eocd.get()), sizeof(char) * 20);

    m_EndCentralDirectory = eocd;

    curroffset += eocd->commentlength + 20;

    return curroffset;
}

size_t Zip::getDataDescriptor(size_t curroffset) {
    std::shared_ptr<DataDescriptor<uint32_t> > DD = (std::shared_ptr<DataDescriptor<uint32_t> >)
            new DataDescriptor<uint32_t> ();

    m_Infile->seekg(curroffset, m_Infile->beg);
    m_Infile->read(reinterpret_cast<char*>(DD.get()), sizeof(char) * 20);

    m_DataDescriptors.push_back(DD);

    curroffset += 16;

    return curroffset;
}
