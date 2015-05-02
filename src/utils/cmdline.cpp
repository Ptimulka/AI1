#include "cmdline.h"
#include <algorithm>
using namespace std;

CmdLine::CmdLine()
{}

CmdLine::~CmdLine()
{

}

void CmdLine::parse(char** args, unsigned int count)
{
    StringList ret;
    for (decltype(count) i = 0; i < count; ++i)
        ret.push_back(args[i]);
    return parse(std::move(ret));
}

void CmdLine::parse(std::string args)
{
    StringList ret;
    auto pos = args.find(' ');
    while (pos != args.npos)
    {
        ret.push_back(std::string(args.c_str(), pos));
        args = args.substr(pos);
        pos = args.find(' ');
    }
    return parse(std::move(ret));
}

void CmdLine::parse(StringList args)
{
    _preParse(args);

    StringList free_items;
    

    CmdOpt* curr_opt = nullptr;
    string curr_opt_name = "";
    StringList opt_args_list;;

    for (auto itr = args.begin(); itr != args.end(); ++itr)
    {
        auto arg = *itr;
        if (arg.empty())
            continue;

        if (arg.substr(0, 1) == "-") //option
        {
            if (arg.size() == 1)
                throw logic_error("Invalid option '-' found on cmd line!");

            arg = arg.substr(1);
            auto itr2 = find_if(opts.begin(), opts.end(), [&arg](CmdOpt* el) -> bool { return find(el->names.begin(), el->names.end(), arg) != el->names.end(); });
            if (itr2 == opts.end())
                throw logic_error("Unknown option '" + arg + "' found on cmd line!");
            if (curr_opt)
            {
                if (opt_args_list.size() < curr_opt->min_args)
                    throw logic_error(makestr("Only ", opt_args_list.size(), " arguments passed for cmd option ", curr_opt_name, " while minimum ", curr_opt->min_args, " excepted"));

                _handle(curr_opt, opt_args_list);
            }
            curr_opt = *itr2;
            curr_opt_name = arg;
            opt_args_list.clear();
        }
        else //option argument (or free item)
        {
            if (curr_opt)
            {
                opt_args_list.push_back(arg);
                if (opt_args_list.size() == curr_opt->max_args)
                {
                    _handle(curr_opt, opt_args_list);
                    curr_opt = nullptr;
                    curr_opt_name = "";
                    opt_args_list.clear();
                }
            }
            else
                free_items.push_back(arg);
        }
    }

    if (curr_opt)
    {
        if (opt_args_list.size() < curr_opt->min_args)
            throw logic_error(makestr("Only ", opt_args_list.size(), " arguments passed for cmd option ", curr_opt_name, " while minimum ", curr_opt->min_args, " excepted"));

        _handle(curr_opt, opt_args_list);
    }

    if (default_handler)
        if (!default_handler(free_items))
            throw logic_error("Error processing free items!");
}

void CmdLine::_preParse(StringList& list)
{
    string tmp = "";
    bool string_parse = false;

    for (auto itr = list.begin(); itr != list.end(); ++itr)
    {
        auto& item = *itr;
        if (string_parse)
        {
            auto str_end = item.find('"');
            tmp += item.substr(0, str_end);
            string_parse = (str_end == itr->npos);
            if (string_parse)
            {
                tmp.append(" ");
                itr = --list.erase(itr);
            }
            else
                item = tmp;

            continue;
        }

        auto break_pos = item.find('=');
        auto str_begin = item.find('"');
        if (str_begin == item.npos && break_pos == item.npos)
            continue;

        if (str_begin == item.npos || break_pos < str_begin)
        {
            itr = list.insert(itr, item.substr(0, break_pos));
            item = item.substr(break_pos+1);
            continue;
        }

        if (break_pos == item.npos || str_begin < break_pos)
        {
            itr = list.insert(itr, item.substr(0, str_begin));
            item = item.substr(str_begin+1);
            string_parse = true;
            continue;
        }
    }
}

void CmdLine::_handle(CmdOpt* opt, StringList values)
{
    opt->handler(values);
    //todo (or no?): take care of handler returning false
}

