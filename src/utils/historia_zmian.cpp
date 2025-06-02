#define _CRT_SECURE_NO_WARNINGS
#include "historia_zmian.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <iomanip>

HistoriaZmian::HistoriaZmian(MenedzerBD& menedzerBD) : menedzerBD(menedzerBD) {
    inicjalizujTabele();
}

bool HistoriaZmian::inicjalizujTabele() {
    try {
        // Tabela logów operacji
        std::string sqlLogi = R"(
            CREATE TABLE IF NOT EXISTS logi_operacji (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                typ_operacji TEXT NOT NULL,
                tabela TEXT NOT NULL,
                id_rekordu INTEGER,
                dane_przed TEXT,
                dane_po TEXT,
                uzytkownik TEXT DEFAULT 'system',
                czas_operacji DATETIME DEFAULT CURRENT_TIMESTAMP,
                opis TEXT
            )
        )";

        // Tabela punktów przywracania
        std::string sqlPunkty = R"(
            CREATE TABLE IF NOT EXISTS punkty_przywracania (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                nazwa TEXT NOT NULL,
                opis TEXT,
                czas_utworzenia DATETIME DEFAULT CURRENT_TIMESTAMP
            )
        )";

        menedzerBD.wykonajZapytanie(sqlLogi);
        menedzerBD.wykonajZapytanie(sqlPunkty);
        return true;

    }
    catch (const std::exception& e) {
        std::cerr << "Błąd inicjalizacji tabel historii: " << e.what() << std::endl;
        return false;
    }
}

int HistoriaZmian::logujOperacje(const std::string& typOperacji, const std::string& tabela,
    int idRekordu, const std::string& danePrzed, const std::string& danePo,
    const std::string& opis) {
    try {
        std::stringstream ssql;
        ssql << "INSERT INTO logi_operacji "
            << "(typ_operacji, tabela, id_rekordu, dane_przed, dane_po, czas_operacji, opis) "
            << "VALUES ('"
            << escapujJSON(typOperacji) << "', '"
            << escapujJSON(tabela) << "', "
            << idRekordu << ", '"
            << escapujJSON(danePrzed) << "', '"
            << escapujJSON(danePo) << "', '"
            << pobierzAktualnyCzas() << "', '"
            << escapujJSON(opis) << "')";

        return menedzerBD.wykonajZapytanieZwracajaceId(ssql.str());

    }
    catch (const std::exception& e) {
        std::cerr << "Błąd logowania operacji: " << e.what() << std::endl;
        return -1;
    }
}

bool HistoriaZmian::cofnijOstatnia() {
    try {
        auto logi = pobierzHistorie(1);
        if (logi.empty()) {
            std::cout << "Brak operacji do cofnięcia." << std::endl;
            return false;
        }

        return cofnijOperacje(logi[0].id);

    }
    catch (const std::exception& e) {
        std::cerr << "Błąd cofania ostatniej operacji: " << e.what() << std::endl;
        return false;
    }
}

bool HistoriaZmian::cofnijOperacje(int idOperacji) {
    try {
        // Pobierz szczegóły operacji
        std::stringstream sql;
        sql << "SELECT * FROM logi_operacji WHERE id = " << idOperacji;

        auto wyniki = menedzerBD.pobierzDane(sql.str());
        if (wyniki.empty()) {
            std::cout << "Nie znaleziono operacji o ID: " << idOperacji << std::endl;
            return false;
        }

        LogOperacji log = utworzLogZWiersza(wyniki[0]);

        bool sukces = false;

        if (log.typOperacji == "INSERT") {
            sukces = cofnijInsert(log);
        }
        else if (log.typOperacji == "UPDATE") {
            sukces = cofnijUpdate(log);
        }
        else if (log.typOperacji == "DELETE") {
            sukces = cofnijDelete(log);
        }
        else {
            std::cout << "Nieznany typ operacji: " << log.typOperacji << std::endl;
            return false;
        }

        if (sukces) {
            // Zaloguj operację cofnięcia
            logujOperacje("UNDO", log.tabela, log.idRekordu, log.danePo, log.danePrzed,
                "Cofnięto operację ID: " + std::to_string(idOperacji));

            std::cout << "Pomyślnie cofnięto operację ID: " << idOperacji << std::endl;
        }

        return sukces;

    }
    catch (const std::exception& e) {
        std::cerr << "Błąd cofania operacji: " << e.what() << std::endl;
        return false;
    }
}

