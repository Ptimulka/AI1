#pragma once
#include "cmdopt.h"
#include <qstring.h>

namespace Opts
{
	//zmienne, ktore przechowuja informacje zwiazane z argumentami
	// jesli jaki argument jest podany przy uruchomieniu programu,
	// jego wartosc zostanie przypisana odpowiedniej zmiennej (jesli jest poprawna)
	// jestli argumentu nie ma, wykorzystana jest wartosc domyslna

	extern QString imgs_dir; //="."
	extern QString imgs_groups_regexp; //="default"
	extern QString log_file; //="AI.log"
	extern bool log_file_append; //=false
	extern bool ann_learn; //=false
	extern bool ann_tli; //=false
	extern unsigned ann_learn_chunk_size; //=2KB

	extern float ann_accept_threshold;

	extern QString marked_cars_path;
	//extern std::string ann_learn_file;
	extern unsigned int iw;
	extern unsigned int ih;
	extern unsigned int nodes_in_hidden1;
	extern unsigned int nodes_in_hidden2;
	extern QString  imgs_learn;
}

