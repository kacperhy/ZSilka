#include "klient_dao.h"
#include <sstream>

KlientDAO::KlientDAO(MenedzerBD& menedzerBD) : menedzerBD(menedzerBD) {
}

std::vector<Klient> KlientDAO::pobierzWszystkich() {
    std::string zapytanie = "SELECT id, first_name, last_name, email, phone, birth_date, registration_date, notes FROM clients ORDER BY last_name, first_name";

    auto wyniki = menedzerBD.pobierzDane(zapytanie);
    std::vector<Klient> klienci;

    for (const auto& wiersz : wyniki) {
        klienci.push_back(utworzKlientaZWiersza(wiersz));
    }

    return klienci;
}

std::unique_ptr<Klient> KlientDAO::pobierzPoId(int id) {
    std::string zapytanie = "SELECT id, first_name, last_name, email, phone, birth_date, registration_date, notes FROM clients WHERE id = " + std::to_string(id);

    auto wiersz = menedzerBD.pobierzWiersz(zapytanie);

    if (wiersz.empty()) {
        return nullptr;
    }

    return std::make_unique<Klient>(utworzKlientaZWiersza(wiersz));
}

int KlientDAO::dodaj(const Klient& klient) {
    std::stringstream ss;
    ss << "INSERT INTO clients (first_name, last_name, email, phone, birth_date, registration_date, notes) VALUES ('"
        << klient.pobierzImie() << "', '"
        << klient.pobierzNazwisko() << "', '"
        << klient.pobierzEmail() << "', '"
        << klient.pobierzTelefon() << "', '"
        << klient.pobierzDateUrodzenia() << "', '"
        << (klient.pobierzDateRejestracji().empty() ? Klient::pobierzAktualnaDate() : klient.pobierzDateRejestracji()) << "', '"
        << klient.pobierzUwagi() << "')";

    return menedzerBD.wykonajZapytanieZwracajaceId(ss.str());
}

bool KlientDAO::aktualizuj(const Klient& klient) {
    std::stringstream ss;
    ss << "UPDATE clients SET "
        << "first_name = '" << klient.pobierzImie() << "', "
        << "last_name = '" << klient.pobierzNazwisko() << "', "
        << "email = '" << klient.pobierzEmail() << "', "
        << "phone = '" << klient.pobierzTelefon() << "', "
        << "birth_date = '" << klient.pobierzDateUrodzenia() << "', "
        << "notes = '" << klient.pobierzUwagi() << "' "
        << "WHERE id = " << klient.pobierzId();

    try {
        menedzerBD.wykonajZapytanie(ss.str());
        return true;
    }
    catch (const std::exception&) {
        return false;
    }
}

bool KlientDAO::usun(int id) {
    std::string zapytanie = "DELETE FROM clients WHERE id = " + std::to_string(id);

    try {
        menedzerBD.wykonajZapytanie(zapytanie);
        return true;
    }
    catch (const std::exception&) {
        return false;
    }
}

std::vector<Klient> KlientDAO::wyszukaj(const std::string& klucz) {
    std::stringstream ss;
    ss << "SELECT id, first_name, last_name, email, phone, birth_date, registration_date, notes FROM clients WHERE "
        << "first_name LIKE '%" << klucz << "%' OR "
        << "last_name LIKE '%" << klucz << "%' OR "
        << "email LIKE '%" << klucz << "%' OR "
        << "phone LIKE '%" << klucz << "%' "
        << "ORDER BY last_name, first_name";

    auto wyniki = menedzerBD.pobierzDane(ss.str());
    std::vector<Klient> klienci;

    for (const auto& wiersz : wyniki) {
        klienci.push_back(utworzKlientaZWiersza(wiersz));
    }

    return klienci;
}

Klient KlientDAO::utworzKlientaZWiersza(const WierszBD& wiersz) {
    if (wiersz.size() < 8) {
        throw std::runtime_error("Nieprawid³owy wiersz danych klienta");
    }

    return Klient(
        std::stoi(wiersz[0]),  // id
        wiersz[1],             // first_name
        wiersz[2],             // last_name
        wiersz[3],             // email
        wiersz[4],             // phone
        wiersz[5],             // birth_date
        wiersz[6],             // registration_date
        wiersz[7]              // notes
    );
}