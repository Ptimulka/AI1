#include <iostream>
#include <fstream>
#include <vector>
#include <regex>

#include "ImageOperations.h"
#include "cmdopt.h"
#include "utils/log.h"
#include "utils/cmdline.h"
#include "utils/strutils.h"
#include "io/dir.h"

using namespace cv;
using namespace std;

#ifdef _DEBUG
void OverrideArgumentInDebuggin(int& argc, char**& argv);
#endif


int main(int argc, char** argv)
{
#ifdef _DEBUG
    OverrideArgumentInDebuggin(argc, argv);
#endif

    sLog.addOutput(&cout);

    try
    {
        CmdLine::singleton().parse(argv, argc);
    }
    catch (exception& e)
    {
        sLog.log("Error: ", e.what());
        system("pause");
        return 1;
    }

    auto flags = (Opts::log_file_append ? ios::out | ios::app : ios::out | ios::trunc);
    ofstream out(Opts::log_file, flags);
    if (!out.is_open())
        sLog.log("Could not open file ", Opts::log_file, " for logging - log output will only be pushed to console.");
    else
        sLog.addOutput(&out);


    wregex group_regexp(convert<string,wstring>(Opts::imgs_groups_regexp));
    Dir imgs_dir(convert<string, wstring>(Opts::imgs_dir));
    for (Dir group : imgs_dir.getSubDirs())
    {
        if (!regex_match(group.name(), group_regexp))
            continue;

        auto refimg = group.getEntries(L"ref.*");
        if (refimg.empty())
        {
            sLog.log("Group ", group.path(), " doesn't have reference image in it!");
            continue;
        }
        if (refimg.size() > 1)
        {
            sLog.log("More than one reference image found in group ", group.path());
            continue;
        }

        vector<string> imageNames;
        for (auto image : group.getEntries(L"zdj*"))
            imageNames.push_back(convert<wstring, string>(image));

        ImageOperations op;

        op.loadReferenceImage(convert<wstring,string>(refimg.front()));
        op.loadVectorOfImages(imageNames);

        op.medianFiltr();
        op.imagesDifference();

        vector<Mat> mats = op.getRecentOperationOnVector(false);

        for (decltype(mats.size()) i = 0; i < mats.size(); i++) {
            namedWindow("window" + i, WINDOW_AUTOSIZE);
            imshow("window" + i, mats.at(i));
        }

        waitKey(); //without this image won't be shown
    }

    sLog.close();
	return 0;
}
