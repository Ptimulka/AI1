#ifndef MIRROR_DIRECTORY_HEADER
#define MIRROR_DIRECTORY_HEADER

#include "utils/flag.h"
#include "utils/stringlist.h"

class Dir;
typedef std::list<Dir> DirsList;

class Dir
{
public:
    enum ListingFlag
    {
        EXCLUDE_VISIBLE = 0x001,
        INCLUDE_HIDDEN = 0x002,
        INCLUDE_ENCRYPTED = 0x004,
        INCLUDE_SYSTEM = 0x008,
        EXCLUDE_TEMPORARY = 0x010,
        EXCLUDE_READONLY = 0x020,

        EXCLUDE_DIRECTORIES = 0x040,
        EXCLUDE_FILES = 0x080,
        INCLUDE_DOTS = 0x100,

        ALL = INCLUDE_HIDDEN | INCLUDE_ENCRYPTED | INCLUDE_SYSTEM,
        ALL_FILES = ALL | EXCLUDE_DIRECTORIES,
        ALL_DIRS = ALL | EXCLUDE_FILES,

        FILES = EXCLUDE_DIRECTORIES,
        DIRS = EXCLUDE_FILES
    };

    typedef Flag<unsigned int> ListingFlags;

    // ^
    //maska rowna 0 (domyslna) bedzie zawierala:
    // widoczne,
    // nie szyfrowane,
    // nie systemowe,
    //pliki, zarowno tymczasowe jak i przeznaczone jedynie do odczytu

public:
    Dir();
    Dir(std::wstring const& dir);

    static DirsList getWinVolumes();

    FilesList getEntries(std::wstring const& regexp = L"*", ListingFlags listing_mask = 0) const;
    DirsList getSubDirs(std::wstring const& regexp = L"*", ListingFlags listing_mask = 0) const;

    inline std::wstring path() const                    { return myself; }
    inline std::wstring name() const                    { auto ret = myself.substr(myself.rfind('/', 1)+1); ret.pop_back(); return ret; }

private:
    std::wstring myself;
};

#endif //MIRROR_DIRECTORY_HEADER
