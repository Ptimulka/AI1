#include <qstring.h>

namespace Opts
{
	//zmienne z pliku .h z wartoscami domyslnymi (w naglowku tylko deklaracje)
	QString imgs_dir = "."; //katalog z grupami zdjec (patrz main.cpp)
	QString imgs_groups_regexp = "default.*"; //interesujaca nas grupa/grupy (patrz main.cpp)
	QString log_file = "AI.log"; //nazwa logu (patrz main.cpp)
	bool log_file_append = false; //czy dopisujemy (patrz main.cpp)
	bool ann_learn = false; //czy uruchamiamy program aby uczyæ sieæ
	bool ann_tli = false;	//czy uruchamiamy program aby sprawdzic czy dla danych uczacych siec zwraca spoko wyniczki
	unsigned ann_learn_chunk_size = 2 * 1024 * 1024;
	
	float ann_accept_threshold = 0.85f;
	QString marked_cars_path = "./markedCarsAnn";
	//std::string ann_learn_file = "test.ann";
	unsigned int iw = 50;
	unsigned int ih = 20;
	unsigned int nodes_in_hidden1 = 500;
	unsigned int nodes_in_hidden2 = 0;
	QString imgs_learn = ".";
}
