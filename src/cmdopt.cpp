#include "cmdopt.h"
#include "utils/cmdline.h"
#include "utils/stringlist.h"
using namespace std;

namespace Opts
{ 
    //zmienne z pliku .h z wartoscami domyslnymi (w naglowku tylko deklaracje)
    std::string imgs_dir = "."; //katalog z grupami zdjec (patrz main.cpp)
    std::string imgs_groups_regexp = "default"; //interesujaca nas grupa/grupy (patrz main.cpp)
    std::string log_file = "AI.log"; //nazwa logu (patrz main.cpp)
    bool log_file_append = false; //czy dopisujemy (patrz main.cpp)
}


namespace
{ 
    //te zmienne globalne de facto umozliwaja nam ladowanie opcji z linii polecen
    // stanowia polaczenie miedzy tym, co zostanie wpisane przez uzytkownika, a zmiennymi reprezentujacymi nasze opcje (te z Opts powyzej)
    // dokladna zasada dzialania jest dosyc dluga (duzo wywolan itd.) a jednoczesnie raczej prosta (sprowadza sie do podzialu
    // argumentow na pary -nazwa=wartosc i wywolywanie odpowiednich funkcji, ktore parsuja i staraja sie przypisac wartosc do odpowiedniej zmiennej
    //
    // opcja skada sie z:
    //  - listy nazw, ktore ja identyfikuja
    //  - opisuj
    //  - minimalnej liczby argumentow (CmdLine rzuca wyjatek, jesli jakas opcja pojawila sie mniej)
    //  - maksymalnej liczby argumentow (j.w.)
    //  - dzialania, jakie ma byc wykonane w chwili znalezienia opcji (tutaj wykorztsujemy tylko CMD_ASSIGMENT_HANDLER, ktory przypisuje
    //    podanej zmiennej odpowiednia wartosc

    static CmdOpt imgs_dir(
        StringList({ "imgs-dir", "idir" }),
        "Specifies directory which contains grouped images to be used by application.\n"
        "All groups should have theirs' folder with name matching group's name.\n"
        "To select which group should be considered use -img-group option.\n\n"
        "Default value: . (current dir)",
        1, 1,
        CMDLINE_ASSIGMENT_HANDLER(Opts::imgs_dir));

    static CmdOpt imgs_groups_regexp(
        StringList({ "img-group", "igr" }),
        "Specifies which images groups should be used within application using regular expression to be matched agains group's name.\n"
        "Default value: default",
        1, 1,
        CMDLINE_ASSIGMENT_HANDLER(Opts::imgs_groups_regexp));

    static CmdOpt log_file(
        StringList({ "log-file", "lf", "log", "lfile" }),
        "Specifies log file.\n"
        "Default: AI.log",
        1, 1,
        CMDLINE_ASSIGMENT_HANDLER(Opts::log_file));

    static CmdOpt log_file_append(
        StringList({ "log-append" }),
        "Specifies if logger should append messeges to existing file rather then truncating it.\n"
        "Default: false",
        0, 1,
        CMDLINE_ASSIGMENT_HANDLER(Opts::log_file_append));
}