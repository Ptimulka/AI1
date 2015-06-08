#include "cmdopt.h"
#include "utils/cmdline.h"
#include "utils/stringlist.h"
using namespace std;

namespace Opts
{ 
    //zmienne z pliku .h z wartoscami domyslnymi (w naglowku tylko deklaracje)
    std::string imgs_dir = "."; //katalog z grupami zdjec (patrz main.cpp)
    std::string imgs_groups_regexp = "default.*"; //interesujaca nas grupa/grupy (patrz main.cpp)
    std::string log_file = "AI.log"; //nazwa logu (patrz main.cpp)
    bool log_file_append = false; //czy dopisujemy (patrz main.cpp)
	bool ann_learn = false; //czy uruchamiamy program aby uczyæ sieæ
	bool ann_tli = false;	//czy uruchamiamy program aby sprawdzic czy dla danych uczacych siec zwraca spoko wyniczki
    unsigned ann_learn_chunk_size = 2 * 1024 * 1024;
    optional<bool> ann_learn_new;
    optional<string> ann_file;
    optional<std::string> ocl_try_compile;

	float ann_accept_threshold = 0.85f;
	unsigned int iw = 50;
	unsigned int ih = 20;
	unsigned int nodes_in_hidden1 = 500;
	unsigned int nodes_in_hidden2 = 0;
	std::string imgs_learn = "fotyUczace";
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

	static CmdOpt ann_learn(
		StringList({ "ann-learning" }),
		"Specifies if ann should learn.\n"
		"Default: false",
		0, 1,
		CMDLINE_ASSIGMENT_HANDLER(Opts::ann_learn));

	static CmdOpt ann_tli(
		StringList({ "ann-tli" }),
		"Specifies if ann should be loaded and give results for learning images.\n"
		"Default: false",
		0, 1,
		CMDLINE_ASSIGMENT_HANDLER(Opts::ann_tli));

    static CmdOpt ann_learn_chunk_size(
        StringList({ "learn-chunk" }),
        "Specifies memory limit for one training chunk.\n"
        "Default: 2KB",
        0, 1,
        CMDLINE_ASSIGMENT_HANDLER(Opts::ann_learn_chunk_size));

    static CmdOpt ann_learn_new(
        StringList({ "new-ann "}),
        "",
        0, 1,
        CMDLINE_ASSIGMENT_HANDLER(Opts::ann_learn_new));

    static CmdOpt ann_file(
        StringList({ "ann" }),
        "",
        1, 1,
        CMDLINE_ASSIGMENT_HANDLER(Opts::ann_file));

    static CmdOpt ocl_try_compile(
        StringList({ "ocl-try-compile", "ocl-compile", "compile" }),
        "",
        0, 1,
        CMDLINE_ASSIGMENT_HANDLER(Opts::ocl_try_compile));

    static CmdOpt ann_accept_threshold(
        StringList({ "ann-threshold" }),
        "",
        0, 1,
        CMDLINE_ASSIGMENT_HANDLER(Opts::ann_accept_threshold));

	static CmdOpt iw(
		StringList({ "iw", "img-width" }),
		"Specifies image width.\n"
		"Default: 20",
		0, 1,
		CMDLINE_ASSIGMENT_HANDLER(Opts::iw));

	static CmdOpt ih(
		StringList({ "ih", "img-height" }),
		"Specifies image height.\n"
		"Default: 14",
		0, 1,
		CMDLINE_ASSIGMENT_HANDLER(Opts::ih));

	static CmdOpt nodes_in_hidden1(
		StringList({ "nodes_in_hidden1", "hidden1", "nodes1", "neurons1" }),
		"Specifies number of neurons in 1 hidden layer.\n"
		"Default: 2KB",
		0, 1,
		CMDLINE_ASSIGMENT_HANDLER(Opts::nodes_in_hidden1));

	static CmdOpt nodes_in_hidden2(
		StringList({ "nodes_in_hidden2", "hidden2", "nodes2", "neurons2" }),
		"Specifies number of neurons in 2 hidden layer.\n"
		"Default: 2KB",
		0, 1,
		CMDLINE_ASSIGMENT_HANDLER(Opts::nodes_in_hidden2));

	static CmdOpt imgs_learn(
		StringList({ "imgs_learn", "imgs" }),
		"Specifies name of directory with images for learning.\n"
		"Default: fotyUczace",
		1, 1,
		CMDLINE_ASSIGMENT_HANDLER(Opts::imgs_learn));

}
