#pragma once

#include <string>

namespace Opts
{
    //zmienne, ktore przechowuja informacje zwiazane z argumentami
    // jesli jaki argument jest podany przy uruchomieniu programu,
    // jego wartosc zostanie przypisana odpowiedniej zmiennej (jesli jest poprawna)
    // jestli argumentu nie ma, wykorzystana jest wartosc domyslna

    extern std::string imgs_dir; //="."
    extern std::string imgs_groups_regexp; //="default"
    extern std::string log_file; //="AI.log"
    extern bool log_file_append; //=false
}

