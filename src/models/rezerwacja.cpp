#define _CRT_SECURE_NO_WARNINGS
#include "rezerwacja.h"

Rezerwacja::Rezerwacja() : id(-1), idKlienta(-1), idZajec(-1),
dataRezerwacji(pobierzAktualnyCzas()), status("potwierdzona") {
}

Rezerwacja::Rezerwacja(int id, int idKlienta, int idZajec,
    const std::string& dataRezerwacji, const std::string& status) :
    id(id), idKlienta(idKlienta), idZajec(idZajec),
    dataRezerwacji(dataRezerwacji.empty() ? pobierzAktualnyCzas() : dataRezerwacji),
    status(status.empty() ? "potwierdzona" : status) {
}

int Rezerwacja::pobierzId() const {
    return id;
}

int Rezerwacja::pobierzIdKlienta() const {
    return idKlienta;
}

int Rezerwacja::pobierzIdZajec() const {
    return idZajec;
}

std::string Rezerwacja::pobierzDateRezerwacji() const {
    return dataRezerwacji;
}

std::string Rezerwacja::pobierzStatus() const {
    return status;
}

void Rezerwacja::ustawId(int id) {
    this->id = id;
}

void Rezerwacja::ustawIdKlienta(int idKlienta) {
    this->idKlienta = idKlienta;
}

void Rezerwacja::ustawIdZajec(int idZajec) {
    this->idZajec = idZajec;
}

void Rezerwacja::ustawDateRezerwacji(const std::string& dataRezerwacji) {
    this->dataRezerwacji = dataRezerwacji;
}

void Rezerwacja::ustawStatus(const std::string& status) {
    this->status = status;
}

bool Rezerwacja::czyPotwierdzona() const {
    return status == "potwierdzona";
}

std::string Rezerwacja::pobierzAktualnyCzas() {
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}