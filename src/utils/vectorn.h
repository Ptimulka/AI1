#pragma once

#include "typedefs.h"
#include <vector>

template <typename T, uint DIM>
struct vectorn : public std::vector<vectorn<T, DIM - 1>>
{
    vectorn() : std::vector<vectorn<T, DIM - 1>>()
    {}

    template <typename... INTS>
    vectorn(T val, uint mysize, INTS... rest) : std::vector<vectorn<T, DIM - 1>>(mysize, vectorn<T, DIM - 1>(val, rest...))
    {
        static_assert(_all_same<uint, INTS...>::value, "");
        static_assert(sizeof...(rest) == DIM - 1, "");
    }

    template <typename... INTS>
    vectorn(uint mysize, INTS... rest) : std::vector<vectorn<T, DIM - 1>>(mysize, vectorn<T, DIM - 1>(rest...))
    {
        static_assert(_all_same<uint, INTS...>::value, "");
        static_assert(sizeof...(rest) == DIM - 1, "");
    }

private:
    template <typename TEST, typename T1, typename... REST> struct _all_same { enum { value = _all_same<TEST, T1>::value && _all_same<TEST, REST...>::value }; };
    template <typename TEST, typename A> struct _all_same<TEST, A> { enum { value = false }; };
    template <typename TEST> struct _all_same<TEST, TEST> { enum { value = true }; };
};

template <typename T>
struct vectorn<T, 1> : public std::vector<T>
{
    vectorn() : std::vector<T>()
    {}

    vectorn(T val, uint mysize) : std::vector<T>(mysize, val)
    {}

    vectorn(uint mysize) : std::vector<T>(mysize)
    {}
};

template <typename T>
struct vectorn<T, 0>
{
};
