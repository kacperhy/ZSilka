#define _CRT_SECURE_NO_WARNINGS
#include "karnet.h"
#include <chrono>
#include <algorithm>

Karnet::Karnet() : id(-1), idKlienta(-1), typ(""), dataRozpoczecia(""), 
    dataZakonczenia(""), cena(0.0), czyAktywny(false) {
}

Karnet::Karnet(int id, int idKlienta, const std::string& typ,
    const std::string& dataRozpoczecia, const std::string& dataZakonczenia,
    double cena, bool czyAktywny) :
    id(id), idKlienta(idKlienta), typ(typ), dataRozpoczecia(dataRozpoczecia),
    dataZakonczenia(dataZakonczenia), cena(cena), czyAktywny(czyAktywny) {
}

// Gettery
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

// Settery
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

// Metody biznesowe
bool Karnet::czyWazny() const {
    if (!czyAktywny) {
        return false;
    }
    
    std::string dzisiaj = pobierzAktualnaDate();
    
    // Sprawdź czy dzisiaj jest między datą rozpoczęcia a zakończenia
    return (dzisiaj >= dataRozpoczecia && dzisiaj <= dataZakonczenia);
}

int Karnet::ileDniPozostalo() const {
    if (!czyAktywny) {
        return 0;
    }
    
    std::string dzisiaj = pobierzAktualnaDate();
    return dniPomiedzy(dzisiaj, dataZakonczenia);
}

// Metody statyczne
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
    auto tp = std::chrono::system_clock::from_time_t(std::mktime(&tm));
    tp += std::chrono::hours(24 * dni);
    auto time = std::chrono::system_clock::to_time_t(tp);
    auto newTm = *std::localtime(&time);
    
    std::ostringstream oss;
    oss << std::put_time(&newTm, "%Y-%m-%d");
    return oss.str();
}

// Metody prywatne
std::tm Karnet::konwertujStringNaDate(const std::string& tekstDaty) {
    std::tm tm = {};
    std::istringstream ss(tekstDaty);
    ss >> std::get_time(&tm, "%Y-%m-%d");
    return tm;
}

int Karnet::dniPomiedzy(const std::string& data1, const std::string& data2) {
    std::tm tm1 = konwertujStringNaDate(data1);
    std::tm tm2 = konwertujStringNaDate(data2);
    
    auto tp1 = std::chrono::system_clock::from_time_t(std::mktime(&tm1));
    auto tp2 = std::chrono::system_clock::from_time_t(std::mktime(&tm2));
    
    auto duration = tp2 - tp1;
    auto days = std::chrono::duration_cast<std::chrono::hours>(duration).count() / 24;
    
    return static_cast<int>(days);
}