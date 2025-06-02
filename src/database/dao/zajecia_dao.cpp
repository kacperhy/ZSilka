#include "zajecia_dao.h"
#include <sstream>

ZajeciaDAO::ZajeciaDAO(MenedzerBD& menedzerBD) : menedzerBD(menedzerBD) {
}

// === ZARZĄDZANIE ZAJĘCIAMI ===

std::vector<Zajecia> ZajeciaDAO::pobierzWszystkieZajecia() {
    std::vector<Zajecia> zajecia;

    std::string zapytanie = R"(
        SELECT id, name, trainer, max_participants, date, time, duration, description
        FROM gym_classes
        ORDER BY date, time
    )";

    auto wyniki = menedzerBD.pobierzDane(zapytanie);

    for (const auto& wiersz : wyniki) {
        zajecia.push_back(utworzZajeciaZWiersza(wiersz));
    }

    return zajecia;
}

std::unique_ptr<Zajecia> ZajeciaDAO::pobierzZajeciaPoId(int id) {
    std::string zapytanie = R"(
        SELECT id, name, trainer, max_participants, date, time, duration, description
        FROM gym_classes
        WHERE id = )" + std::to_string(id);

    auto wyniki = menedzerBD.pobierzDane(zapytanie);

    if (wyniki.empty()) {
        return nullptr;
    }

    return std::make_unique<Zajecia>(utworzZajeciaZWiersza(wyniki[0]));
}

int ZajeciaDAO::dodajZajecia(const Zajecia& zajecia) {
    std::stringstream ss;
    ss << "INSERT INTO gym_classes (name, trainer, max_participants, date, time, duration, description) "
       << "VALUES ('" << zajecia.pobierzNazwe() << "', '"
       << zajecia.pobierzTrenera() << "', "
       << zajecia.pobierzMaksUczestnikow() << ", '"
       << zajecia.pobierzDate() << "', '"
       << zajecia.pobierzCzas() << "', "
       << zajecia.pobierzCzasTrwania() << ", '"
       << zajecia.pobierzOpis() << "')";

    return menedzerBD.wykonajZapytanieZwracajaceId(ss.str());
}

