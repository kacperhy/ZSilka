#include "import_danych.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <map>
#include <algorithm>
#include <cstring>
#include <cctype>

namespace {
    std::string usunBialeZnaki(const std::string& wejscie) {
        std::string wyjscie;
        wyjscie.reserve(wejscie.size());
        bool wCudzyslowie = false;

        for (char c : wejscie) {
            if (c == '"') {
                wCudzyslowie = !wCudzyslowie;
                wyjscie += c;
            }
            else if (wCudzyslowie || !std::isspace(c)) {
                wyjscie += c;
            }
        }

        return wyjscie;
    }

    std::vector<std::string> podziel(const std::string& wejscie, char separator) {
        std::vector<std::string> wynik;
        std::stringstream ss(wejscie);
        std::string element;

        while (std::getline(ss, element, separator)) {
            wynik.push_back(element);
        }

        return wynik;
    }

    std::string usunCudzyslow(const std::string& s) {
        if (s.size() >= 2 && s.front() == '"' && s.back() == '"') {
            return s.substr(1, s.size() - 2);
        }
        return s;
    }

    std::string przytnij(const std::string& s) {
        auto start = s.begin();
        while (start != s.end() && std::isspace(*start)) {
            start++;
        }

        auto koniec = s.end();
        do {
            koniec--;
        } while (start != koniec && std::isspace(*koniec));

        return std::string(start, koniec + 1);
    }
}

ImportDanych::ImportDanych(UslugiKlienta& uslugiKlienta, UslugiKarnetu& uslugiKarnetu, UslugiZajec& uslugiZajec)
    : uslugiKlienta(uslugiKlienta), uslugiKarnetu(uslugiKarnetu), uslugiZajec(uslugiZajec) {
}

std::vector<std::vector<std::string>> ImportDanych::analizujCSV(const std::string& sciezkaPliku, char separator) {
    std::ifstream plik(sciezkaPliku);
    if (!plik.is_open()) {
        throw WyjatekImportu("Nie można otworzyć pliku: " + sciezkaPliku);
    }

    std::vector<std::vector<std::string>> dane;
    std::string linia;

    std::getline(plik, linia);

    while (std::getline(plik, linia)) {
        std::vector<std::string> wiersz;
        std::string komorka;

        bool wCudzyslowie = false;
        std::stringstream ss(linia);

        while (ss.good()) {
            char c = ss.get();

            if (c == EOF) {
                break;
            }
            else if (c == '"') {
                wCudzyslowie = !wCudzyslowie;
            }
            else if (c == separator && !wCudzyslowie) {
                wiersz.push_back(przytnij(komorka));
                komorka.clear();
            }
            else {
                komorka += c;
            }
        }

        if (!komorka.empty()) {
            wiersz.push_back(przytnij(komorka));
        }

        if (!wiersz.empty()) {
            dane.push_back(wiersz);
        }
    }

    plik.close();
    return dane;
}

Klient ImportDanych::analizujWierszKlienta(const std::vector<std::string>& wiersz) {
    if (wiersz.size() < 6) {
        throw WyjatekImportu("Nieprawidłowa liczba kolumn dla klienta");
    }

    Klient klient;


    klient.ustawImie(usunCudzyslow(wiersz[1]));
    klient.ustawNazwisko(usunCudzyslow(wiersz[2]));
    klient.ustawEmail(usunCudzyslow(wiersz[3]));
    klient.ustawTelefon(usunCudzyslow(wiersz[4]));
    klient.ustawDateUrodzenia(usunCudzyslow(wiersz[5]));

    if (wiersz.size() > 6) {
        klient.ustawDateRejestracji(usunCudzyslow(wiersz[6]));
    }

    if (wiersz.size() > 7) {
        klient.ustawUwagi(usunCudzyslow(wiersz[7]));
    }

    return klient;
}

