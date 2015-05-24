#pragma once

#include "ocldevice.h"
#include <tuple>

class OclContext
{
public:
    OclContext();
    OclContext(OclDevice create_on_device);
    ~OclContext();

    inline OclDevice getDevice() const { return device; }
    void* getNativeHandler() const;

    enum BufferMode
    {
        READ,
        WRITE,
        READ_WRITE
    };

    void* createBuffer(BufferMode mode, size_t size, void* ptr = nullptr);

    template <typename... TYPES>
    void* createBufferForData(BufferMode mode, TYPES... data)
    {
        auto _mem = std::make_tuple(std::move(data)...);
        return createBuffer(mode, _bytes_counter<TYPES...>::count, &_mem);
    }

private:
    struct _Data;
    _Data* d = nullptr;

    OclDevice device;

    void init(void* pid, void** dptrs, unsigned int dptrs_count);
    void release();

private:
    template <typename... _TS>
    struct _bytes_counter
    {
        static constexpr size_t count = 0;
    };
    template <typename _T, typename... _REST>
    struct _bytes_counter<_T, _REST...>
    {
        static constexpr size_t count = sizeof(_T) + _bytes_counter<_REST...>::count;
    };
};