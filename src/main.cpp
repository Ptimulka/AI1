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
#include "ann/cl/oclkernel.h"
#include "ann/fann_irprop_logical.h"

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

    if (Opts::ocl_try_compile.isSet())
    {
        OclKernel _tmp(Opts::ocl_try_compile);
        sLog.close();
        return 0;
    }

    {
        //FanniRPROP xor_test({ 2, 3, 1 });
        //auto* session = xor_test.createLearningSession();
        //xor_test.learn(session, { 0, 1 }, { 1 });
        ////xor_test.learn(session, { 0, 0 }, { 0 });
        //
        ////xor_test.learn(session, { 1, 1 }, { 0 });

        //auto result = xor_test.calc(session, { 1, 0 });
        //sLog.log("1^0 = ", result.back());
        //result = xor_test.calc(session, { 0, 1 });
        //sLog.log("0^1 = ", result.back());
        //result = xor_test.calc(session, { 0, 0 });
        //sLog.log("0^0 = ", result.back());
        //result = xor_test.calc(session, { 1, 1 });
        //sLog.log("1^1 = ", result.back());

        //xor_test.save(ofstream("xor.ann"));
        //sLog.close();
        //return 0;
    }

    //s?�w kilka a propos ?adowania obrazk�w,
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

	int groupNumber = 1;
	cv::ocl::setUseOpenCL(false);


    for (Dir group : imgs_dir.getSubDirs()) //pobieramy liste podkatalog i przegladamy ja (patrz: C++ 11 range-based for)
    {
        if (!regex_match(group.name(), group_regexp)) //sprawdzamy czy nazwa podkatalogu pasuje do nazwy grupy podanej jako wyrazenie regularne (Opts::imgs_groups_regexp)
            continue; //jesli nie to pomijamy

        //jesli tak to pobieramy liste plikow/katalogow ktore pasuje do wyrazenie 'ref.*' (patrz obrazek referecyjny wy?ej)
		//pozwolilam to sobie zakomentowac, bo niepoczebny referencyjny :)
        //auto refimg = group.getEntries(L"ref.*");
        //if (refimg.empty()) //nie znaleziono nic
        //{
        //    sLog.log("Group ", group.path(), " doesn't have reference image in it!");
        //    continue;
        //}
        //if (refimg.size() > 1) //znaleziono wiecej niz jeden
        //{
        //    sLog.log("More than one reference image found in group ", group.path());
        //    continue;
        //}

        //oraz ladujemy liste zdjec

		//zrobilam 2 petle bo nie wiem jak zrobic lub w �ukasza regex
        vector<string> imageNames;
        for (auto image : group.getEntries(L"MWSnap*"))
            imageNames.push_back(convert<wstring, string>(image));

		for (auto image : group.getEntries(L"zdj*"))
			imageNames.push_back(convert<wstring, string>(image));


        ImageOperations op;


        //ktore razem z obrazkiem referencyjnym ladujemy do ImageOperations
        //op.loadReferenceImage(convert<wstring,string>(refimg.front()));	//nie ma juz tego, w ogole nie bedziemy brali od uwage obrazka referencyjnego!
        
		//jesli loadVectorOfImages zwraca NIE zero to znaczy ze blad
		ImageOperations::ImagesErrors loaded = op.loadVectorOfImages(imageNames);
		if (loaded != op.OK) {
			//mozemy sprawdzic blad np:
			if (loaded == ImageOperations::DIFFERENT_SIZES) {
				sLog.log("obrazki maja rozne wymiary!\n");
				return 1;
			}
		}

		int poczatek = clock();
		
        //z ktorymi cos potem robimy (to juz nie moje ;d)

		
		std::vector<int> threshes;
		threshes.push_back(10);
		threshes.push_back(20);
		threshes.push_back(30);
		threshes.push_back(40);
		op.addRectsWithOptions(7, 3, threshes);
		op.markAllPossibleCars();
		

		vector<UMat> mats = op.getLoadedImages();
		string windowName = "window";

		int czas = clock() - poczatek;

		sLog.log("Tyle czasu zajelo przetwarzanie obrazkow: ", czas);

		std::vector<std::vector<Mat>> allRects = op.getMatsScaledTo(50, 30);


		//po kolei dla ka�dego obrazka pobieramy 
		for (decltype(allRects.size()) i = 0; i < allRects.size(); i++) {
			for (decltype(allRects[i].size()) j = 0; j < allRects.size(); j++) {
				std::vector<uchar> array;
				array.assign(allRects[i][j].datastart, allRects[i][j].dataend);
				std::vector<double> arrayOfDoubles(array.size());
				for (decltype(array.size()) it = 0; it < array.size(); it++)
					arrayOfDoubles[it] = array[it];			

				//tu arrayOfDoubles jako wejsce sieci
				
			}
		}

		//aby obczai� obrazki te niby gotowe na sie� neuronow� trzeba da� tu breakpointa i przejrze� wektor wektor�w allRects!!!

        for (decltype(mats.size()) i = 0; i < mats.size(); i++) {
			//nie pokazuje bo bajzyl sie robi na ekranie
			//namedWindow(windowName + std::to_string(groupNumber) + "_" + std::to_string(i), WINDOW_AUTOSIZE);
			//imshow(windowName + std::to_string(groupNumber) + "_" + std::to_string(i), mats[i]);

	
			std::string gdzie = "./markedCars/obr" + std::to_string(groupNumber) + "_" + std::to_string(i) + ".jpg";
			imwrite(gdzie, mats[i]);

        }

        //waitKey(); //without this image won't be shown

		groupNumber++;
        //tutaj petla sie konczy, czyli sprawdzamy nastepna grupe
    }

    //po sprawdzeniu wszystkich grup zamykamy log (jest to wymagane, bo Log wypisuje informacje asynchronicznie co 1s, wiec
    //musimy mu dac czas, aby sprawdzil, czy cos jeszcze nie zostalo do wypisania itd.)
    //  w praktyce - Log::close ustawia flage run na false po czym nwywoluje funkcje thread::join na watku
    //  odpowiedzialnym za wypisywanie informacji. Watek ten co 1s sie budzi i sprawdza, czy run==true i wypisuje
    //  zawartosc kolejki powiazanej z logiem i znowu zasypia. jesli run==false, konczy prace.
    sLog.close();
	return 0;
}
