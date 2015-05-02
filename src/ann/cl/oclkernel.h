#pragma once

#include <string>

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

private:
    std::string mypath;
    bool build_success = false;

    void* myprogram = nullptr;
};
