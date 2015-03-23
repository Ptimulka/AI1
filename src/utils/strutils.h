#ifndef SHARED_STR_UTILS_H
#define SHARED_STR_UTILS_H

//plik z przydatnymi funkcja do operacji na napisach
// kopiowany od jednego projektu do drugiego :P
// najchetniej wstawilbym to do biblioteki standardowej, no ale coz...

#include <string>
#include <sstream>
#include <ctime>
#include <chrono>
#include <vector>

#pragma warning(disable: 4996)

//kopiuje napis typu FROM na napis typu TO
//de facto do konwersji miedzy string i wstring
template <typename FROM, typename TO>
inline TO convert(FROM const& str)
{
    return TO().assign(str.begin(), str.end());
}

//anonimowy namespace - funkcje niewiczone poza tym plikiem, wykorzystywane pozniej przez inne
//ta konkretna grupa funkcji zwiazana jest z funkcja mkstr, ktora tworzy stringa z podanych argumentow
namespace
{
    //domyslna struktura udostepniajace funkcje dodajaca argument typu T do strumienia
    // (domyslne dla dowolnego T)
    template <typename T>
    struct _Writer
    {
        static inline std::ostream& push(std::ostream& out, T const& t) { return out << t; }
    };

    // wyspecjalizowana forma powyzszej struktyr
    // jesli wypisywanym typem jest vector elementow typu T
    // to odpowiednio formatujemy wyjscie
    // dla kazdego elementu vectora wywolujemy _Writer<T>::push (jakby-rekurencja)
    template <typename T>
    struct _Writer<std::vector<T>>
    {
        static inline std::ostream& push(std::ostream& out, std::vector<T> const& t)
        {
            out << "[";
            for (decltype(ts.size()) i = 0; i < ts.size(); ++i)
            {
                _Writer<T>::push(out, ts[i]);
                if (i + 1 < ts.size())
                    out << ", ";
            }
            out << "]";
            return out;
        }
    };

    //kolejny wyspecjalizowany typ - std::chrono::system_clock::time_point
    //formatujemy jako dzien miesiac rok (czy jakos tak, nie pamietam xD)
    template <>
    struct _Writer<std::chrono::system_clock::time_point>
    {
        static inline std::ostream& push(std::ostream& out, std::chrono::system_clock::time_point const& tp)
        {
            std::time_t tt;
            tt = std::chrono::system_clock::to_time_t(tp);

            struct tm* t;
            t = localtime(&tt);

            char tmp[64];
            strftime(tmp, sizeof(tmp), "%d %B %Y %X", t);
            return out << tmp;
        }
    };

    //wywolujemy _Write<T>::push dla t
    template <typename T>
    inline void _makestr(std::ostream& out, T const& t)
    {
        _Writer<T>::push(out,t);
    }

    //wywolujemy _Writer<T>::push dla t (pierwszego argumentu poza out)
    // i rekurencyjnie _makestr dla reszty.
    // bardziej szczegolowo:
    //   mamy tu do czynienia z takim fajnym narzedziem, ktore nazywa sie po angielsku variadic templates,
    //   znaczy to mniej wiecej tyle co "szablon o zmiennej licznie typow"
    //   poza dosyc szerokimi mozliwosciami, jakie daje takie narzedzie, tutaj wykorzystujemy cos pdoobnego do
    //   znanego z C printf'a, ktory moze pobrac dowolna liczbe argumentow dzieki uzyciu ...
    //   w C jest tylko problem, bo nie znamy typow tych argumentow - dlatego musimy podawac dodatkowo
    //   napis z formatowaniem, ktore przy okazji mowi: pierwszy argument potraktuj jako int (a raczej pierwsze 4 bajty argumentow) itd.
    //   tutaj mamy zmienna liczbe argumentow, ale przy okazji znamy ich typy!
    //   dziala to tak, ze mamy paczke typow 'ARGS' i paczke zmiennych 'args' (patrz definicja argumentu funkcji)
    //   poniewaz ARGS moze byc dowolna liczba argumentow, moze tez byc zerem, dlatego wymagamy ekstra, zeby przynajmniej
    //   jeden argument (typu T) zostal na pewno przekazany.
    //    mamy wiec:
    //       _makestr(output, dowolny argument, dowolna liczba dowolnych argumentow)
    //    jesli jako trzeci argument nie bedzie zadnych argumentow (dziwnie to brzmi, ale moze byc dowolna ilosc, wiec 0 tez!)
    //    to zostajemy de facto z:
    //       _makestr(output, dowolny argument)
    //    co wywola funkcje powyzej (i rekurencja sie skonczy, bo tamta funkcja nie wywoluje juz samej siebie!)
    //    jesli natomiast bedziemy mieli np:
    //       _makestr(ostream, int, {int, double, string})
    //    to kolejnosc dzialania jest nastepujaca:
    //       _Writer<int>::push
    //       _makestr(ostream, {int, double, string}...) == _makestr(ostream int, double, string) == _makestr(ostream, int, {double, string})
    //       _Writer<int>::push
    //       _makestr(ostream, {double, string}...) == _makestr(ostream, double, string) == _makestr(ostream, double, {string})
    //       _Writer<double>::push
    //       _makestr(ostream, {string}...) == _makestr(ostream, string) (funckaj powyzej)
    //
    template <typename T, typename... ARGS>
    inline void _makestr(std::ostream& out, T const& t, ARGS const&... args)
    {
        return _makestr(_Writer<T>::push(out, t), args...);
    }
}

