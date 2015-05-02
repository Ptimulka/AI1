#include "ocldevice.h"
#include "utils/strutils.h"
#include "oclplatform.h"

#include <CL/cl.h>
#include <cassert>
#include <memory>
using namespace std;

#pragma warning(disable: 4800)

struct OclDevice::_Data
{
    unsigned int ref = 0;

    cl_device_id id;
    string name = "";
    string vendor = "";
    unsigned int major = 0;
    unsigned int minor = 0;

    bool avaliable = false;
    unsigned int adress_size = 0;
    bool has_compiler = false;
    bool little_endian = false;
    unsigned int max_witems_dim = 0;
    vector<size_t> max_witems = vector<size_t>();
    unsigned int max_compute_units = 0;
    unsigned int max_clock_freq = 0;
    bool image_support = false;
    size_t image_maxw = 0;
    size_t image_maxh = 0;
};

OclDevice::OclDevice(const OclPlatform* platform, void* device) : my_platform(new OclPlatform(*platform))
{
    assert(my_platform != nullptr);
    if (device != nullptr)
        _init(device);
}

OclDevice::OclDevice(OclDevice const& other) : my_platform(new OclPlatform(*other.my_platform)), d(other.d)
{
    if (d)
        ++d->ref;
}

OclDevice::OclDevice(OclDevice&& other) : my_platform(other.my_platform), d(other.d)
{
    other.d = nullptr;
    other.my_platform = nullptr;
}

OclDevice::~OclDevice()
{
    _release();
    delete my_platform;
}

#define IMPL_PROP_GET(f_name, prop_name) \
    auto OclDevice::f_name() const -> decltype(OclDevice::_Data::prop_name) \
    { \
        if (!valid()) throw runtime_error("OclDevice not valid!"); \
        return d->prop_name; \
    }

IMPL_PROP_GET(getName, name)
IMPL_PROP_GET(getVendor, vendor)
IMPL_PROP_GET(getOclMajorVersion, major)
IMPL_PROP_GET(getOclMinorVersion, minor)
IMPL_PROP_GET(isAvaliable, avaliable)
IMPL_PROP_GET(getAdressSize, adress_size)
IMPL_PROP_GET(canCompile, has_compiler)
IMPL_PROP_GET(isLittleEndian, little_endian)
IMPL_PROP_GET(getMaxWorkItems, max_witems)
IMPL_PROP_GET(getAvaliableComputeUnits, max_compute_units)
IMPL_PROP_GET(getClockFrequency, max_clock_freq)
IMPL_PROP_GET(hasImageSupport, image_support)
IMPL_PROP_GET(getImageMaxWidth, image_maxw)
IMPL_PROP_GET(getImageMaxHeight, image_maxh)

void* OclDevice::getNativeHandler() const
{
    return d->id;
}

#define IMPL_PROP_READ(prop, mem, constant) \
    result = clGetDeviceInfo(did, constant, sizeof(mem), addressof(mem), &ret_size); \
    assert(result == CL_SUCCESS); \
    d->prop = mem

void OclDevice::_init(void* dptr)
{
    cl_device_id did = (cl_device_id)dptr;

    size_t ret_size = 0;
    char buff[1024];
    cl_bool bval;
    size_t stval;
    cl_uint uival;

    auto result = clGetDeviceInfo(did, CL_DEVICE_VERSION, sizeof(buff), buff, &ret_size);
    if (result == CL_INVALID_DEVICE)
        return;

    d = new _Data;
    d->ref = 1;
    d->id = did;

    string version = buff;
    auto dot = version.find('.');
    version[dot] = '\0';
    d->major = parse<decltype(d->major)>(&version[6]);
    d->minor = parse<decltype(d->minor)>(&version[dot + 1]);

    IMPL_PROP_READ(name, buff, CL_DEVICE_NAME);
    IMPL_PROP_READ(vendor, buff, CL_DEVICE_VENDOR);
    IMPL_PROP_READ(avaliable, bval, CL_DEVICE_AVAILABLE);
    IMPL_PROP_READ(adress_size, uival, CL_DEVICE_ADDRESS_BITS);
    IMPL_PROP_READ(has_compiler, bval, CL_DEVICE_COMPILER_AVAILABLE);
    IMPL_PROP_READ(little_endian, bval, CL_DEVICE_ENDIAN_LITTLE);
    IMPL_PROP_READ(max_witems_dim, uival, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS);

    vector<size_t> sizes(d->max_witems_dim);
    result = clGetDeviceInfo(did, CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(size_t)*sizes.size(), &sizes.front(), &ret_size);
    assert(result == CL_SUCCESS);
    d->max_witems = sizes;

    IMPL_PROP_READ(max_compute_units, uival, CL_DEVICE_MAX_COMPUTE_UNITS);
    IMPL_PROP_READ(max_clock_freq, uival, CL_DEVICE_MAX_CLOCK_FREQUENCY);
    IMPL_PROP_READ(image_support, bval, CL_DEVICE_IMAGE_SUPPORT);
    IMPL_PROP_READ(image_maxw, stval, CL_DEVICE_IMAGE2D_MAX_WIDTH);
    IMPL_PROP_READ(image_maxh, stval, CL_DEVICE_IMAGE2D_MAX_HEIGHT);
}

void OclDevice::_release()
{
    if (!d)
        return;
    if (0 == --d->ref)
        delete d;

    d = nullptr;
}
