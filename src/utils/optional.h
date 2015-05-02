#pragma once

#include <cassert>
#include <algorithm>

template <typename T>
struct optional
{
public:
    optional() = default;

    optional(T const& t) : ptr(new T(t))
    {}
    optional(T&& t) : ptr(new T(std::move(t)))
    {}

    template <typename T1, typename... TS>
    optional(T1 arg1, TS... args) : ptr(new T(arg1, args...))
    {}

    optional(optional const& other)
    {
        if (other.ptr != nullptr)
            ptr = new T(*other.ptr);
    }
    optional(optional&& other) : ptr(other.ptr)
    {
        other.ptr = nullptr;
    }
    ~optional()
    {
        abandon();
    }

    inline bool isSet() const           { return ptr != nullptr; }
    
    inline T& operator*()               { assert(isSet() && "Dereferencing optional variable which wasn't set!"); return *ptr; }
    inline T const& operator *() const  { assert(isSet() && "Dereferencing optional variable which wasn't set!"); return *ptr; }
    inline operator T() const           { return *(*this); }

    inline T* raw() const               { return ptr; }
    inline T* operator ->() const       { return ptr; }

    inline void abandon()               { delete ptr; ptr = nullptr; }

    inline optional& operator =(T const& t)
    {
        if (ptr)
        {
            ptr->~T();
            new (ptr) T(t);
        }
        else
            ptr = new T(t);

        return *this;
    }
    inline optional& operator =(T&& t)
    {
        if (ptr)
        {
            ptr->~T();
            new (ptr) T(std::move(t));
        }
        else
            ptr = new T(std::move(t));

        return *this;
    }

    inline optional& operator =(optional const& opt)        { if (!opt.isSet()) abandon(); else this->operator=(*opt); return *this; }
    inline optional& operator =(optional&& opt)             { std::swap(ptr, opt.ptr); return *this; }

private:
    T* ptr = nullptr;
};

