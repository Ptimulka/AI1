#pragma once

#include <string>
#include "typedefs.h"

class OclContext;

class OclKernel
{
public:
    OclKernel(std::string const& path, OclContext* context = nullptr);
    ~OclKernel();

    OclKernel(OclKernel const& other) = delete;

    inline std::string getPath() const
    {
        return mypath;
    }

    inline bool buildSuccessfully() const { return build_success; }

    struct Instance;

    template <typename... ARGS>
    Instance* getCallableInstance(std::string kernelName, ARGS... a)
    {
        Instance* i = _createNewKernel(kernelName);
        _pushArgs(i, a...);
        return i;
    }

    void run(Instance* i, uint xworkers, uint yworkers) const;

private:
    std::string mypath;
    bool build_success = false;

    void* myprogram = nullptr;

    OclContext* mycontext = nullptr;

    OclKernel::Instance * _createNewKernel(std::string const & kernelname);

    template <typename... ARGS>
    void _pushArgs(Instance* i, ARGS... a)
    {
        return;
    }
    template <typename ARG, typename... REST>
    void _pushArgs(Instance* i, ARG a, REST... rest)
    {
        _pushArg(i, (const char*)&a, sizeof(a));
        _pushArgs(i, rest...);
    }
    void _pushArg(Instance* i, const char* ptr, size_t size);
};
