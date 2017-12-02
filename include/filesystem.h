#pragma once

#include <zip.h>
#include <unordered_map>
#include <string>
#include <fstream>
#include <sys/stat.h>

#ifndef __cplusplus > 201402L
#error c++ 14 not available
#endif // __cplusplus

#ifdef __cplusplus

extern "C" {
#endif

#ifdef BUILDING_VFS_DLL
#define VFS_DLL __declspec(dllexport)
#else
#define VFS_DLL __declspec(dllimport)
#endif

#ifdef __cplusplus
}
#endif

namespace Filesystem {

static std::unordered_map<std::string, std::shared_ptr<Zip> > m_Container;

__stdcall static bool Mount(std::string name) {
    struct stat s;
    if( stat(name.c_str(),&s) == 0 ) {
        if( s.st_mode & S_IFDIR ) {
            //it's a directory
        } else if( s.st_mode & S_IFREG ) {
            m_Container[name] = (std::shared_ptr<Zip>) new Zip(name);
            std::cout << name << ": Virtual File System Mounted" <<std::endl;
        } else {
            //something else
            return false;
        }
    } else {
        //error
        return false;
    }
    return true;
}

__stdcall static bool Unmount(std::string name) {
    std::unordered_map<std::string, std::shared_ptr<Zip> >::iterator it;
    it = m_Container.find(name);
    m_Container.erase(it);
    return true;
}

template <typename T>
__stdcall static T open(std::string name) {

    T outFile;

    for(auto container: m_Container) {
        outFile = container.second->getFileByName(name);
        if(outFile->m_isValid != false)
            break;
    }

    if(outFile->m_isValid == false)
        std::cout << name << ": No such file or directory" << std::endl;

    outFile->m_isopen = true;
    return outFile;
}

template <typename T>
__stdcall static bool close(T file) {
    file->close();
}

}
