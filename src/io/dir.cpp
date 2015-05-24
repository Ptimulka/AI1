#include "dir.h"
#include <regex>
#include <algorithm>
using namespace std;

#include <Windows.h>
#undef min
#undef max

/*
Note: Windows uses utf16 encoding
*/

Dir::Dir() : myself()
{
}

Dir::Dir(wstring const& dir) : myself(dir)
{
    replace_if(myself.begin(), myself.end(), [](const char& c) -> bool { return c == '\\'; }, '/');
    if (myself.back() != '/')
        myself.append(L"/");
}


DirsList Dir::getWinVolumes()
{
    DWORD mask = GetLogicalDrives();
    DirsList ret;

    char c = 'A';
    for (DWORD i = 0x01; i<mask; i <<= 1, ++c)
        if (mask & i)
            ret.push_back(Dir(c + L":/"));

    return ret;
}

FilesList Dir::getEntries(wstring const& regexp, ListingFlags listing_options) const
{
    if (myself.empty())
        return FilesList();

    auto path = (myself + L"*");
    wregex _regexp(regexp == L"*" ? L".*" : regexp);

    WIN32_FIND_DATAW found_file;
    HANDLE search = FindFirstFileW((LPCWSTR)path.data(), &found_file);
    if (search == INVALID_HANDLE_VALUE)
        return FilesList();

    FilesList ret;
    wstring name;

    do
    {
        if (!listing_options[INCLUDE_DOTS] && wcscmp(found_file.cFileName, L".") == 0)
            continue;
        if (!listing_options[INCLUDE_DOTS] && wcscmp(found_file.cFileName, L"..") == 0)
            continue;
        if (listing_options[EXCLUDE_DIRECTORIES] && found_file.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            continue;
        if (listing_options[EXCLUDE_FILES] && ((found_file.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0))
            continue;
        if (listing_options[EXCLUDE_VISIBLE] && ((found_file.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) == 0))
            continue;
        if (!listing_options[INCLUDE_HIDDEN] && found_file.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)
            continue;
        if (!listing_options[INCLUDE_ENCRYPTED] && found_file.dwFileAttributes & FILE_ATTRIBUTE_ENCRYPTED)
            continue;
        if (!listing_options[INCLUDE_SYSTEM] && found_file.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)
            continue;
        if (listing_options[EXCLUDE_TEMPORARY] && found_file.dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY)
            continue;
        if (listing_options[EXCLUDE_READONLY] && found_file.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
            continue;

        name = move(wstring(&found_file.cFileName[0], lstrlenW(found_file.cFileName)));
        if (!regex_match(name, _regexp))
            continue;

        ret.push_back(myself + name);
    } while (FindNextFileW(search, &found_file));

    FindClose(search);

    return ret;
}

DirsList Dir::getSubDirs(wstring const& regexp, ListingFlags listing_mask) const
{
    listing_mask.add(EXCLUDE_FILES);
    DirsList ret;
    for (auto const& f : getEntries(regexp, listing_mask))
        ret.push_back(f);
    return ret;
}

