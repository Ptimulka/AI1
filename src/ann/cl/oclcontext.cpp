#include "oclcontext.h"
#include "utils/log.h"
#include "oclmgr.h"
#include "ocldevice.h"
#include "oclplatform.h"
#include <CL/cl.h>
#include <cassert>

struct OclContext::_Data
{
    unsigned int ref = 0;
    cl_context context;
};

OclContext::OclContext() : OclContext(*OclMgr::singleton().getComputeDevice())
{
}

OclContext::OclContext(OclDevice create_on_device) : device(create_on_device)
{
    void* dptrs[] = { device.getNativeHandler() };
    init(device.getPlatform()->getNativeHandler(), dptrs, 1);
}

OclContext::~OclContext()
{
    release();
}

void* OclContext::getNativeHandler() const
{
    return d->context;
}

void OclContext::init(void* pptr, void** dptrs, unsigned int dptrs_count)
{
    cl_platform_id pid = (cl_platform_id)pptr;
    cl_device_id* dids = (cl_device_id*)dptrs;

    cl_context_properties props[] = { CL_CONTEXT_PLATFORM, (cl_context_properties)pid, 0 };
    cl_int errcode;
    cl_context myself = clCreateContext(props, dptrs_count, dids, nullptr, nullptr, &errcode);
    if (errcode == CL_SUCCESS)
    {
        d = new _Data;
        d->ref = 1;
        d->context = myself;
    }
    else
        sLog.log("Error while initializing OpenCL context, error code: ", errcode);
}

void OclContext::release()
{
    if (!d)
        return;

    if (!d->ref--)
    {
        clReleaseContext(d->context);
        delete d;
        d = nullptr;
    }
}
