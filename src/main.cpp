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
#include "ann/ann.h"


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
        sLog.close();
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

    /*{
		ArtificialNeuralNetwork ann({ 2, 3, 1 });
        ann.init<ArtificialNeuralNetwork::FannDriver>(3u);
        
        vector<float> inputs = { 0.0, 0.0, 0.0, 1.0, 1.0, 1.0 };
        vector<float> results = { 0.0, 1.0, 0.0 };
        ann.learn(inputs, results);
        ann.saveNative("default1.ann");
        
        ArtificialNeuralNetwork ann2;
        ann2.init<ArtificialNeuralNetwork::FannDriver>("default1.ann", 0u);

        auto r1 = ann.run({ 1.0, 0.0 }).front();
        auto r2 = ann2.run({ 1.0, 0.0 }).front();

        sLog.close();
        return 0;
    }*/

	///////------UCZENIE SIECIUNI!!!!---------\\\\\\\\

	uint iw = 20, ih = 14;	//image with, height

	if (Opts::ann_learn) {

	    const unsigned bytesPerPixel = sizeof(float);
		const unsigned bytesPerCase = iw * ih * bytesPerPixel;
		if (Opts::ann_learn_chunk_size < bytesPerCase)
        {
            sLog.log("Error: one trainging chunk isn't capable of holding one input image!");
            sLog.close();
            return 1;
        }

		const unsigned casesPerRun = Opts::ann_learn_chunk_size / bytesPerCase;

		//ArtificialNeuralNetwork ann({ iw*ih, 36, 36*4, 1 });
		ArtificialNeuralNetwork ann({ iw*ih, iw*ih+9,  1 });	//takie sobie parametry dalam z sufitu, bo chialam wyprobowac i tak zostalo xD
		ann.init<ArtificialNeuralNetwork::FannDriver>(casesPerRun);

		Dir imgs_dir(convert<string, wstring>("fotyUczace"));
		std::vector<string> pathsPos;
		std::vector<string> pathsNeg;

		for (auto image : imgs_dir.getEntries(L"pos.*"))
			pathsPos.push_back(convert<wstring, string>(image));

		for (auto image : imgs_dir.getEntries(L"neg.*"))
			pathsNeg.push_back(convert<wstring, string>(image));

		

		ImageOperations op;

		std::vector<float> array;
		array.reserve(Opts::ann_learn_chunk_size);
		std::vector<float> results;

		op.loadVectorOfImagesToLearn(pathsPos);
		std::vector<Mat> learn_pos = op.getLearningImagesScaledTo(iw, ih);	//szerokosæ, wysokoœæ
		op.loadVectorOfImagesToLearn(pathsNeg);
		std::vector<Mat> learn_neg = op.getLearningImagesScaledTo(iw, ih);

		vector<pair<bool, uint>> tests;
		for (uint i = 0; i < learn_pos.size(); ++i)
			tests.push_back(make_pair(true, i));
		for (uint i = 0; i < learn_neg.size(); ++i)
			tests.push_back(make_pair(false, i));
		std::random_shuffle(tests.begin(), tests.end());

		results.reserve(casesPerRun);

		for (uint i = 0; i < tests.size(); ++i)
		{
			int j = 0;
			for (; j < casesPerRun && i < tests.size(); ++j, ++i)
				if (tests[i].first)
				{
					array.insert(array.end(), learn_pos[tests[i].second].datastart, learn_pos[tests[i].second].dataend);
					results.push_back(1.0f);
				}
				else
				{
					array.insert(array.end(), learn_neg[tests[i].second].datastart, learn_neg[tests[i].second].dataend);
					results.push_back(0.0f);
				}

			ann.learn(array, results);
			array.clear();
			results.clear();
		}

		ann.saveNative("test.ann");

		sLog.close();
		return 0;
	}


	//////   TUTAJ SPRAWDZANKO CZY NAUCZONA SIEC ZWRACA SPOKO WYNIKI DLA DANYCH UCZACYCH!!!

	if (Opts::ann_tli) {

		Dir imgs_dir(convert<string, wstring>("fotyUczace"));
		std::vector<string> pathsPos;
		std::vector<string> pathsNeg;

		for (auto image : imgs_dir.getEntries(L"pos.*"))
			pathsPos.push_back(convert<wstring, string>(image));

		for (auto image : imgs_dir.getEntries(L"neg.*"))
			pathsNeg.push_back(convert<wstring, string>(image));


		ImageOperations op;
		op.loadVectorOfImagesToLearn(pathsPos);
		std::vector<Mat> learn_pos = op.getLearningImagesScaledTo(iw, ih);	//szerokosæ, wysokoœæ
		op.loadVectorOfImagesToLearn(pathsNeg);
		std::vector<Mat> learn_neg = op.getLearningImagesScaledTo(iw, ih);

		ArtificialNeuralNetwork ann;
		ann.init<ArtificialNeuralNetwork::FannDriver>("test.ann", 0u);

		std::vector<float> results_pos;
		std::vector<float> results_neg;

		for (uint i = 0; i < learn_pos.size(); i++) {
			std::vector < float > input_array;
			input_array.assign(learn_pos[i].datastart, learn_pos[i].dataend);
			float answer = ann.run(input_array).front();
			results_pos.push_back(answer);
			continue;
		}

		for (uint i = 0; i < learn_neg.size(); i++) {
			std::vector < float > input_array;
			input_array.assign(learn_neg[i].datastart, learn_neg[i].dataend);
			float answer = ann.run(input_array).front();
			results_neg.push_back(answer);
			continue;
		}

		//tu breakpoint i sprawdzamy czy results_pos i results_neg sa odpowiednio bliskie 1 albo bliskie 0!
		sLog.close();
		return 0;

	}

	///////------TUTAJ NIE UCZENIE SIECIUNI!!!!---------\\\\\\\\

    if (!Opts::ann_file.isSet())
    {
        sLog.log("Ann file not specified - using 'default.ann'");
        Opts::ann_file = string("default.ann");
    }

    ArtificialNeuralNetwork ann;
    ann.init<ArtificialNeuralNetwork::FannDriver>("test.ann", 0u);

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

	int groupNumber = 1;
	cv::ocl::setUseOpenCL(false);		//czy opencv ma korzystac z opencl

    for (Dir group : imgs_dir.getSubDirs()) //pobieramy liste podkatalog i przegladamy ja (patrz: C++ 11 range-based for)
    {
        if (!regex_match(group.name(), group_regexp)) //sprawdzamy czy nazwa podkatalogu pasuje do nazwy grupy podanej jako wyrazenie regularne (Opts::imgs_groups_regexp)
            continue; //jesli nie to pomijamy     

        //ladujemy liste zdjec
        vector<string> imageNames;
        for (auto image : group.getEntries(L"(MWSnap|zdj).*"))
            imageNames.push_back(convert<wstring, string>(image));

        ImageOperations op;
		//jesli loadVectorOfImages nie zwraca OK to znaczy ze blad
		ImageOperations::ImagesErrors loaded = op.loadVectorOfImages(imageNames);
		if (loaded != op.OK) {
			//mozemy sprawdzic blad np:
			if (loaded == ImageOperations::DIFFERENT_SIZES) {
				sLog.log("obrazki maja rozne wymiary!\n");
				sLog.close();
				return 1;
			}
		}

		int poczatek = clock();
		
        //szukamy miejsc podejrzanych z roznymi parametrami
		std::vector<int> threshes = { 10, 20, 30, 40 };
		op.addRectsWithOptions(7, 3, threshes);
		op.markAllPossibleCars();
		
		int czas = clock() - poczatek;
		sLog.log("Tyle czasu zajelo przetwarzanie obrazkow: ", czas);

		//obrazki z zaznaczonymi podejrzanymi miejscami zapisujemy
		vector<UMat> mats = op.getLoadedImagesWithPossibleCars();
		for (decltype(mats.size()) i = 0; i < mats.size(); i++) {
			std::string gdzie = "./markedCars/obr" + std::to_string(groupNumber) + "_" + std::to_string(i) + ".jpg";
			imwrite(gdzie, mats[i]);
		}



		//teraz czas na ann!!!
		std::vector<std::vector<Mat>> allRects = op.getMatsScaledTo(iw, ih);	//pobieramy obrazki ju¿ przystosowane na wejscie, o takim rozmiarze

		//aby obczaiæ obrazki te niby gotowe na sieæ neuronow¹ trzeba daæ tu breakpointa i przejrzeæ wektor wektorów allRects!!!

		//po kolei dla ka¿dego obrazka obczajamy 
		for (decltype(allRects.size()) i = 0; i < allRects.size(); i++) {

			vector<float> results;
			for (decltype(allRects[i].size()) j = 0; j < allRects[i].size(); j++) {

				std::vector<float> array;
				array.assign(allRects[i][j].datastart, allRects[i][j].dataend);

                auto result = ann.run(array).front();
				results.push_back(result);

				//odpowiedŸ sieci
				if (result > Opts::ann_accept_threshold) {
					op.setRectAsCar(i, j);
				}
			}
			
			continue;
		}


		//teraz zaznczamy te juz co s¹ samochodami
		op.markRealCars();


		//zapisujemy
		vector<UMat> mats2 = op.getLoadedImagesWithPossibleCars();
		for (decltype(mats2.size()) i = 0; i < mats2.size(); i++) {
			std::string gdzie = "./markedCarsAnn/obr" + std::to_string(groupNumber) + "_" + std::to_string(i) + ".jpg";
			imwrite(gdzie, mats2[i]);
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
