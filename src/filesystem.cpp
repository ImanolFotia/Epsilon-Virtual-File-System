#include <filesystem.h>

static bool Filesystem::Mount(std::string name) {
    m_Container[name] = (std::shared_ptr<Zip>) new Zip(name);
    return true;
}

static bool Filesystem::Unmount(std::string name) {
    std::unordered_map<std::string, std::shared_ptr<Zip> >::iterator it;
    it = m_Container.find(name);
    m_Container.erase(it);
    return true;
}

static std::shared_ptr<File> Filesystem::open(std::string name) {

    std::shared_ptr<File> outFile;

    for(auto container: m_Container) {
        outFile = container.second->getFileByName(name);
        if(outFile == nullptr)
            continue;
    }

    return outFile;
}