template <typename... T>
std::string makestr(T const&... t)
{
    if (sizeof...(t) == 0) //jesli zero argumentow, zwracamy ""
        return "";

    std::stringstream ss;
    _makestr(ss, t...); //jesli conajmniej 1, to pakujemy je do strumienia
    return ss.str(); //zwracamy zawartosc strumienia po upakowaniu wszsystkich zmiennych
}

namespace
{
    //analogicznie jak przy formatowaniu (tylko prosciej ;d)
    // parsujemy zmienne = domyslnie uzywamy stringstream
    template <typename T>
    struct _Parser
    {
        static inline T evaluate(std::string const& str) 
        {
            std::stringstream ss;
            ss.str(str);

            T ret;
            ss >> ret;
            return ret;
        }
    };

    //wyspecjalizowana wersja dla bool (stringstream nie zamieni np. "yes" na true)
    template <>
    struct _Parser<bool>
    {
        static inline bool evaluate(std::string const& str)
        {
            if (str == "1" || str == "y" || str == "yes" || str == "t" || str == "tak" || str == "true")
                return true;
            if (str == "0" || str == "n" || str == "no" || str == "nie" || str == "f" || str == "false")
                return false;
            throw std::runtime_error("Invalid value '" + str + "' passed to be parsed as bool!");
            return false;
        }
    };
    
    //dla wczytywania stringa po prostu zwracamy to, co dostaniemy
    //  notka: mozna sie zapytac, po co w ogole wywolywac paroswanie stringa
    //    klopot w tym, ze uzywajac czesto szablonow mozemy nie wiedziec, co tak naprawde
    //    dostajemy. zamiast sprawdzac za kazdym razem, kiedy parsujemy zmienna nieznanego typu
    //    czy przypadkiem jest stringiem, latwiej uwzglednic to tutaj i sie nie martwic gdzie indziej. :)
    template <>
    struct _Parser<std::string>
    {
        static inline std::string evaluate(std::string const& str) { return str; }
    };

    //dla wstringa konwertujemy
    template <>
    struct _Parser<std::wstring>
    {
        static inline std::wstring evaluate(std::string const& str) { return convert<std::string, std::wstring>(str); }
    };
}

//probuje zwrocic str jako T (notka: domyslnie uzywany jest stringstream, ktory bierze pierwszy wyraz w str)
// moze rzucic wyjatkiem - np. parsowanie bool
template <typename T>
inline T parse(std::string const& str)
{
    return _Parser<T>::evaluate(str);
}

//jak wyzej tylko z try-catchem itd, zwraca, czy sie powiodlo, cel zapisuje przekazywany jako referencja
template <typename T>
inline bool tryParse(std::string const& str, T& var)
{
    try { var = parse<T>(str); return true; }
    catch (...) { return false; }
}

#endif //SHARED_STR_UTILS_H
