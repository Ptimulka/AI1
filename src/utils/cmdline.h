#ifndef SHARED_CMDLINE_H
#define SHARED_CMDLINE_H

#include "singleton.h"
#include "strutils.h"
#include "stringlist.h"

#include <list>
#include <string>
#include <vector>
#include <functional>
#include <type_traits>

//callback to be called when processing values for CmdOpt
typedef std::function<bool(StringList const&)> OptionHandler;

template <typename T>
struct AssigmentHandler
{
    inline AssigmentHandler(T& t) : ref(t)
    {}

    T& ref;

    bool operator()(StringList const& values)
    {
        return tryParse(values.front(), ref);
    };
};

template <>
struct AssigmentHandler<bool>
{
    inline AssigmentHandler(bool& t) : ref(t)
    {}

    bool& ref;

    bool operator()(StringList const& values)
    {
        if (values.empty())
        {
            ref = true;
            return true;
        }

        return tryParse(values.front(), ref);
    }
};

#define CMDLINE_ASSIGMENT_HANDLER(variable) \
    AssigmentHandler<decltype(variable)>(variable)

class CmdLine : public Singleton<CmdLine>
{
    friend struct CmdOpt;

public:
    CmdLine();
    ~CmdLine();

    void parse(const char* args[], unsigned int count);
    void parse(std::string args);
    void parse(StringList args);


    inline void setDefaultOptionHandler(OptionHandler const& val)
    {
        default_handler = val;
    }

private:
    std::list<CmdOpt*> opts;
    OptionHandler default_handler;

    void _preParse(StringList& list);
    void _handle(CmdOpt* opt, StringList values);

    inline void _registerOpt(CmdOpt* opt)
    {
        opts.push_back(opt);
    }
    inline void _unregisterOpt(CmdOpt* opt)
    {
        opts.remove(opt);
    }
};

struct CmdOpt
{
    inline CmdOpt()
    {
        CmdLine::singleton()._registerOpt(this);
    }
    inline CmdOpt(StringList&& names, std::string&& desc, unsigned int min_args, unsigned int max_args, OptionHandler&& hnd) : CmdOpt()
    {
        this->names = names; this->desc = desc; this->min_args = min_args; this->max_args = max_args; this->handler = hnd;
    }
    inline ~CmdOpt()
    {
        CmdLine::singleton()._unregisterOpt(this);
    }

    StringList names = StringList();
    OptionHandler handler = [](StringList const&) -> bool { throw std::runtime_error("CmdOpt's handler called but not set!");  return false; };
    std::string desc = "";
    unsigned int min_args = 0;
    unsigned int max_args = 0;
};

#endif //SHARED_CMDLINE_H