int HistoriaZmian::utworzPunktPrzywracania(const std::string& nazwa, const std::string& opis) {
    try {
        std::stringstream sql;
        sql << "INSERT INTO punkty_przywracania (nazwa, opis, czas_utworzenia) "
            << "VALUES ('"
            << escapujJSON(nazwa) << "', '"
            << escapujJSON(opis) << "', '"
            << pobierzAktualnyCzas() << "')";

        int id = menedzerBD.wykonajZapytanieZwracajaceId(sql.str());
        if (id > 0) {
            std::cout << "Utworzono punkt przywracania \"" << nazwa << "\" (ID: " << id << ")" << std::endl;
        }
        return id;

    }
    catch (const std::exception& e) {
        std::cerr << "Błąd tworzenia punktu przywracania: " << e.what() << std::endl;
        return -1;
    }
}

bool HistoriaZmian::przywrocDoPunktu(int idPunktu) {
    try {
        // Pobierz informacje o punkcie przywracania
        std::stringstream sql;
        sql << "SELECT * FROM punkty_przywracania WHERE id = " << idPunktu;

        auto wyniki = menedzerBD.pobierzDane(sql.str());
        if (wyniki.empty()) {
            std::cout << "Nie znaleziono punktu przywracania o ID: " << idPunktu << std::endl;
            return false;
        }

        PunktPrzywracania punkt = utworzPunktZWiersza(wyniki[0]);

        // Pobierz wszystkie operacje wykonane po utworzeniu punktu
        std::stringstream sqlOperacje;
        sqlOperacje << "SELECT * FROM logi_operacji "
            << "WHERE czas_operacji > '" << punkt.czasUtworzenia << "' "
            << "AND typ_operacji != 'UNDO' "
            << "ORDER BY id DESC";

        auto operacje = menedzerBD.pobierzDane(sqlOperacje.str());

        std::cout << "Cofanie " << operacje.size() << " operacji do punktu \""
            << punkt.nazwa << "\"..." << std::endl;

        int liczbaCofnietych = 0;

        // Cofnij operacje w odwrotnej kolejności
        for (const auto& wiersz : operacje) {
            LogOperacji log = utworzLogZWiersza(wiersz);

            bool sukces = false;
            if (log.typOperacji == "INSERT") {
                sukces = cofnijInsert(log);
            }
            else if (log.typOperacji == "UPDATE") {
                sukces = cofnijUpdate(log);
            }
            else if (log.typOperacji == "DELETE") {
                sukces = cofnijDelete(log);
            }

            if (sukces) {
                liczbaCofnietych++;
            }
            else {
                std::cerr << "Błąd cofania operacji ID: " << log.id << std::endl;
            }
        }

        // Zaloguj przywrócenie
        logujOperacje("RESTORE", "system", idPunktu, "", "",
            "Przywrócono do punktu: " + punkt.nazwa + " (cofnięto " +
            std::to_string(liczbaCofnietych) + " operacji)");

        std::cout << "Przywrócono do punktu \"" << punkt.nazwa
            << "\". Cofnięto " << liczbaCofnietych << " operacji." << std::endl;

        return liczbaCofnietych > 0;

    }
    catch (const std::exception& e) {
        std::cerr << "Błąd przywracania do punktu: " << e.what() << std::endl;
        return false;
    }
}

