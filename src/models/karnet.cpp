#define _CRT_SECURE_NO_WARNINGS
#include "karnet.h"
#include <sstream>

Karnet::Karnet() : id(-1), idKlienta(-1), typ(""), dataRozpoczecia(""),
dataZakonczenia(""), cena(0.0), czyAktywny(false) {
}

Karnet::Karnet(int id, int idKlienta, const std::string& typ,
    const std::string& dataRozpoczecia, const std::string& dataZakonczenia,
    double cena, bool czyAktywny) :
    id(id), idKlienta(idKlienta), typ(typ), dataRozpoczecia(dataRozpoczecia),
    dataZakonczenia(dataZakonczenia), cena(cena), czyAktywny(czyAktywny) {
}

int Karnet::pobierzId() const {
    return id;
}

int Karnet::pobierzIdKlienta() const {
    return idKlienta;
}

std::string Karnet::pobierzTyp() const {
    return typ;
}

std::string Karnet::pobierzDateRozpoczecia() const {
    return dataRozpoczecia;
}

std::string Karnet::pobierzDateZakonczenia() const {
    return dataZakonczenia;
}

double Karnet::pobierzCene() const {
    return cena;
}

bool Karnet::pobierzCzyAktywny() const {
    return czyAktywny;
}

void Karnet::ustawId(int id) {
    this->id = id;
}

void Karnet::ustawIdKlienta(int idKlienta) {
    this->idKlienta = idKlienta;
}

void Karnet::ustawTyp(const std::string& typ) {
    this->typ = typ;
}

void Karnet::ustawDateRozpoczecia(const std::string& dataRozpoczecia) {
    this->dataRozpoczecia = dataRozpoczecia;
}

void Karnet::ustawDateZakonczenia(const std::string& dataZakonczenia) {
    this->dataZakonczenia = dataZakonczenia;
}

void Karnet::ustawCene(double cena) {
    this->cena = cena;
}

void Karnet::ustawCzyAktywny(bool czyAktywny) {
    this->czyAktywny = czyAktywny;
}

bool Karnet::czyWazny() const {
    std::string dzisiaj = pobierzAktualnaDate();
    return dzisiaj >= dataRozpoczecia && dzisiaj <= dataZakonczenia && czyAktywny;
}

int Karnet::ileDniPozostalo() const {
    std::string dzisiaj = pobierzAktualnaDate();
    return dniPomiedzy(dzisiaj, dataZakonczenia);
}

std::string Karnet::pobierzAktualnaDate() {
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d");
    return oss.str();
}

std::string Karnet::dodajDniDoData(const std::string& data, int dni) {
    std::tm tm = konwertujStringNaDate(data);

    // Dodaj dni
    tm.tm_mday += dni;

    // Normalizuj datê
    std::mktime(&tm);

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d");
    return oss.str();
}

std::tm Karnet::konwertujStringNaDate(const std::string& tekstDaty) {
    std::tm tm = {};
    std::istringstream ss(tekstDaty);
    ss >> std::get_time(&tm, "%Y-%m-%d");

    if (ss.fail()) {
        // Jeœli parsowanie siê nie powiod³o, zwróæ dzisiejsz¹ datê
        auto t = std::time(nullptr);
        return *std::localtime(&t);
    }

    return tm;
}

int Karnet::dniPomiedzy(const std::string& data1, const std::string& data2) {
    std::tm tm1 = konwertujStringNaDate(data1);
    std::tm tm2 = konwertujStringNaDate(data2);

    std::time_t time1 = std::mktime(&tm1);
    std::time_t time2 = std::mktime(&tm2);

    // Oblicz ró¿nicê w sekundach i przekonwertuj na dni
    double diff = std::difftime(time2, time1);
    return static_cast<int>(diff / (60 * 60 * 24));
}