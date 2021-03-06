#include "oclkernel.h"
#include "utils/log.h"
#include "oclcontext.h"
#include "ocldevice.h"
#include "oclmgr.h"
#include <fstream>
#include <CL/cl.h>
#include <cassert>
using namespace std;

OclKernel::OclKernel(string const& path, OclContext* context) : mypath(path)
{
    if (!context)
        context = OclMgr::singleton().getDefaultContext();

    mycontext = context;

    std::ifstream in(mypath);
    if (!in.is_open())
    {
        sLog.log("Could not find kernel file: ", mypath);
        return;
    }

    string mysource = string(istreambuf_iterator<char>(in), istreambuf_iterator<char>());

    size_t lengths[1] = { mysource.size() };
    const char* sources[1] = { mysource.data() };

    cl_int error = 0;
    cl_program program = clCreateProgramWithSource((cl_context)context->getNativeHandler(), 1, sources, lengths, &error);
    mysource.clear();

    if (error != CL_SUCCESS)
    {
        sLog.log('"', mypath, "\": OpenCL program creation failed with error code: ", error);
        return;
    }

    myprogram = program;
	error = clBuildProgram(program, 0, nullptr, "", nullptr, nullptr);
    if (error != CL_SUCCESS && error != CL_BUILD_PROGRAM_FAILURE)
    {
        sLog.log('"', mypath, "\": OpenCL program build failed with error code: ", error);
        return;
    }

    cl_build_status status;
    clGetProgramBuildInfo(program, (cl_device_id)context->getDevice().getNativeHandler(), CL_PROGRAM_BUILD_STATUS, sizeof(status), &status, nullptr);

    assert(status != CL_BUILD_NONE);
    assert(status != CL_BUILD_IN_PROGRESS);
    if (status == CL_BUILD_ERROR)
    {
        char log[2048];
        size_t log_len;
        clGetProgramBuildInfo(program, (cl_device_id)context->getDevice().getNativeHandler(), CL_PROGRAM_BUILD_LOG, sizeof(log), log, &log_len);
        assert(log_len != sizeof(log));
        sLog.log("kernel '", mypath, "' compilation failed.\n\n", log, "\n\n");
    }
    else
    {
        sLog.log("kernel '", mypath, "' has compiled successfully on device ", context->getDevice().getName(), ".\n");
        build_success = true;
    }
}

OclKernel::~OclKernel()
{
    clReleaseProgram((cl_program)myprogram);
}

struct OclKernel::Instance
{
    cl_kernel kernel;
    cl_uint args;
};

void OclKernel::run(Instance * i, uint xworkers, uint yworkers) const
{
    size_t dim[2] = { xworkers, yworkers };
    cl_event calc_done;
    clEnqueueNDRangeKernel((cl_command_queue)mycontext->getCommandQueue(), i->kernel, 2, nullptr, dim, dim, 0, nullptr, &calc_done);
    clWaitForEvents(1, &calc_done);
}

OclKernel::Instance* OclKernel::_createNewKernel(std::string const& kernelname)
{
    Instance* ret = new Instance;
    cl_int error;
    ret->kernel = clCreateKernel((cl_program)myprogram, kernelname.c_str(), &error);
    if (error != CL_SUCCESS)
    {
        sLog.log("error while creating new callable kernel instance: ", error);
        ret->kernel = nullptr;
    }
    ret->args = 0;
    return ret;
}

void OclKernel::_pushArg(Instance * i, const char * ptr, size_t size)
{
    clSetKernelArg(i->kernel, i->args++, size, ptr);
}