Karnet ImportDanych::analizujWierszKarnetu(const std::vector<std::string>& wiersz) {
    if (wiersz.size() < 6) {
        throw WyjatekImportu("Nieprawidłowa liczba kolumn dla karnetu");
    }

    Karnet karnet;

    karnet.ustawIdKlienta(std::stoi(usunCudzyslow(wiersz[1])));
    karnet.ustawTyp(usunCudzyslow(wiersz[2]));
    karnet.ustawDateRozpoczecia(usunCudzyslow(wiersz[3]));
    karnet.ustawDateZakonczenia(usunCudzyslow(wiersz[4]));
    karnet.ustawCene(std::stod(usunCudzyslow(wiersz[5])));

    if (wiersz.size() > 6) {
        karnet.ustawCzyAktywny(usunCudzyslow(wiersz[6]) == "1" || usunCudzyslow(wiersz[6]) == "true");
    }

    return karnet;
}

Zajecia ImportDanych::analizujWierszZajec(const std::vector<std::string>& wiersz) {
    if (wiersz.size() < 7) {
        throw WyjatekImportu("Nieprawidłowa liczba kolumn dla zajęć");
    }

    Zajecia zajecia;


    zajecia.ustawNazwe(usunCudzyslow(wiersz[1]));
    zajecia.ustawTrenera(usunCudzyslow(wiersz[2]));
    zajecia.ustawMaksUczestnikow(std::stoi(usunCudzyslow(wiersz[3])));
    zajecia.ustawDate(usunCudzyslow(wiersz[4]));
    zajecia.ustawCzas(usunCudzyslow(wiersz[5]));
    zajecia.ustawCzasTrwania(std::stoi(usunCudzyslow(wiersz[6])));

    if (wiersz.size() > 7) {
        zajecia.ustawOpis(usunCudzyslow(wiersz[7]));
    }

    return zajecia;
}

std::vector<Klient> ImportDanych::importujKlientowZCSV(const std::string& sciezkaPliku) {
    std::vector<Klient> klienci;

    try {
        auto dane = analizujCSV(sciezkaPliku);

        for (const auto& wiersz : dane) {
            klienci.push_back(analizujWierszKlienta(wiersz));
        }
    }
    catch (const std::exception& e) {
        throw WyjatekImportu(std::string("Błąd podczas importu klientów z CSV: ") + e.what());
    }

    return klienci;
}

std::vector<Karnet> ImportDanych::importujKarnetyZCSV(const std::string& sciezkaPliku) {
    std::vector<Karnet> karnety;

    try {
        auto dane = analizujCSV(sciezkaPliku);

        for (const auto& wiersz : dane) {
            karnety.push_back(analizujWierszKarnetu(wiersz));
        }
    }
    catch (const std::exception& e) {
        throw WyjatekImportu(std::string("Błąd podczas importu karnetów z CSV: ") + e.what());
    }

    return karnety;
}

std::vector<Zajecia> ImportDanych::importujZajeciaZCSV(const std::string& sciezkaPliku) {
    std::vector<Zajecia> zajecia;

    try {
        auto dane = analizujCSV(sciezkaPliku);

        for (const auto& wiersz : dane) {
            zajecia.push_back(analizujWierszZajec(wiersz));
        }
    }
    catch (const std::exception& e) {
        throw WyjatekImportu(std::string("Błąd podczas importu zajęć z CSV: ") + e.what());
    }

    return zajecia;
}


std::string ImportDanych::wczytajPlikJSON(const std::string& sciezkaPliku) {
    std::ifstream plik(sciezkaPliku);
    if (!plik.is_open()) {
        throw WyjatekImportu("Nie można otworzyć pliku: " + sciezkaPliku);
    }

    std::stringstream bufor;
    bufor << plik.rdbuf();
    plik.close();

    return bufor.str();
}

