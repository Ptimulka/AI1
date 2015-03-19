#include "cmdopt.h"
#include "utils/cmdline.h"
#include "utils/stringlist.h"
using namespace std;

namespace Opts
{
    std::string imgs_dir = ".";
    std::string imgs_groups_regexp = "default";
    std::string log_file = "AI.log";
    bool log_file_append = false;
}


namespace
{
    static CmdOpt imgs_dir(
        StringList({ "imgs-dir", "idir" }),
        "Specifies directory which contains grouped images to be used by application.\n"
        "All groups should have theirs' folder with name matching group's name.\n"
        "To select which group should be considered use -img-group option.\n\n"
        "Default value: . (current dir)",
        0, 1,
        CMDLINE_ASSIGMENT_HANDLER(Opts::imgs_dir));

    static CmdOpt imgs_groups_regexp(
        StringList({ "img-group", "igr" }),
        "Specifies which images groups should be used within application using regular expression to be matched agains group's name.\n"
        "Default value: default",
        0, 1,
        CMDLINE_ASSIGMENT_HANDLER(Opts::imgs_groups_regexp));

    static CmdOpt log_file(
        StringList({ "log-file", "lf", "log", "lfile" }),
        "Specifies log file.\n"
        "Default: AI.log",
        0, 1,
        CMDLINE_ASSIGMENT_HANDLER(Opts::log_file));

    static CmdOpt log_file_append(
        StringList({ "log-append" }),
        "Specifies if logger should append messeges to existing file rather then truncating it.\n"
        "Default: false",
        0, 1,
        CMDLINE_ASSIGMENT_HANDLER(Opts::log_file_append));
}
