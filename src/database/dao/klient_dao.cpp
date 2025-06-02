#include "klient_dao.h"
#include <sstream>

KlientDAO::KlientDAO(MenedzerBD& menedzerBD) : menedzerBD(menedzerBD) {
}

std::vector<Klient> KlientDAO::pobierzWszystkich() {
    std::vector<Klient> klienci;

    std::string zapytanie = R"(
        SELECT id, first_name, last_name, email, phone, 
               birth_date, registration_date, notes
        FROM clients
        ORDER BY last_name, first_name
    )";

    auto wyniki = menedzerBD.pobierzDane(zapytanie);

    for (const auto& wiersz : wyniki) {
        klienci.push_back(utworzKlientaZWiersza(wiersz));
    }

    return klienci;
}

std::unique_ptr<Klient> KlientDAO::pobierzPoId(int id) {
    std::string zapytanie = R"(
        SELECT id, first_name, last_name, email, phone, 
               birth_date, registration_date, notes
        FROM clients
        WHERE id = ?
    )";

    std::vector<ParamZapytania> parametry = {{1, std::to_string(id)}};
    auto wyniki = menedzerBD.pobierzDaneZParametrami(zapytanie, parametry);

    if (wyniki.empty()) {
        return nullptr;
    }

    return std::make_unique<Klient>(utworzKlientaZWiersza(wyniki[0]));
}

int KlientDAO::dodaj(const Klient& klient) {
    std::stringstream ss;
    ss << "INSERT INTO clients (first_name, last_name, email, phone, "
       << "birth_date, registration_date, notes) VALUES ('"
       << klient.pobierzImie() << "', '"
       << klient.pobierzNazwisko() << "', '"
       << klient.pobierzEmail() << "', '"
       << klient.pobierzTelefon() << "', '"
       << klient.pobierzDateUrodzenia() << "', '"
       << klient.pobierzDateRejestracji() << "', '"
       << klient.pobierzUwagi() << "')";

    return menedzerBD.wykonajZapytanieZwracajaceId(ss.str());
}

bool KlientDAO::aktualizuj(const Klient& klient) {
    try {
        std::stringstream ss;
        ss << "UPDATE clients SET "
           << "first_name = '" << klient.pobierzImie() << "', "
           << "last_name = '" << klient.pobierzNazwisko() << "', "
           << "email = '" << klient.pobierzEmail() << "', "
           << "phone = '" << klient.pobierzTelefon() << "', "
           << "birth_date = '" << klient.pobierzDateUrodzenia() << "', "
           << "registration_date = '" << klient.pobierzDateRejestracji() << "', "
           << "notes = '" << klient.pobierzUwagi() << "' "
           << "WHERE id = " << klient.pobierzId();

        menedzerBD.wykonajZapytanie(ss.str());
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

bool KlientDAO::usun(int id) {
    try {
        std::string zapytanie = "DELETE FROM clients WHERE id = " + std::to_string(id);
        menedzerBD.wykonajZapytanie(zapytanie);
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

std::vector<Klient> KlientDAO::wyszukaj(const std::string& klucz) {
    std::vector<Klient> klienci;

    std::string zapytanie = R"(
        SELECT id, first_name, last_name, email, phone, 
               birth_date, registration_date, notes
        FROM clients
        WHERE first_name LIKE ? OR last_name LIKE ? OR email LIKE ? OR phone LIKE ?
        ORDER BY last_name, first_name
    )";

    std::string wzorzec = "%" + klucz + "%";
    std::vector<ParamZapytania> parametry = {
        {1, wzorzec},
        {2, wzorzec},
        {3, wzorzec},
        {4, wzorzec}
    };

    auto wyniki = menedzerBD.pobierzDaneZParametrami(zapytanie, parametry);

    for (const auto& wiersz : wyniki) {
        klienci.push_back(utworzKlientaZWiersza(wiersz));
    }

    return klienci;
}

Klient KlientDAO::utworzKlientaZWiersza(const WierszBD& wiersz) {
    if (wiersz.size() < 8) {
        throw WyjatekBazyDanych("Nieprawidłowa liczba kolumn w wierszu klienta");
    }

    int id = std::stoi(wiersz[0]);
    std::string imie = wiersz[1];
    std::string nazwisko = wiersz[2];
    std::string email = wiersz[3];
    std::string telefon = wiersz[4];
    std::string dataUrodzenia = wiersz[5];
    std::string dataRejestracji = wiersz[6];
    std::string uwagi = wiersz[7];

    return Klient(id, imie, nazwisko, email, telefon, dataUrodzenia, dataRejestracji, uwagi);
}