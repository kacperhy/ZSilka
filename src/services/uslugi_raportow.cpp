#include "uslugi_raportow.h"
#include <algorithm>

UslugiRaportow::UslugiRaportow(MenedzerBD& menedzerBD) : menedzerBD(menedzerBD) {
}

std::vector<RaportAktywnosciKlienta> UslugiRaportow::generujRaportAktywnosciKlienta(
    const std::string& dataOd, const std::string& dataDo) {

    std::vector<RaportAktywnosciKlienta> wyniki;

    std::string zapytanie =
        "SELECT c.id, c.first_name || ' ' || c.last_name AS 'customer_name', "
        "COUNT(r.id) AS 'total_classes', "
        "SUM(CASE WHEN r.status = 'anulowana' THEN 1 ELSE 0 END) AS 'cancelled_classes', "
        "(SELECT gc.name FROM gym_classes gc "
        "JOIN reservations r2 ON gc.id = r2.class_id "
        "WHERE r2.client_id = c.id AND r2.reservation_date BETWEEN '" + dataOd + "' AND '" + dataDo + "' "
        "GROUP BY gc.name ORDER BY COUNT(*) DESC LIMIT 1) AS 'most_frequent_class', "
        "MAX(r.reservation_date) AS 'last_visit' "
        "FROM clients c "
        "LEFT JOIN reservations r ON c.id = r.client_id "
        "WHERE r.reservation_date BETWEEN '" + dataOd + "' AND '" + dataDo + "' "
        "GROUP BY c.id "
        "ORDER BY total_classes DESC";

    auto dane = menedzerBD.pobierzDane(zapytanie);

    for (const auto& wiersz : dane) {
        RaportAktywnosciKlienta raport;
        raport.nazwaKlienta = wiersz[1];
        raport.lacznaLiczbaZajec = std::stoi(wiersz[2]);
        raport.liczbaAnulowanychZajec = std::stoi(wiersz[3]);
        raport.najczestszaZajecia = wiersz[4];
        raport.ostatniaWizyta = wiersz[5];
        wyniki.push_back(raport);
    }

    return wyniki;
}

std::vector<RaportPopularnosciZajec> UslugiRaportow::generujRaportPopularnosciZajec(
    const std::string& dataOd, const std::string& dataDo) {

    std::vector<RaportPopularnosciZajec> wyniki;

    std::string zapytanie =
        "SELECT gc.id, gc.name, gc.trainer, "
        "COUNT(r.id) AS 'total_reservations', "
        "100.0 * COUNT(r.id) / gc.max_participants AS 'fill_rate' "
        "FROM gym_classes gc "
        "LEFT JOIN reservations r ON gc.id = r.class_id "
        "WHERE gc.date BETWEEN '" + dataOd + "' AND '" + dataDo + "' "
        "AND (r.status = 'potwierdzona' OR r.status IS NULL) "
        "GROUP BY gc.id "
        "ORDER BY total_reservations DESC";

    auto dane = menedzerBD.pobierzDane(zapytanie);

    for (const auto& wiersz : dane) {
        RaportPopularnosciZajec raport;
        raport.nazwaZajec = wiersz[1];
        raport.trener = wiersz[2];
        raport.lacznaLiczbaRezerwacji = std::stoi(wiersz[3]);
        raport.stopienWypelnienia = std::stod(wiersz[4]);
        wyniki.push_back(raport);
    }

    return wyniki;
}

RaportFinansowy UslugiRaportow::generujRaportFinansowy(
    const std::string& dataOd, const std::string& dataDo) {

    RaportFinansowy raport;

    std::string zapytanieSuma =
        "SELECT SUM(price) AS 'total_revenue', COUNT(*) AS 'total_memberships' "
        "FROM memberships "
        "WHERE start_date BETWEEN '" + dataOd + "' AND '" + dataDo + "'";

    auto daneSuma = menedzerBD.pobierzWiersz(zapytanieSuma);

    if (!daneSuma.empty()) {
        raport.lacznyPrzychod = std::stod(daneSuma[0]);
        raport.lacznaLiczbaKarnetow = std::stoi(daneSuma[1]);
    }

    std::string zapytanieTypy =
        "SELECT "
        "SUM(CASE WHEN type LIKE 'student_%' THEN price ELSE 0 END) AS 'student_revenue', "
        "SUM(CASE WHEN type LIKE 'normalny_%' THEN price ELSE 0 END) AS 'standard_revenue' "
        "FROM memberships "
        "WHERE start_date BETWEEN '" + dataOd + "' AND '" + dataDo + "'";

    auto daneTypy = menedzerBD.pobierzWiersz(zapytanieTypy);

    if (!daneTypy.empty()) {
        raport.przychodStudencki = std::stod(daneTypy[0]);
        raport.przychodStandardowy = std::stod(daneTypy[1]);
    }

    std::string zapytanieWgTypu =
        "SELECT type, SUM(price) AS 'revenue', COUNT(*) AS 'count' "
        "FROM memberships "
        "WHERE start_date BETWEEN '" + dataOd + "' AND '" + dataDo + "' "
        "GROUP BY type";

    auto daneWgTypu = menedzerBD.pobierzDane(zapytanieWgTypu);

    for (const auto& wiersz : daneWgTypu) {
        std::string typ = wiersz[0];
        double przychod = std::stod(wiersz[1]);
        int liczba = std::stoi(wiersz[2]);

        raport.przychodWgTypuKarnetu[typ] = przychod;
        raport.liczbaKarnetowWgTypu[typ] = liczba;
    }

    return raport;
}