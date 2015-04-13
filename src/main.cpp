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

    //Log pozwala nam na logowanie rzeczy, w szczegolnosci - wypisywanie informacji do wielu miejsc na raz
    // nowe cele dodajemy tak, jak ponizej (wskaznik na std::ostream), domyslnie wypisujemy do konsoli
    sLog.addOutput(&cout);

    try
    {
        //CmdLine pozwala nam ?atwo przetworzy? argumenty podane programowani przy uruchomieniu
        // lista dostepnych argumentow jest zdefiniowana w cmdopt.cpp/.h
        // CmdLine moze rzucic wyjatkiem, jesli podane argumenty sa niepoprawne (np. cos wystepuje dwa razy
        // a powinno tylko raz, wiec mamy try catcha)
        CmdLine::singleton().parse(argv, argc);
    }
    catch (exception& e)
    {
        sLog.log("Error: ", e.what());
        system("pause");
        return 1;
    }

    //po parsowaniu argumentow odpowiednie zmienne w przestrzeni Opts (patrz cmdopt.cpp/.h)
    //maja ustawiona odpwiednia wartosc (domyslna lub, jesli argument zostal podany, podana przez uzytkownika)

    //pierwsza rzecza, jaka chcemy zrobic, to dodac wyjscie do pliku dla naszego logu (obok konsoli)
    // chodzi o to, ze jak duzo tekstu przelatuje przez konsole, to lubi sie gubic, a w pliku zawsze mozna pozniej znalezc wszystko
    //na poczatku sprawdzamy, czy - jesli plik z logiem juz istnieje - mamy go czyscic, czy dopisywac do niego
    //za to odpowiedzialna jest zmienna Opts::log_file_append => true - dopisujemy, false - czyscimy
    auto flags = (Opts::log_file_append ? ios::out | ios::app : ios::out | ios::trunc);

    //teraz staramy sie uzyskac dostep do pliku (lub go utworzyc, jesli nie istnieje) wskazanego przez Opts::log_file
    ofstream out(Opts::log_file, flags);
    if (!out.is_open()) //sprawdzamy, czy wystapil blad - jesli tak wypisujemy (tylko do konsoli) komunikat
        sLog.log("Could not open file ", Opts::log_file, " for logging - log output will only be pushed to console.");
    else //jesli nie, to dodajemy dodatkowe wyj?cie loga do pliku
        sLog.addOutput(&out);

    //s?ów kilka a propos ?adowania obrazków,
    // obrazki dzielimy na grupy/paczki/itd. idea jest taka, zeby kazda taka paczka
    // stanowila osobny przypadek - np. osobne skrzyzowanie
    // grupy istnieja de facto jako foldery, zawierajace pliki ze zdjeciami,
    // umieszczone w katalogu wskazanym przez opcje Opts::imgs_dir
    // Grupa moze byc grupa jesli:
    //  - jej katalog znajduje sie w katalogu Opts::imgs_dir (j.w.)
    //  - w katalogu znajduje sie zdjecie referencyjne o nazwie ref.*
    //    gdzie gwiazdka to dowolne rozszerzenie (naprawd? dowolne, wiec
    //    txt tez przejdzie - trzeba na to uwazac)
    //  - wszystkie inne zdjecia, jakie chcemy brac pod uwage, powinny miec nazwe
    //    rozpoczynajaca sie od zdj

    //aby wszystko bylo nieco bardziej praktyczne, nasza aplikacja
    //ma mozliwosc wskazania, ktore grupy powinna przetworzyc (kazda osobna)
    //stosujemy do tego wyrazenie regularne utworzone z przekazanego parametru
    // Opts::imgs_groups_regexp
    // domyslnie jego wartosc to 'default', czyli interesuje nas tylko grupa o takiej
    // konkretnie nazwie
    //uzywamy wstring/wregex, aby umozliwic nazwy plikow z niestandardowymi znakami
    wregex group_regexp(convert<string,wstring>(Opts::imgs_groups_regexp));

    //Opts::imgs_dir to argument wskazujacy katalog z grupami zdjec,
    //przeszukujemy go uzywajac klasy Dir (ponownie uzywamy wstring)
    Dir imgs_dir(convert<string, wstring>(Opts::imgs_dir));
    for (Dir group : imgs_dir.getSubDirs()) //pobieramy liste podkatalog i przegladamy ja (patrz: C++ 11 range-based for)
    {
        if (!regex_match(group.name(), group_regexp)) //sprawdzamy czy nazwa podkatalogu pasuje do nazwy grupy podanej jako wyrazenie regularne (Opts::imgs_groups_regexp)
            continue; //jesli nie to pomijamy

        //jesli tak to pobieramy liste plikow/katalogow ktore pasuje do wyrazenie 'ref.*' (patrz obrazek referecyjny wy?ej)
        auto refimg = group.getEntries(L"ref.*");
        if (refimg.empty()) //nie znaleziono nic
        {
            sLog.log("Group ", group.path(), " doesn't have reference image in it!");
            continue;
        }
        if (refimg.size() > 1) //znaleziono wiecej niz jeden
        {
            sLog.log("More than one reference image found in group ", group.path());
            continue;
        }

        //oraz ladujemy liste zdjec
        vector<string> imageNames;
        for (auto image : group.getEntries(L"zdj*"))
            imageNames.push_back(convert<wstring, string>(image));

        ImageOperations op;

        //ktore razem z obrazkiem referencyjnym ladujemy do ImageOperations
        op.loadReferenceImage(convert<wstring,string>(refimg.front()));
        op.loadVectorOfImages(imageNames);

        //z ktorymi cos potem robimy (to juz nie moje ;d)
        op.medianFiltr(op.ALL, 11);
        op.imagesDifference();
		op.threshold(op.VECTOR_OF_IMAGES);

        vector<Mat> mats = op.getRecentOperationOnVector(false);

		string windowName = "window";

        for (decltype(mats.size()) i = 0; i < mats.size(); i++) {
			namedWindow(windowName + std::to_string(i), WINDOW_AUTOSIZE);
			imshow(windowName + std::to_string(i), mats.at(i));
        }

        waitKey(); //without this image won't be shown

        //tutaj petla sie konczy, czyli sprawdzamy nastepna grupe
    }

    //po sprawdzeniu wszystkich grup zamykamy log (jest to wymagane, bo Log wypisuje informacje asynchronicznie co 1s, wiec
    //musimy mu dac czas, aby sprawdzil, czy cos jeszcze nie zostalo do wypisania itd.)
    //  w praktyce - Log::close ustawia flage run na false po czym wywoluje funkcje thread::join na watku
    //  odpowiedzialnym za wypisywanie informacji. Watek ten co 1s sie budzi i sprawdza, czy run==true i wypisuje
    //  zawartosc kolejki powiazanej z logiem i znowu zasypia. jesli run==false, konczy prace.
    sLog.close();
	return 0;
}