bool ZajeciaDAO::aktualizujZajecia(const Zajecia& zajecia) {
    try {
        std::stringstream ss;
        ss << "UPDATE gym_classes SET "
           << "name = '" << zajecia.pobierzNazwe() << "', "
           << "trainer = '" << zajecia.pobierzTrenera() << "', "
           << "max_participants = " << zajecia.pobierzMaksUczestnikow() << ", "
           << "date = '" << zajecia.pobierzDate() << "', "
           << "time = '" << zajecia.pobierzCzas() << "', "
           << "duration = " << zajecia.pobierzCzasTrwania() << ", "
           << "description = '" << zajecia.pobierzOpis() << "' "
           << "WHERE id = " << zajecia.pobierzId();

        menedzerBD.wykonajZapytanie(ss.str());
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

bool ZajeciaDAO::usunZajecia(int id) {
    try {
        // Najpierw usuń wszystkie rezerwacje powiązane z tymi zajęciami
        std::string zapytanieRezerwacje = "DELETE FROM reservations WHERE class_id = " + std::to_string(id);
        menedzerBD.wykonajZapytanie(zapytanieRezerwacje);

        // Następnie usuń zajęcia
        std::string zapytanie = "DELETE FROM gym_classes WHERE id = " + std::to_string(id);
        menedzerBD.wykonajZapytanie(zapytanie);
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

// === ZARZĄDZANIE REZERWACJAMI ===

std::vector<Rezerwacja> ZajeciaDAO::pobierzWszystkieRezerwacje() {
    std::vector<Rezerwacja> rezerwacje;

    std::string zapytanie = R"(
        SELECT id, client_id, class_id, reservation_date, status
        FROM reservations
        ORDER BY reservation_date DESC
    )";

    auto wyniki = menedzerBD.pobierzDane(zapytanie);

    for (const auto& wiersz : wyniki) {
        rezerwacje.push_back(utworzRezerwacjeZWiersza(wiersz));
    }

    return rezerwacje;
}

std::vector<Rezerwacja> ZajeciaDAO::pobierzRezerwacjeKlienta(int idKlienta) {
    std::vector<Rezerwacja> rezerwacje;

    std::string zapytanie = R"(
        SELECT id, client_id, class_id, reservation_date, status
        FROM reservations
        WHERE client_id = )" + std::to_string(idKlienta) + R"(
        ORDER BY reservation_date DESC
    )";

    auto wyniki = menedzerBD.pobierzDane(zapytanie);

    for (const auto& wiersz : wyniki) {
        rezerwacje.push_back(utworzRezerwacjeZWiersza(wiersz));
    }

    return rezerwacje;
}

std::vector<Rezerwacja> ZajeciaDAO::pobierzRezerwacjeZajec(int idZajec) {
    std::vector<Rezerwacja> rezerwacje;

    std::string zapytanie = R"(
        SELECT id, client_id, class_id, reservation_date, status
        FROM reservations
        WHERE class_id = )" + std::to_string(idZajec) + R"(
        AND status = 'potwierdzona'
        ORDER BY reservation_date
    )";

    auto wyniki = menedzerBD.pobierzDane(zapytanie);

    for (const auto& wiersz : wyniki) {
        rezerwacje.push_back(utworzRezerwacjeZWiersza(wiersz));
    }

    return rezerwacje;
}

int ZajeciaDAO::dodajRezerwacje(const Rezerwacja& rezerwacja) {
    std::stringstream ss;
    ss << "INSERT INTO reservations (client_id, class_id, reservation_date, status) "
       << "VALUES (" << rezerwacja.pobierzIdKlienta() << ", "
       << rezerwacja.pobierzIdZajec() << ", '"
       << rezerwacja.pobierzDateRezerwacji() << "', '"
       << rezerwacja.pobierzStatus() << "')";

    return menedzerBD.wykonajZapytanieZwracajaceId(ss.str());
}

bool ZajeciaDAO::aktualizujRezerwacje(const Rezerwacja& rezerwacja) {
    try {
        std::stringstream ss;
        ss << "UPDATE reservations SET "
           << "client_id = " << rezerwacja.pobierzIdKlienta() << ", "
           << "class_id = " << rezerwacja.pobierzIdZajec() << ", "
           << "reservation_date = '" << rezerwacja.pobierzDateRezerwacji() << "', "
           << "status = '" << rezerwacja.pobierzStatus() << "' "
           << "WHERE id = " << rezerwacja.pobierzId();

        menedzerBD.wykonajZapytanie(ss.str());
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

bool ZajeciaDAO::usunRezerwacje(int id) {
    try {
        std::string zapytanie = "DELETE FROM reservations WHERE id = " + std::to_string(id);
        menedzerBD.wykonajZapytanie(zapytanie);
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

// === METODY BIZNESOWE ===

int ZajeciaDAO::policzRezerwacjeZajec(int idZajec) {
    std::string zapytanie = R"(
        SELECT COUNT(*) 
        FROM reservations 
        WHERE class_id = )" + std::to_string(idZajec) + R"(
        AND status = 'potwierdzona'
    )";

    std::string wynik = menedzerBD.pobierzWartosc(zapytanie);
    return wynik.empty() ? 0 : std::stoi(wynik);
}

// === METODY POMOCNICZE ===

Zajecia ZajeciaDAO::utworzZajeciaZWiersza(const WierszBD& wiersz) {
    if (wiersz.size() < 8) {
        throw WyjatekBazyDanych("Nieprawidłowa liczba kolumn w wierszu zajęć");
    }

    int id = std::stoi(wiersz[0]);
    std::string nazwa = wiersz[1];
    std::string trener = wiersz[2];
    int maksUczestnikow = std::stoi(wiersz[3]);
    std::string data = wiersz[4];
    std::string czas = wiersz[5];
    int czasTrwania = std::stoi(wiersz[6]);
    std::string opis = wiersz[7];

    return Zajecia(id, nazwa, trener, maksUczestnikow, data, czas, czasTrwania, opis);
}

Rezerwacja ZajeciaDAO::utworzRezerwacjeZWiersza(const WierszBD& wiersz) {
    if (wiersz.size() < 5) {
        throw WyjatekBazyDanych("Nieprawidłowa liczba kolumn w wierszu rezerwacji");
    }

    int id = std::stoi(wiersz[0]);
    int idKlienta = std::stoi(wiersz[1]);
    int idZajec = std::stoi(wiersz[2]);
    std::string dataRezerwacji = wiersz[3];
    std::string status = wiersz[4];

    return Rezerwacja(id, idKlienta, idZajec, dataRezerwacji, status);
}