#ifdef _DEBUG
#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <string>
#include <vector>
#include <iostream>
using namespace std;

//ta funkcja to taki ma?y hack,
//pozwala na wykrycie, czy program zosta? uruchomiony przy u?yciu debuggera i je?li tak,
//to pozwala na nadpisanie argumentów wywo?ania programu (chodzi o to, ze je?li debuggujemy, to argumenty trzeba
// zmieniac za kazdym razem w opcjach projektu - komu by sie chcialo?)
//  dzia?anie jest dosy? toporne: wczytujemy lini? tekstu - je?li jest pusta, to nic nie robimy (zostawiamy jak jest)
//  je?li nie to rozbijamy j? na wyrazy (szukamy spacji) i zapisujemy pó?niej wszystko we wskazanej tablicy (argv)
void OverrideArgumentInDebuggin(int& argc, char**& argv)
{
    if (!IsDebuggerPresent())
        return;

    cout << "Debugger dettected! You can override application arguments now (or press enter to continue)\n"
        << "Current arguments\n";
    for (int i = 1; i < argc; ++i)
        cout << argv[i] << '\n';
    cout << "> ";

    string new_args = "";
    getline(cin, new_args);
    if (new_args.empty())
        return;

    unsigned int words = count(new_args.begin(), new_args.end(), ' ') + 1;

    vector<char*> tmp;
    tmp.push_back(new char[strlen(argv[0]) + 1]);
    strcpy(tmp[0], argv[0]);

    while (!new_args.empty())
    {
        auto pos = new_args.find(' ');
        if (pos == new_args.npos)
            pos = new_args.size();

        tmp.push_back(new char[pos + 1]);
        strncpy(tmp.back(), new_args.c_str(), pos);
        tmp.back()[pos] = 0;
        try
        {
            new_args = new_args.substr(pos + 1);
        }
        catch (out_of_range&)
        {
            break;
        }
    }

    char** new_buff = new char*[tmp.size()];
    for (unsigned int i = 0; i < tmp.size(); ++i)
        new_buff[i] = tmp[i];

    argc = tmp.size();
    argv = new_buff;
}

#endif
