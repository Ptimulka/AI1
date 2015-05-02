#pragma once

#include "ocldevice.h"
#include <vector>

class OclPlatform
{
public:
    OclPlatform(void* platform = nullptr);
    OclPlatform(OclPlatform const& other);
    OclPlatform(OclPlatform&& other);
    ~OclPlatform();

    inline bool valid() const       { return d != nullptr; }

    std::string getName() const;
    std::string getVendor() const;
    unsigned int getOclMajorVersion() const;
    unsigned int getOclMinorVersion() const;

    std::vector<OclDevice> queryDevices() const;

    static std::vector<OclPlatform> queryPlatforms();

    void* getNativeHandler() const;

private:
    struct _Data;
    _Data* d = nullptr;

    void _init(void* p);
    void _release();
};