std::vector<LogOperacji> HistoriaZmian::pobierzHistorie(int limit) {
    std::vector<LogOperacji> historia;

    try {
        std::stringstream sql;
        sql << "SELECT * FROM logi_operacji ORDER BY id DESC";
        if (limit > 0) {
            sql << " LIMIT " << limit;
        }

        auto wyniki = menedzerBD.pobierzDane(sql.str());

        for (const auto& wiersz : wyniki) {
            historia.push_back(utworzLogZWiersza(wiersz));
        }

    }
    catch (const std::exception& e) {
        std::cerr << "Błąd pobierania historii: " << e.what() << std::endl;
    }

    return historia;
}

std::vector<LogOperacji> HistoriaZmian::pobierzHistorieTabeli(const std::string& tabela, int limit) {
    std::vector<LogOperacji> historia;

    try {
        std::stringstream sql;
        sql << "SELECT * FROM logi_operacji WHERE tabela = '" << escapujJSON(tabela)
            << "' ORDER BY id DESC";
        if (limit > 0) {
            sql << " LIMIT " << limit;
        }

        auto wyniki = menedzerBD.pobierzDane(sql.str());

        for (const auto& wiersz : wyniki) {
            historia.push_back(utworzLogZWiersza(wiersz));
        }

    }
    catch (const std::exception& e) {
        std::cerr << "Błąd pobierania historii tabeli: " << e.what() << std::endl;
    }

    return historia;
}

std::vector<LogOperacji> HistoriaZmian::pobierzHistorieRekordu(const std::string& tabela, int idRekordu) {
    std::vector<LogOperacji> historia;

    try {
        std::stringstream sql;
        sql << "SELECT * FROM logi_operacji WHERE tabela = '" << escapujJSON(tabela)
            << "' AND id_rekordu = " << idRekordu << " ORDER BY id DESC";

        auto wyniki = menedzerBD.pobierzDane(sql.str());

        for (const auto& wiersz : wyniki) {
            historia.push_back(utworzLogZWiersza(wiersz));
        }

    }
    catch (const std::exception& e) {
        std::cerr << "Błąd pobierania historii rekordu: " << e.what() << std::endl;
    }

    return historia;
}

std::vector<PunktPrzywracania> HistoriaZmian::pobierzPunktyPrzywracania() {
    std::vector<PunktPrzywracania> punkty;

    try {
        auto wyniki = menedzerBD.pobierzDane("SELECT * FROM punkty_przywracania ORDER BY id DESC");

        for (const auto& wiersz : wyniki) {
            punkty.push_back(utworzPunktZWiersza(wiersz));
        }

    }
    catch (const std::exception& e) {
        std::cerr << "Błąd pobierania punktów przywracania: " << e.what() << std::endl;
    }

    return punkty;
}

bool HistoriaZmian::usunPunktPrzywracania(int idPunktu) {
    try {
        std::string zapytanie = "DELETE FROM punkty_przywracania WHERE id = " + std::to_string(idPunktu);
        menedzerBD.wykonajZapytanie(zapytanie);
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Błąd usuwania punktu przywracania: " << e.what() << std::endl;
        return false;
    }
}