std::vector<Klient> ImportDanych::importujKlientowZJSON(const std::string& sciezkaPliku) {

    std::vector<Klient> klienci;

    try {
        std::string json = wczytajPlikJSON(sciezkaPliku);
        json = usunBialeZnaki(json);

        size_t start = json.find('[');
        size_t koniec = json.rfind(']');

        if (start == std::string::npos || koniec == std::string::npos) {
            throw WyjatekImportu("Nieprawidłowy format JSON");
        }

        std::string zawartoscTablicy = json.substr(start + 1, koniec - start - 1);
        std::vector<std::string> objektyKlientow;

        size_t pos = 0;
        std::string delimiter = "},{";
        std::string token;

        while ((pos = zawartoscTablicy.find(delimiter)) != std::string::npos) {
            token = zawartoscTablicy.substr(0, pos + 1); 
            objektyKlientow.push_back(token);
            zawartoscTablicy.erase(0, pos + delimiter.length() - 1); 
        }

        if (!zawartoscTablicy.empty()) {
            objektyKlientow.push_back(zawartoscTablicy);
        }

        for (const auto& objektKlienta : objektyKlientow) {
            Klient klient;

            std::map<std::string, std::string> pola;
            std::string objektStr = objektKlienta;


            if (objektStr.front() == '{') objektStr = objektStr.substr(1);
            if (objektStr.back() == '}') objektStr = objektStr.substr(0, objektStr.size() - 1);

            std::vector<std::string> paryPol = podziel(objektStr, ',');

            for (const auto& para : paryPol) {
                size_t pozycjaDwukropka = para.find(':');
                if (pozycjaDwukropka != std::string::npos) {
                    std::string klucz = para.substr(0, pozycjaDwukropka);
                    std::string wartosc = para.substr(pozycjaDwukropka + 1);

                    if (klucz.front() == '"') klucz = klucz.substr(1);
                    if (klucz.back() == '"') klucz = klucz.substr(0, klucz.size() - 1);

                    if (wartosc.front() == '"') wartosc = wartosc.substr(1);
                    if (wartosc.back() == '"') wartosc = wartosc.substr(0, wartosc.size() - 1);

                    pola[klucz] = wartosc;
                }
            }

            if (pola.count("imie")) klient.ustawImie(pola["imie"]);
            if (pola.count("nazwisko")) klient.ustawNazwisko(pola["nazwisko"]);
            if (pola.count("email")) klient.ustawEmail(pola["email"]);
            if (pola.count("telefon")) klient.ustawTelefon(pola["telefon"]);
            if (pola.count("dataUrodzenia")) klient.ustawDateUrodzenia(pola["dataUrodzenia"]);
            if (pola.count("dataRejestracji")) klient.ustawDateRejestracji(pola["dataRejestracji"]);
            if (pola.count("uwagi")) klient.ustawUwagi(pola["uwagi"]);

            klienci.push_back(klient);
        }
    }
    catch (const std::exception& e) {
        throw WyjatekImportu(std::string("Błąd podczas importu klientów z JSON: ") + e.what());
    }

    return klienci;
}

std::vector<Karnet> ImportDanych::importujKarnetyZJSON(const std::string& sciezkaPliku) {

    std::vector<Karnet> karnety;

    return karnety;
}

std::vector<Zajecia> ImportDanych::importujZajeciaZJSON(const std::string& sciezkaPliku) {

    std::vector<Zajecia> zajecia;

    return zajecia;
}

void ImportDanych::zapiszZaimportowanychKlientow(const std::vector<Klient>& klienci) {
    for (const auto& klient : klienci) {
        uslugiKlienta.dodajKlienta(klient);
    }
}

void ImportDanych::zapiszZaimportowaneKarnety(const std::vector<Karnet>& karnety) {
    for (const auto& karnet : karnety) {
        uslugiKarnetu.dodajKarnet(karnet);
    }
}

void ImportDanych::zapiszZaimportowaneZajecia(const std::vector<Zajecia>& zajecia) {
    for (const auto& zajecie : zajecia) {
        uslugiZajec.dodajZajecia(zajecie);
    }
}