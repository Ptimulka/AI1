#ifndef SHARED_STR_UTILS_H
#define SHARED_STR_UTILS_H

#include <string>
#include <sstream>
#include <ctime>
#include <chrono>
#include <vector>

#pragma warning(disable: 4996)

namespace
{
    template <typename T>
    inline std::ostream& _push(std::ostream& out, T const& t)
    {
        return out << t;
    }

    template <typename T>
    std::ostream& _push(std::ostream& out, std::vector<T> const& ts)
    {
        out << "[";
        for (decltype(ts.size()) i = 0; i < ts.size(); ++i)
        {
            _push(out, ts[i]);
            if (i + 1 < ts.size())
                out << ", ";
        }
        out << "]";
        return out;
    }

    template <>
    inline std::ostream& _push(std::ostream& out, std::chrono::system_clock::time_point const& tp)
    {
        std::time_t tt;
        tt = std::chrono::system_clock::to_time_t(tp);
        
        struct tm* t;
        t = localtime(&tt);

        char tmp[64];
        strftime(tmp, sizeof(tmp), "%d %B %Y %X", t);
        return out << tmp;
    }

    template <typename T>
    inline void _makestr(std::ostream& out, T const& t)
    {
        _push(out,t);
    }
    template <typename T, typename... ARGS>
    inline void _makestr(std::ostream& out, T const& t, ARGS const&... args)
    {
        return _makestr(_push(out,t), args...);
    }
}

template <typename... T>
std::string makestr(T const&... t)
{
    if (sizeof...(t) == 0)
        return "";

    std::stringstream ss;
    _makestr(ss, t...);
    return ss.str();
}

namespace
{
    template <typename T>
    T _stringStreamParse(std::string const& str)
    {
        stringstream ss;
        ss.str(str);

        T ret;
        ss >> ret;
        return ret;
    }
}

template <typename T>
inline T parse(std::string const& str)
{
    return _stringStreamParse<T>(str);
}

template <>
inline bool parse<bool>(std::string const& str)
{
    if (str == "1" || str == "y" || str == "yes" || str == "t" || str == "tak" || str == "true")
        return true;
    if (str == "0" || str == "n" || str == "no" || str == "nie" || str == "f" || str == "false")
        return false;
    throw std::runtime_error("Invalid value '" + str + "' passed to be parsed as bool!");
    return false;
}

template <typename T>
inline bool tryParse(std::string const& str, T& var)
{
    try { var = parse<T>(str); return true; }
    catch (...) { return false; }
}

#endif //SHARED_STR_UTILS_H
