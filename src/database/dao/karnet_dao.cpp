#include "karnet_dao.h"
#include <sstream>

KarnetDAO::KarnetDAO(MenedzerBD& menedzerBD) : menedzerBD(menedzerBD) {
}

std::vector<Karnet> KarnetDAO::pobierzWszystkie() {
    std::string zapytanie = "SELECT id, client_id, type, start_date, end_date, price, is_active FROM memberships ORDER BY start_date DESC";

    auto wyniki = menedzerBD.pobierzDane(zapytanie);
    std::vector<Karnet> karnety;

    for (const auto& wiersz : wyniki) {
        karnety.push_back(utworzKarnetZWiersza(wiersz));
    }

    return karnety;
}

std::unique_ptr<Karnet> KarnetDAO::pobierzPoId(int id) {
    std::string zapytanie = "SELECT id, client_id, type, start_date, end_date, price, is_active FROM memberships WHERE id = " + std::to_string(id);

    auto wiersz = menedzerBD.pobierzWiersz(zapytanie);

    if (wiersz.empty()) {
        return nullptr;
    }

    return std::make_unique<Karnet>(utworzKarnetZWiersza(wiersz));
}

std::vector<Karnet> KarnetDAO::pobierzDlaKlienta(int idKlienta) {
    std::string zapytanie = "SELECT id, client_id, type, start_date, end_date, price, is_active FROM memberships WHERE client_id = " + std::to_string(idKlienta) + " ORDER BY start_date DESC";

    auto wyniki = menedzerBD.pobierzDane(zapytanie);
    std::vector<Karnet> karnety;

    for (const auto& wiersz : wyniki) {
        karnety.push_back(utworzKarnetZWiersza(wiersz));
    }

    return karnety;
}

int KarnetDAO::dodaj(const Karnet& karnet) {
    std::stringstream ss;
    ss << "INSERT INTO memberships (client_id, type, start_date, end_date, price, is_active) VALUES ("
        << karnet.pobierzIdKlienta() << ", '"
        << karnet.pobierzTyp() << "', '"
        << karnet.pobierzDateRozpoczecia() << "', '"
        << karnet.pobierzDateZakonczenia() << "', "
        << karnet.pobierzCene() << ", "
        << (karnet.pobierzCzyAktywny() ? 1 : 0) << ")";

    return menedzerBD.wykonajZapytanieZwracajaceId(ss.str());
}

bool KarnetDAO::aktualizuj(const Karnet& karnet) {
    std::stringstream ss;
    ss << "UPDATE memberships SET "
        << "client_id = " << karnet.pobierzIdKlienta() << ", "
        << "type = '" << karnet.pobierzTyp() << "', "
        << "start_date = '" << karnet.pobierzDateRozpoczecia() << "', "
        << "end_date = '" << karnet.pobierzDateZakonczenia() << "', "
        << "price = " << karnet.pobierzCene() << ", "
        << "is_active = " << (karnet.pobierzCzyAktywny() ? 1 : 0) << " "
        << "WHERE id = " << karnet.pobierzId();

    try {
        menedzerBD.wykonajZapytanie(ss.str());
        return true;
    }
    catch (const std::exception&) {
        return false;
    }
}

bool KarnetDAO::usun(int id) {
    std::string zapytanie = "DELETE FROM memberships WHERE id = " + std::to_string(id);

    try {
        menedzerBD.wykonajZapytanie(zapytanie);
        return true;
    }
    catch (const std::exception&) {
        return false;
    }
}

bool KarnetDAO::czyKlientMaAktywnyKarnet(int idKlienta) {
    std::string zapytanie = "SELECT COUNT(*) FROM memberships WHERE client_id = " + std::to_string(idKlienta) +
        " AND is_active = 1 AND date('now') BETWEEN start_date AND end_date";

    std::string wynik = menedzerBD.pobierzWartosc(zapytanie);

    return !wynik.empty() && std::stoi(wynik) > 0;
}

Karnet KarnetDAO::utworzKarnetZWiersza(const WierszBD& wiersz) {
    if (wiersz.size() < 7) {
        throw std::runtime_error("Nieprawid³owy wiersz danych karnetu");
    }

    return Karnet(
        std::stoi(wiersz[0]),           // id
        std::stoi(wiersz[1]),           // client_id
        wiersz[2],                      // type
        wiersz[3],                      // start_date
        wiersz[4],                      // end_date
        std::stod(wiersz[5]),           // price
        std::stoi(wiersz[6]) == 1       // is_active
    );
}