#pragma once

#include "ocldevice.h"

class OclContext
{
public:
    OclContext();
    OclContext(OclDevice create_on_device);
    ~OclContext();

    inline OclDevice getDevice() const { return device; }
    void* getNativeHandler() const;

private:
    struct _Data;
    _Data* d = nullptr;

    OclDevice device;

    void init(void* pid, void** dptrs, unsigned int dptrs_count);
    void release();
};