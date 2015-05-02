#include "oclplatform.h"
#include "utils/strutils.h"

#include <CL/cl.h>
#include <cassert>
using namespace std;

struct OclPlatform::_Data
{
    unsigned int ref = 0;

    cl_platform_id id;
    string name = "";
    string vendor = "";
    unsigned int major = 0;
    unsigned int minor = 0;
};

OclPlatform::OclPlatform(void* platform)
{
    if (platform != nullptr)
        _init(platform);
}

OclPlatform::OclPlatform(OclPlatform const& other) : d(other.d)
{
    if (d)
        ++d->ref;
}

OclPlatform::OclPlatform(OclPlatform&& other) : d(other.d)
{
    other.d = nullptr;
}

OclPlatform::~OclPlatform()
{
    _release();
}

#define IMPL_PROP_GET(f_name, prop_name) \
    auto OclPlatform::f_name() const -> decltype(OclPlatform::_Data::prop_name) \
    { \
        if (!valid()) throw runtime_error("OclPlatform not valid!"); \
        return d->prop_name; \
    }

IMPL_PROP_GET(getName, name)
IMPL_PROP_GET(getVendor, vendor)
IMPL_PROP_GET(getOclMajorVersion, major)
IMPL_PROP_GET(getOclMinorVersion, minor)

vector<OclDevice> OclPlatform::queryDevices() const
{
    if (!valid())
        throw exception("OclPlatform not valid!");

    cl_uint deviceIdCount = 0;
    clGetDeviceIDs(d->id, CL_DEVICE_TYPE_ALL, 0, nullptr, &deviceIdCount);

    std::vector<cl_device_id> deviceIds(deviceIdCount);
    clGetDeviceIDs(d->id, CL_DEVICE_TYPE_ALL, deviceIdCount, deviceIds.data(), nullptr);

    vector<OclDevice> ret;
    for (auto did : deviceIds)
        ret.push_back(OclDevice(this, did));

    return ret;
}

vector<OclPlatform> OclPlatform::queryPlatforms()
{
    cl_uint platformIdCount = 0;
    clGetPlatformIDs(0, nullptr, &platformIdCount);

    vector<cl_platform_id> platformIds(platformIdCount);
    clGetPlatformIDs(platformIdCount, platformIds.data(), nullptr);

    vector<OclPlatform> ret;
    for (auto platform_id : platformIds)
        ret.push_back(OclPlatform(platform_id));

    return ret;
}

void* OclPlatform::getNativeHandler() const
{
    return d->id;
}

void OclPlatform::_init(void* p)
{
    cl_platform_id pid = (cl_platform_id)p;

    size_t ret_size = 0;
    char buff[1024];
    auto result = clGetPlatformInfo(pid, CL_PLATFORM_VERSION, sizeof(buff), buff, &ret_size);
    if (result == CL_INVALID_PLATFORM)
        return;

    d = new _Data;
    d->ref = 1;
    d->id = pid;

    string version = buff;
    auto dot = version.find('.');
    version[dot] = '\0';
    d->major = parse<decltype(d->major)>(&version[6]);
    d->minor = parse<decltype(d->minor)>(&version[dot + 1]);

    result = clGetPlatformInfo(pid, CL_PLATFORM_NAME, sizeof(buff), buff, &ret_size);
    assert(result == CL_SUCCESS);
    d->name = buff;
    result = clGetPlatformInfo(pid, CL_PLATFORM_VENDOR, sizeof(buff), buff, &ret_size);
    assert(result == CL_SUCCESS);
    d->vendor = buff;
}

void OclPlatform::_release()
{
    if (!d)
        return;
    if (0 == --d->ref)
        delete d;
    
    d = nullptr;
}