std::string HistoriaZmian::generujRaportHistorii() {
    std::stringstream raport;

    raport << "========================================\n";
    raport << "       RAPORT HISTORII ZMIAN            \n";
    raport << "========================================\n";
    raport << "Data wygenerowania: " << pobierzAktualnyCzas() << "\n";
    raport << "----------------------------------------\n\n";

    // Statystyki ogólne
    int wszystkieOperacje = policzOperacje();
    int operacjeInsert = policzOperacje("INSERT");
    int operacjeUpdate = policzOperacje("UPDATE");
    int operacjeDelete = policzOperacje("DELETE");
    int operacjeUndo = policzOperacje("UNDO");

    raport << "STATYSTYKI OGÓLNE:\n";
    raport << "==================\n";
    raport << "• Łączna liczba operacji: " << wszystkieOperacje << "\n";
    raport << "• Dodane rekordy (INSERT): " << operacjeInsert << "\n";
    raport << "• Zaktualizowane rekordy (UPDATE): " << operacjeUpdate << "\n";
    raport << "• Usunięte rekordy (DELETE): " << operacjeDelete << "\n";
    raport << "• Cofnięte operacje (UNDO): " << operacjeUndo << "\n\n";

    // Ostatnie operacje
    auto ostatnieOperacje = pobierzHistorie(10);

    raport << "OSTATNIE 10 OPERACJI:\n";
    raport << "=====================\n";

    if (ostatnieOperacje.empty()) {
        raport << "Brak operacji w historii.\n\n";
    }
    else {
        for (const auto& operacja : ostatnieOperacje) {
            raport << "• ID: " << operacja.id
                << " | " << operacja.typOperacji
                << " | " << operacja.tabela;

            if (operacja.idRekordu > 0) {
                raport << " | Rekord ID: " << operacja.idRekordu;
            }

            raport << " | " << operacja.czasOperacji;

            if (!operacja.opis.empty()) {
                raport << " | " << operacja.opis;
            }

            raport << "\n";
        }
        raport << "\n";
    }

    // Punkty przywracania
    auto punkty = pobierzPunktyPrzywracania();

    raport << "PUNKTY PRZYWRACANIA:\n";
    raport << "====================\n";

    if (punkty.empty()) {
        raport << "Brak punktów przywracania.\n\n";
    }
    else {
        for (const auto& punkt : punkty) {
            raport << "• ID: " << punkt.id
                << " | \"" << punkt.nazwa << "\""
                << " | " << punkt.czasUtworzenia;

            if (!punkt.opis.empty()) {
                raport << " | " << punkt.opis;
            }

            raport << "\n";
        }
        raport << "\n";
    }

    raport << "========================================\n";
    raport << "Koniec raportu\n";

    return raport.str();
}

bool HistoriaZmian::wyczyscStaraHistorie(int dniDoZachowania) {
    try {
        std::stringstream sql;
        sql << "DELETE FROM logi_operacji "
            << "WHERE czas_operacji < datetime('now', '-" << dniDoZachowania << " days')";

        menedzerBD.wykonajZapytanie(sql.str());

        logujOperacje("CLEANUP", "logi_operacji", 0, "", "",
            "Wyczyszczono historię starszą niż " + std::to_string(dniDoZachowania) + " dni");

        return true;

    }
    catch (const std::exception& e) {
        std::cerr << "Błąd czyszczenia historii: " << e.what() << std::endl;
        return false;
    }
}

bool HistoriaZmian::wyczyscWszystko() {
    try {
        menedzerBD.wykonajZapytanie("DELETE FROM logi_operacji");
        menedzerBD.wykonajZapytanie("DELETE FROM punkty_przywracania");
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Błąd czyszczenia całej historii: " << e.what() << std::endl;
        return false;
    }
}

// Metody pomocnicze

