#ifndef EKSPORT_DANYCH_H
#define EKSPORT_DANYCH_H

#include <string>
#include <vector>
#include <stdexcept>
#include "../models/klient.h"
#include "../models/karnet.h"
#include "../models/zajecia.h"
#include "../models/rezerwacja.h"
#include "../services/uslugi_klienta.h"
#include "../services/uslugi_karnetu.h"
#include "../services/uslugi_zajec.h"

class WyjatekEksportu : public std::runtime_error {
public:
    explicit WyjatekEksportu(const std::string& wiadomosc) : std::runtime_error(wiadomosc) {}
};

class EksportDanych {
public:
    EksportDanych(UslugiKlienta& uslugiKlienta, UslugiKarnetu& uslugiKarnetu, UslugiZajec& uslugiZajec);

    bool eksportujKlientowDoCSV(const std::string& sciezkaPliku);
    bool eksportujKarnetyDoCSV(const std::string& sciezkaPliku);
    bool eksportujZajeciaDoCSV(const std::string& sciezkaPliku);
    bool eksportujRezerwacjeDoCSV(const std::string& sciezkaPliku);

    bool eksportujKlientowDoJSON(const std::string& sciezkaPliku);
    bool eksportujKarnetyDoJSON(const std::string& sciezkaPliku);
    bool eksportujZajeciaDoJSON(const std::string& sciezkaPliku);
    bool eksportujRezerwacjeDoJSON(const std::string& sciezkaPliku);

private:
    UslugiKlienta& uslugiKlienta;
    UslugiKarnetu& uslugiKarnetu;
    UslugiZajec& uslugiZajec;

    std::string escapujCSV(const std::string& str);

    std::string klientDoJSON(const Klient& klient);
    std::string karnetDoJSON(const Karnet& karnet);
    std::string zajeciaDoJSON(const Zajecia& zajecia);
    std::string rezerwacjaDoJSON(const Rezerwacja& rezerwacja);
    std::string escapujJSON(const std::string& str);
};

#endif 