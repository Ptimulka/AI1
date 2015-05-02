#pragma once

#include <string>
#include <vector>

class OclPlatform;

class OclDevice
{
public:
    OclDevice(const OclPlatform* platform, void* devide = nullptr);
    OclDevice(OclDevice const& other);
    OclDevice(OclDevice&& other);
    ~OclDevice();

    inline const OclPlatform* getPlatform() const { return my_platform; }

    inline bool valid() const       { return d != nullptr; }

    std::string getName() const;
    std::string getVendor() const;
    unsigned int getOclMajorVersion() const;
    unsigned int getOclMinorVersion() const;

    bool isAvaliable() const;
    unsigned int getAdressSize() const;
    bool canCompile() const; //check if device has compiler
    bool isLittleEndian() const;
    std::vector<size_t> getMaxWorkItems() const;
    unsigned int getAvaliableComputeUnits() const;
    unsigned getClockFrequency() const;
    bool hasImageSupport() const;
    size_t getImageMaxWidth() const;
    size_t getImageMaxHeight() const;

    void* getNativeHandler() const;

private:
    const OclPlatform* my_platform = nullptr;

    struct _Data;
    _Data* d = nullptr;

    void _init(void* dptr);
    void _release();
};

