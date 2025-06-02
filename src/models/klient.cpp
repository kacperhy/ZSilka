#define _CRT_SECURE_NO_WARNINGS
#include "klient.h"

Klient::Klient() : id(-1), imie(""), nazwisko(""), email(""),
telefon(""), dataUrodzenia(""), dataRejestracji(pobierzAktualnaDate()),
uwagi("") {
}

Klient::Klient(int id, const std::string& imie, const std::string& nazwisko,
    const std::string& email, const std::string& telefon,
    const std::string& dataUrodzenia, const std::string& dataRejestracji,
    const std::string& uwagi) :
    id(id), imie(imie), nazwisko(nazwisko), email(email),
    telefon(telefon), dataUrodzenia(dataUrodzenia),
    dataRejestracji(dataRejestracji.empty() ? pobierzAktualnaDate() : dataRejestracji),
    uwagi(uwagi) {
}

int Klient::pobierzId() const {
    return id;
}

std::string Klient::pobierzImie() const {
    return imie;
}

std::string Klient::pobierzNazwisko() const {
    return nazwisko;
}

std::string Klient::pobierzEmail() const {
    return email;
}

std::string Klient::pobierzTelefon() const {
    return telefon;
}

std::string Klient::pobierzDateUrodzenia() const {
    return dataUrodzenia;
}

std::string Klient::pobierzDateRejestracji() const {
    return dataRejestracji;
}

std::string Klient::pobierzUwagi() const {
    return uwagi;
}

std::string Klient::pobierzPelneNazwisko() const {
    return imie + " " + nazwisko;
}

void Klient::ustawId(int id) {
    this->id = id;
}

void Klient::ustawImie(const std::string& imie) {
    this->imie = imie;
}

void Klient::ustawNazwisko(const std::string& nazwisko) {
    this->nazwisko = nazwisko;
}

void Klient::ustawEmail(const std::string& email) {
    this->email = email;
}

void Klient::ustawTelefon(const std::string& telefon) {
    this->telefon = telefon;
}

void Klient::ustawDateUrodzenia(const std::string& dataUrodzenia) {
    this->dataUrodzenia = dataUrodzenia;
}

void Klient::ustawDateRejestracji(const std::string& dataRejestracji) {
    this->dataRejestracji = dataRejestracji;
}

void Klient::ustawUwagi(const std::string& uwagi) {
    this->uwagi = uwagi;
}

std::string Klient::pobierzAktualnaDate() {
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d");
    return oss.str();
}