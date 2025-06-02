#include "zajecia_dao.h"
#include <sstream>

ZajeciaDAO::ZajeciaDAO(MenedzerBD& menedzerBD) : menedzerBD(menedzerBD) {
}

std::vector<Zajecia> ZajeciaDAO::pobierzWszystkieZajecia() {
    std::string zapytanie = "SELECT id, name, trainer, max_participants, date, time, duration, description FROM gym_classes ORDER BY date, time";

    auto wyniki = menedzerBD.pobierzDane(zapytanie);
    std::vector<Zajecia> zajecia;

    for (const auto& wiersz : wyniki) {
        zajecia.push_back(utworzZajeciaZWiersza(wiersz));
    }

    return zajecia;
}

std::unique_ptr<Zajecia> ZajeciaDAO::pobierzZajeciaPoId(int id) {
    std::string zapytanie = "SELECT id, name, trainer, max_participants, date, time, duration, description FROM gym_classes WHERE id = " + std::to_string(id);

    auto wiersz = menedzerBD.pobierzWiersz(zapytanie);

    if (wiersz.empty()) {
        return nullptr;
    }

    return std::make_unique<Zajecia>(utworzZajeciaZWiersza(wiersz));
}

int ZajeciaDAO::dodajZajecia(const Zajecia& zajecia) {
    std::stringstream ss;
    ss << "INSERT INTO gym_classes (name, trainer, max_participants, date, time, duration, description) VALUES ('"
        << zajecia.pobierzNazwe() << "', '"
        << zajecia.pobierzTrenera() << "', "
        << zajecia.pobierzMaksUczestnikow() << ", '"
        << zajecia.pobierzDate() << "', '"
        << zajecia.pobierzCzas() << "', "
        << zajecia.pobierzCzasTrwania() << ", '"
        << zajecia.pobierzOpis() << "')";

    return menedzerBD.wykonajZapytanieZwracajaceId(ss.str());
}

bool ZajeciaDAO::aktualizujZajecia(const Zajecia& zajecia) {
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

    try {
        menedzerBD.wykonajZapytanie(ss.str());
        return true;
    }
    catch (const std::exception&) {
        return false;
    }
}

bool ZajeciaDAO::usunZajecia(int id) {
    std::string zapytanie = "DELETE FROM gym_classes WHERE id = " + std::to_string(id);

    try {
        menedzerBD.wykonajZapytanie(zapytanie);
        return true;
    }
    catch (const std::exception&) {
        return false;
    }
}

std::vector<Rezerwacja> ZajeciaDAO::pobierzWszystkieRezerwacje() {
    std::string zapytanie = "SELECT id, client_id, class_id, reservation_date, status FROM reservations ORDER BY reservation_date DESC";

    auto wyniki = menedzerBD.pobierzDane(zapytanie);
    std::vector<Rezerwacja> rezerwacje;

    for (const auto& wiersz : wyniki) {
        rezerwacje.push_back(utworzRezerwacjeZWiersza(wiersz));
    }

    return rezerwacje;
}

std::vector<Rezerwacja> ZajeciaDAO::pobierzRezerwacjeKlienta(int idKlienta) {
    std::string zapytanie = "SELECT id, client_id, class_id, reservation_date, status FROM reservations WHERE client_id = " + std::to_string(idKlienta) + " ORDER BY reservation_date DESC";

    auto wyniki = menedzerBD.pobierzDane(zapytanie);
    std::vector<Rezerwacja> rezerwacje;

    for (const auto& wiersz : wyniki) {
        rezerwacje.push_back(utworzRezerwacjeZWiersza(wiersz));
    }

    return rezerwacje;
}

std::vector<Rezerwacja> ZajeciaDAO::pobierzRezerwacjeZajec(int idZajec) {
    std::string zapytanie = "SELECT id, client_id, class_id, reservation_date, status FROM reservations WHERE class_id = " + std::to_string(idZajec) + " AND status = 'potwierdzona' ORDER BY reservation_date";

    auto wyniki = menedzerBD.pobierzDane(zapytanie);
    std::vector<Rezerwacja> rezerwacje;

    for (const auto& wiersz : wyniki) {
        rezerwacje.push_back(utworzRezerwacjeZWiersza(wiersz));
    }

    return rezerwacje;
}

int ZajeciaDAO::dodajRezerwacje(const Rezerwacja& rezerwacja) {
    std::stringstream ss;
    ss << "INSERT INTO reservations (client_id, class_id, reservation_date, status) VALUES ("
        << rezerwacja.pobierzIdKlienta() << ", "
        << rezerwacja.pobierzIdZajec() << ", '"
        << (rezerwacja.pobierzDateRezerwacji().empty() ? Rezerwacja::pobierzAktualnyCzas() : rezerwacja.pobierzDateRezerwacji()) << "', '"
        << rezerwacja.pobierzStatus() << "')";

    return menedzerBD.wykonajZapytanieZwracajaceId(ss.str());
}

bool ZajeciaDAO::aktualizujRezerwacje(const Rezerwacja& rezerwacja) {
    std::stringstream ss;
    ss << "UPDATE reservations SET "
        << "client_id = " << rezerwacja.pobierzIdKlienta() << ", "
        << "class_id = " << rezerwacja.pobierzIdZajec() << ", "
        << "reservation_date = '" << rezerwacja.pobierzDateRezerwacji() << "', "
        << "status = '" << rezerwacja.pobierzStatus() << "' "
        << "WHERE id = " << rezerwacja.pobierzId();

    try {
        menedzerBD.wykonajZapytanie(ss.str());
        return true;
    }
    catch (const std::exception&) {
        return false;
    }
}

bool ZajeciaDAO::usunRezerwacje(int id) {
    std::string zapytanie = "DELETE FROM reservations WHERE id = " + std::to_string(id);

    try {
        menedzerBD.wykonajZapytanie(zapytanie);
        return true;
    }
    catch (const std::exception&) {
        return false;
    }
}

int ZajeciaDAO::policzRezerwacjeZajec(int idZajec) {
    std::string zapytanie = "SELECT COUNT(*) FROM reservations WHERE class_id = " + std::to_string(idZajec) + " AND status = 'potwierdzona'";

    std::string wynik = menedzerBD.pobierzWartosc(zapytanie);

    return wynik.empty() ? 0 : std::stoi(wynik);
}

Zajecia ZajeciaDAO::utworzZajeciaZWiersza(const WierszBD& wiersz) {
    if (wiersz.size() < 8) {
        throw std::runtime_error("Nieprawid³owy wiersz danych zajêæ");
    }

    return Zajecia(
        std::stoi(wiersz[0]),   // id
        wiersz[1],              // name
        wiersz[2],              // trainer
        std::stoi(wiersz[3]),   // max_participants
        wiersz[4],              // date
        wiersz[5],              // time
        std::stoi(wiersz[6]),   // duration
        wiersz[7]               // description
    );
}

Rezerwacja ZajeciaDAO::utworzRezerwacjeZWiersza(const WierszBD& wiersz) {
    if (wiersz.size() < 5) {
        throw std::runtime_error("Nieprawid³owy wiersz danych rezerwacji");
    }

    return Rezerwacja(
        std::stoi(wiersz[0]),   // id
        std::stoi(wiersz[1]),   // client_id
        std::stoi(wiersz[2]),   // class_id
        wiersz[3],              // reservation_date
        wiersz[4]               // status
    );
}