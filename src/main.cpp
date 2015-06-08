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
#include "ann/ann.h"
#include "annlearn.h"


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
        sLog.close();
        system("pause");
        return 1;
    }
    
    auto flags = (Opts::log_file_append ? ios::out | ios::app : ios::out | ios::trunc);

    ofstream out(Opts::log_file, flags);
    if (!out.is_open()) 
        sLog.log("Could not open file ", Opts::log_file, " for logging - log output will only be pushed to console.");
    else 
        sLog.addOutput(&out);


    if (Opts::ocl_try_compile.isSet())
    {
        OclKernel _tmp(Opts::ocl_try_compile);
        sLog.close();
        return 0;
    }
    


	//uint iw = 50, ih = 20;	//image width, height
	const char *filename = "test50x20.ann";	//pod jak¹ nazw¹ zapisaæ nauczon¹ sieæ

	///////------UCZENIE SIECIUNI!!!!---------\\\\\\\\	
	if (Opts::ann_learn) {	
        //-ann-learning
		ann_learn(Opts::iw, Opts::ih, filename, Opts::nodes_in_hidden1, Opts::nodes_in_hidden2);	//wysokosæ, szerokoœæ, nazwa pliku w którym zapisaæ sieæ, iloœæ neuronów w 1. warstwie ukrytej, potem 2. ukrytej - domyœlnie 0
		sLog.close();
		return 0;
	}

	////////   TUTAJ SPRAWDZANKO CZY NAUCZONA SIEC ZWRACA SPOKO WYNIKI DLA DANYCH UCZACYCH!!!
	if (Opts::ann_tli) {
        //-ann-tli
		ann_test_learning_images(Opts::iw, Opts::ih, filename, 0.85);
		sLog.close();
		return 0;
	}

	///////------TUTAJ NIE UCZENIE SIECIUNI!!!!---------\\\\\\\\

    if (!Opts::ann_file.isSet())
    {
        sLog.log("Ann file not specified - using '.*\\.ann'");
        Opts::ann_file = string(".*\\.ann");
    }

    uint ann_idx = 0;
    std::list<ArtificialNeuralNetwork> anns;
    auto entries = Dir(L"anns").getEntries(convert<string, wstring>(Opts::ann_file));
    for (auto& file : entries)
    {
        ArtificialNeuralNetwork ann;
        ann.init<ArtificialNeuralNetwork::FannDriver>(convert<wstring,string>(file).c_str(), 0u);

        int width, height;
        istringstream ss(convert<wstring, string>(file.substr(file.rfind('/')+1)));
        string tmp;
        getline(ss, tmp, '.'); width = parse<int>(tmp);
        getline(ss, tmp, '.'); height = parse<int>(tmp);
        ann.setInputSize(width, height);

        anns.push_back(ann);
        sLog.log("Loaded ann: ", file, " ", ++ann_idx, "/", entries.size());
    }

    wregex group_regexp(convert<string,wstring>(Opts::imgs_groups_regexp));
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
			}
			sLog.close();
			return 1;
		}

        sLog.log("Group ", group.name(), " (", imageNames.size(), " images)");
		
        //szukamy miejsc podejrzanych z roznymi parametrami
		std::vector<int> threshes = { 10, 20, 30, 40 };
		op.addRectsWithOptions(7, 3, threshes);
		op.markAllPossibleCars();

		//obrazki z zaznaczonymi podejrzanymi miejscami zapisujemy do pliku
		vector<UMat> mats = op.getLoadedImagesWithPossibleCars();
		for (decltype(mats.size()) i = 0; i < mats.size(); i++) {
			std::string gdzie = "./markedCars/obr" + std::to_string(groupNumber) + "_" + std::to_string(i) + ".jpg";
			imwrite(gdzie, mats[i]);
		}

		//teraz czas na ann!!!
        for (int i = 0; i < op.getImagesCount(); i++) {

            sLog.log("Testing image: ", i+1, "/", op.getImagesCount(), ", ", op.getRectsCountForImage(i), " possible rects");
            for (int j = 0; j < op.getRectsCountForImage(i); j++) {

				std::vector<float> array;
                float result = 0.0;
                for (auto& ann : anns)
                {
                    auto scaled = op.getMatScaledTo(i, j, ann.getInputWidth(), ann.getInputHeight());
                    array.assign(scaled.datastart, scaled.dataend);
                    result = max(result, ann.run(array).front());
                }

				//jeœli odpowiedŸ sieci jes na tyle duza aby uznaæ prostok¹t za samochód, to ustawiamy ¿e to jest samochód
				if (result > Opts::ann_accept_threshold) {
					op.setRectAsCar(i, j);
				}
			}			
		}


		//teraz zaznaczamy te juz co s¹ ustawione jako samochody
		op.markRealCars();
		
		//zapisujemy obrazu z zielonymi zaznaczonymi do pliku
		vector<UMat> mats2 = op.getLoadedImagesWithPossibleCars();
		for (decltype(mats2.size()) i = 0; i < mats2.size(); i++) {
			std::string gdzie = "./markedCarsAnn/obr" + std::to_string(groupNumber) + "_" + std::to_string(i) + ".jpg";
			imwrite(gdzie, mats2[i]);
		}


		groupNumber++;
        //tutaj petla sie konczy, czyli sprawdzamy nastepna grupe
    }

    sLog.close();
	return 0;
}
