#include <File.h>
#include <filesystem.h>

int File::read(char * s, unsigned int n) {

    std::shared_ptr<std::ifstream> InFile =  (std::shared_ptr<std::ifstream>) new std::ifstream(ContainerName, std::ifstream::binary);

    std::shared_ptr<unsigned char> buffer = (std::shared_ptr<unsigned char>) new unsigned char[this->FileSizeComp];
    InFile->seekg(OffsetInContainer, InFile->beg);
    InFile->read(reinterpret_cast<char*>(buffer.get()), sizeof(unsigned char) * this->FileSizeComp);
    std::shared_ptr<unsigned char> fileContent = (std::shared_ptr<unsigned char>) new unsigned char[this->FileSizeUncomp];

    switch(compInfo.CompMethod) {
    case 0: /**Content is stored*/
        /**Nothing to do here, the content is just stored*/
        fileContent = buffer;
        break;
    case 8: /**Compressed using Deflate*/
        Filesystem::m_Container[ContainerName]->InflateFile(buffer.get(), this->FileSizeComp, fileContent.get(), this->FileSizeUncomp, -MAX_WBITS);
        break;
    default:
        std::cout << "This compression method is not supported yet." << std::endl;
        return -1;
        break;
    }

    if(buffer.get() == nullptr) {
        return -1;
    }

    std::memcpy(s, &fileContent.get()[m_charposition], n);
    InFile->close();
    return 1;
}