std::string HistoriaZmian::pobierzAktualnyCzas() {
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

std::string HistoriaZmian::escapujJSON(const std::string& str) {
    std::string escaped;
    escaped.reserve(str.size());

    for (char c : str) {
        switch (c) {
        case '"': escaped += "\\\""; break;
        case '\\': escaped += "\\\\"; break;
        case '\n': escaped += "\\n"; break;
        case '\r': escaped += "\\r"; break;
        case '\t': escaped += "\\t"; break;
        case '\'': escaped += "''"; break; // Escape dla SQL
        default: escaped += c; break;
        }
    }

    return escaped;
}

LogOperacji HistoriaZmian::utworzLogZWiersza(const WierszBD& wiersz) {
    LogOperacji log;

    if (wiersz.size() >= 9) {
        log.id = std::stoi(wiersz[0]);
        log.typOperacji = wiersz[1];
        log.tabela = wiersz[2];
        log.idRekordu = wiersz[3].empty() ? 0 : std::stoi(wiersz[3]);
        log.danePrzed = wiersz[4];
        log.danePo = wiersz[5];
        log.uzytkownik = wiersz[6];
        log.czasOperacji = wiersz[7];
        log.opis = wiersz[8];
    }

    return log;
}

PunktPrzywracania HistoriaZmian::utworzPunktZWiersza(const WierszBD& wiersz) {
    PunktPrzywracania punkt;

    if (wiersz.size() >= 4) {
        punkt.id = std::stoi(wiersz[0]);
        punkt.nazwa = wiersz[1];
        punkt.opis = wiersz[2];
        punkt.czasUtworzenia = wiersz[3];
    }

    return punkt;
}

int HistoriaZmian::policzOperacje(const std::string& typOperacji) {
    try {
        std::string sql = "SELECT COUNT(*) FROM logi_operacji";
        if (!typOperacji.empty()) {
            sql += " WHERE typ_operacji = '" + escapujJSON(typOperacji) + "'";
        }

        std::string wynik = menedzerBD.pobierzWartosc(sql);
        return wynik.empty() ? 0 : std::stoi(wynik);

    }
    catch (const std::exception& e) {
        std::cerr << "Błąd liczenia operacji: " << e.what() << std::endl;
        return 0;
    }
}

// Metody cofania operacji (uproszczone - w rzeczywistości wymagałyby pełnej implementacji)

bool HistoriaZmian::cofnijInsert(const LogOperacji& log) {
    try {
        // Usuń rekord, który został dodany
        std::stringstream sql;
        sql << "DELETE FROM " << log.tabela << " WHERE id = " << log.idRekordu;

        menedzerBD.wykonajZapytanie(sql.str());
        return true;

    }
    catch (const std::exception& e) {
        std::cerr << "Błąd cofania INSERT: " << e.what() << std::endl;
        return false;
    }
}

bool HistoriaZmian::cofnijUpdate(const LogOperacji& log) {
    // UWAGA: To jest uproszczona implementacja
    // W prawdziwej aplikacji trzeba by parsować JSON i przywracać wszystkie pola

    try {
        std::cout << "Cofanie UPDATE dla tabeli " << log.tabela
            << ", rekord ID: " << log.idRekordu << std::endl;
        std::cout << "UWAGA: Funkcja cofania UPDATE wymaga dalszej implementacji" << std::endl;

        // Tutaj powinno być parsowanie JSON z danePrzed i przywracanie wszystkich pól
        return true; // Tymczasowo zwracamy true

    }
    catch (const std::exception& e) {
        std::cerr << "Błąd cofania UPDATE: " << e.what() << std::endl;
        return false;
    }
}

bool HistoriaZmian::cofnijDelete(const LogOperacji& log) {
    try {
        std::cout << "Cofanie DELETE dla tabeli " << log.tabela
            << ", rekord ID: " << log.idRekordu << std::endl;
        std::cout << "UWAGA: Funkcja cofania DELETE wymaga dalszej implementacji" << std::endl;

        // Tutaj powinno być parsowanie JSON z danePrzed i odtworzenie rekordu
        return true; // Tymczasowo zwracamy true

    }
    catch (const std::exception& e) {
        std::cerr << "Błąd cofania DELETE: " << e.what() << std::endl;
        return false;
    }
}

// Implementacja klasy AutoLogger

AutoLogger::AutoLogger(HistoriaZmian& historia, const std::string& tabela, int idRekordu, const std::string& danePrzed)
    : historia(historia), tabela(tabela), idRekordu(idRekordu), danePrzed(danePrzed),
    typOperacji("UPDATE"), anulowano(false) {
}

AutoLogger::~AutoLogger() {
    if (!anulowano && !danePo.empty()) {
        historia.logujOperacje(typOperacji, tabela, idRekordu, danePrzed, danePo, opis);
    }
}

void AutoLogger::ustawDanePo(const std::string& danePo) {
    this->danePo = danePo;
}

void AutoLogger::ustawTypOperacji(const std::string& typ) {
    this->typOperacji = typ;
}

void AutoLogger::ustawOpis(const std::string& opis) {
    this->opis = opis;
}

void AutoLogger::anuluj() {
    this->anulowano = true;
}