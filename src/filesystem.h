#pragma once

#include <zip.h>
#include <File.h>
#include <unordered_map>
#include <string>
#include <fstream>

namespace Filesystem {

std::unordered_map<std::string, std::shared_ptr<Zip> > m_Container;

static bool Mount(std::string name) {
    m_Container[name] = (std::shared_ptr<Zip>) new Zip(name);
    return true;
}

static bool Unmount(std::string name) {
    std::unordered_map<std::string, std::shared_ptr<Zip> >::iterator it;
    it = m_Container.find(name);
    m_Container.erase(it);
    return true;
}

static std::shared_ptr<File> open(std::string name) {
    return m_Container["hello.dat"]->getFileByName(name);
}

}
