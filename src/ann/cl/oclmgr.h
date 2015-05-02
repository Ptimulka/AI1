#pragma once

#include "utils/singleton.h"

class OclDevice;
class OclContext;

class OclMgr : public Singleton<OclMgr>
{
public:
    OclMgr();
    ~OclMgr();

    inline OclDevice* getComputeDevice() const { return _computeDevice; }
    inline OclContext* getDefaultContext() const { return _oclContext; }

private:
    OclDevice* _computeDevice;
    OclContext* _oclContext;
};
