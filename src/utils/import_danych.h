#ifndef IMPORT_DANYCH_H
#define IMPORT_DANYCH_H

#include <string>
#include <vector>
#include <memory>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include "../models/klient.h"
#include "../models/karnet.h"
#include "../models/zajecia.h"
#include "../services/uslugi_klienta.h"
#include "../services/uslugi_karnetu.h"
#include "../services/uslugi_zajec.h"

class WyjatekImportu : public std::runtime_error {
public:
    explicit WyjatekImportu(const std::string& wiadomosc) : std::runtime_error(wiadomosc) {}
};

class ImportDanych {
public:
    ImportDanych(UslugiKlienta& uslugiKlienta, UslugiKarnetu& uslugiKarnetu, UslugiZajec& uslugiZajec);

    std::vector<Klient> importujKlientowZCSV(const std::string& sciezkaPliku);
    std::vector<Karnet> importujKarnetyZCSV(const std::string& sciezkaPliku);
    std::vector<Zajecia> importujZajeciaZCSV(const std::string& sciezkaPliku);

    std::vector<Klient> importujKlientowZJSON(const std::string& sciezkaPliku);
    std::vector<Karnet> importujKarnetyZJSON(const std::string& sciezkaPliku);
    std::vector<Zajecia> importujZajeciaZJSON(const std::string& sciezkaPliku);

    void zapiszZaimportowanychKlientow(const std::vector<Klient>& klienci);
    void zapiszZaimportowaneKarnety(const std::vector<Karnet>& karnety);
    void zapiszZaimportowaneZajecia(const std::vector<Zajecia>& zajecia);

private:
    UslugiKlienta& uslugiKlienta;
    UslugiKarnetu& uslugiKarnetu;
    UslugiZajec& uslugiZajec;

    std::vector<std::vector<std::string>> analizujCSV(const std::string& sciezkaPliku, char separator = ',');
    Klient analizujWierszKlienta(const std::vector<std::string>& wiersz);
    Karnet analizujWierszKarnetu(const std::vector<std::string>& wiersz);
    Zajecia analizujWierszZajec(const std::vector<std::string>& wiersz);

    std::string wczytajPlikJSON(const std::string& sciezkaPliku);
};

#endif