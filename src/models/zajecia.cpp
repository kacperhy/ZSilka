#include "zajecia.h"

Zajecia::Zajecia() : id(-1), nazwa(""), trener(""), maksUczestnikow(0),
data(""), czas(""), czasTrwania(0), opis("") {
}

Zajecia::Zajecia(int id, const std::string& nazwa, const std::string& trener,
    int maksUczestnikow, const std::string& data, const std::string& czas,
    int czasTrwania, const std::string& opis) :
    id(id), nazwa(nazwa), trener(trener), maksUczestnikow(maksUczestnikow),
    data(data), czas(czas), czasTrwania(czasTrwania), opis(opis) {
}

int Zajecia::pobierzId() const {
    return id;
}

std::string Zajecia::pobierzNazwe() const {
    return nazwa;
}

std::string Zajecia::pobierzTrenera() const {
    return trener;
}

int Zajecia::pobierzMaksUczestnikow() const {
    return maksUczestnikow;
}

std::string Zajecia::pobierzDate() const {
    return data;
}

std::string Zajecia::pobierzCzas() const {
    return czas;
}

int Zajecia::pobierzCzasTrwania() const {
    return czasTrwania;
}

std::string Zajecia::pobierzOpis() const {
    return opis;
}

void Zajecia::ustawId(int id) {
    this->id = id;
}

void Zajecia::ustawNazwe(const std::string& nazwa) {
    this->nazwa = nazwa;
}

void Zajecia::ustawTrenera(const std::string& trener) {
    this->trener = trener;
}

void Zajecia::ustawMaksUczestnikow(int maksUczestnikow) {
    this->maksUczestnikow = maksUczestnikow;
}

void Zajecia::ustawDate(const std::string& data) {
    this->data = data;
}

void Zajecia::ustawCzas(const std::string& czas) {
    this->czas = czas;
}

void Zajecia::ustawCzasTrwania(int czasTrwania) {
    this->czasTrwania = czasTrwania;
}

void Zajecia::ustawOpis(const std::string& opis) {
    this->opis = opis;
}