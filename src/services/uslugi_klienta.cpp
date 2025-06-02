#include "uslugi_klienta.h"

UslugiKlienta::UslugiKlienta(KlientDAO& klientDAO) : klientDAO(klientDAO) {
}

std::vector<Klient> UslugiKlienta::pobierzWszystkichKlientow() {
    return klientDAO.pobierzWszystkich();
}

std::unique_ptr<Klient> UslugiKlienta::pobierzKlientaPoId(int id) {
    return klientDAO.pobierzPoId(id);
}

int UslugiKlienta::dodajKlienta(const Klient& klient) {
    return klientDAO.dodaj(klient);
}

bool UslugiKlienta::aktualizujKlienta(const Klient& klient) {
    return klientDAO.aktualizuj(klient);
}

bool UslugiKlienta::usunKlienta(int id) {
    return klientDAO.usun(id);
}

std::vector<Klient> UslugiKlienta::wyszukajKlientow(const std::string& klucz) {
    return klientDAO.wyszukaj(klucz);
}