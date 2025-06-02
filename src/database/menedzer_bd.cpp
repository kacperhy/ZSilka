#include "menedzer_bd.h"
#include <iostream>

MenedzerBD::MenedzerBD(const std::string& sciezkaPliku)
    : sciezkaPliku(sciezkaPliku), polaczenie(nullptr) {
    otworz();
    inicjalizujBazeDanych();
}

MenedzerBD::~MenedzerBD() {
    zamknij();
}

void MenedzerBD::otworz() {
    int kod = sqlite3_open(sciezkaPliku.c_str(), &polaczenie);
    sprawdzBlad(kod, "Otwieranie bazy danych");
}

void MenedzerBD::zamknij() {
    if (polaczenie) {
        sqlite3_close(polaczenie);
        polaczenie = nullptr;
    }
}

bool MenedzerBD::czyOtwarta() const {
    return polaczenie != nullptr;
}

void MenedzerBD::wykonajZapytanie(const std::string& zapytanie) {
    char* bladWiadomosc = nullptr;
    int kod = sqlite3_exec(polaczenie, zapytanie.c_str(), nullptr, nullptr, &bladWiadomosc);

    if (kod != SQLITE_OK) {
        std::string blad = bladWiadomosc ? bladWiadomosc : "Nieznany b³¹d";
        sqlite3_free(bladWiadomosc);
        throw WyjatekBazyDanych("B³¹d wykonywania zapytania: " + blad);
    }
}

int MenedzerBD::wykonajZapytanieZwracajaceId(const std::string& zapytanie) {
    wykonajZapytanie(zapytanie);
    return static_cast<int>(sqlite3_last_insert_rowid(polaczenie));
}

TabelaBD MenedzerBD::pobierzDane(const std::string& zapytanie) {
    sqlite3_stmt* stmt;
    int kod = sqlite3_prepare_v2(polaczenie, zapytanie.c_str(), -1, &stmt, nullptr);
    sprawdzBlad(kod, "Przygotowywanie zapytania");

    TabelaBD wyniki;

    while ((kod = sqlite3_step(stmt)) == SQLITE_ROW) {
        WierszBD wiersz;
        int liczbaKolumn = sqlite3_column_count(stmt);

        for (int i = 0; i < liczbaKolumn; i++) {
            const char* tekst = reinterpret_cast<const char*>(sqlite3_column_text(stmt, i));
            wiersz.push_back(tekst ? tekst : "");
        }

        wyniki.push_back(wiersz);
    }

    if (kod != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        sprawdzBlad(kod, "Wykonywanie zapytania");
    }

    sqlite3_finalize(stmt);
    return wyniki;
}

WierszBD MenedzerBD::pobierzWiersz(const std::string& zapytanie) {
    auto wyniki = pobierzDane(zapytanie);
    return wyniki.empty() ? WierszBD() : wyniki[0];
}

std::string MenedzerBD::pobierzWartosc(const std::string& zapytanie) {
    auto wiersz = pobierzWiersz(zapytanie);
    return wiersz.empty() ? "" : wiersz[0];
}

TabelaBD MenedzerBD::pobierzDaneZParametrami(const std::string& zapytanie,
    const std::vector<ParamZapytania>& parametry) {
    sqlite3_stmt* stmt;
    int kod = sqlite3_prepare_v2(polaczenie, zapytanie.c_str(), -1, &stmt, nullptr);
    sprawdzBlad(kod, "Przygotowywanie zapytania z parametrami");

    bindParameters(stmt, parametry);

    TabelaBD wyniki;

    while ((kod = sqlite3_step(stmt)) == SQLITE_ROW) {
        WierszBD wiersz;
        int liczbaKolumn = sqlite3_column_count(stmt);

        for (int i = 0; i < liczbaKolumn; i++) {
            const char* tekst = reinterpret_cast<const char*>(sqlite3_column_text(stmt, i));
            wiersz.push_back(tekst ? tekst : "");
        }

        wyniki.push_back(wiersz);
    }

    if (kod != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        sprawdzBlad(kod, "Wykonywanie zapytania z parametrami");
    }

    sqlite3_finalize(stmt);
    return wyniki;
}

WierszBD MenedzerBD::pobierzWierszZParametrami(const std::string& zapytanie,
    const std::vector<ParamZapytania>& parametry) {
    auto wyniki = pobierzDaneZParametrami(zapytanie, parametry);
    return wyniki.empty() ? WierszBD() : wyniki[0];
}

std::string MenedzerBD::pobierzWartoscZParametrami(const std::string& zapytanie,
    const std::vector<ParamZapytania>& parametry) {
    auto wiersz = pobierzWierszZParametrami(zapytanie, parametry);
    return wiersz.empty() ? "" : wiersz[0];
}

void MenedzerBD::inicjalizujBazeDanych() {
    // Tworzenie tabeli klientów
    std::string sqlKlienci = R"(
        CREATE TABLE IF NOT EXISTS clients (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            first_name TEXT NOT NULL,
            last_name TEXT NOT NULL,
            email TEXT,
            phone TEXT,
            birth_date TEXT,
            registration_date TEXT DEFAULT (datetime('now', 'localtime')),
            notes TEXT
        )
    )";

    // Tworzenie tabeli karnetów
    std::string sqlKarnety = R"(
        CREATE TABLE IF NOT EXISTS memberships (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            client_id INTEGER NOT NULL,
            type TEXT NOT NULL,
            start_date TEXT NOT NULL,
            end_date TEXT NOT NULL,
            price REAL NOT NULL,
            is_active INTEGER DEFAULT 1,
            FOREIGN KEY (client_id) REFERENCES clients (id)
        )
    )";

    // Tworzenie tabeli zajêæ
    std::string sqlZajecia = R"(
        CREATE TABLE IF NOT EXISTS gym_classes (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            trainer TEXT NOT NULL,
            max_participants INTEGER NOT NULL,
            date TEXT NOT NULL,
            time TEXT NOT NULL,
            duration INTEGER NOT NULL,
            description TEXT
        )
    )";

    // Tworzenie tabeli rezerwacji
    std::string sqlRezerwacje = R"(
        CREATE TABLE IF NOT EXISTS reservations (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            client_id INTEGER NOT NULL,
            class_id INTEGER NOT NULL,
            reservation_date TEXT DEFAULT (datetime('now', 'localtime')),
            status TEXT DEFAULT 'potwierdzona',
            FOREIGN KEY (client_id) REFERENCES clients (id),
            FOREIGN KEY (class_id) REFERENCES gym_classes (id)
        )
    )";

    try {
        wykonajZapytanie(sqlKlienci);
        wykonajZapytanie(sqlKarnety);
        wykonajZapytanie(sqlZajecia);
        wykonajZapytanie(sqlRezerwacje);
    }
    catch (const std::exception& e) {
        throw WyjatekBazyDanych("B³¹d inicjalizacji bazy danych: " + std::string(e.what()));
    }
}

void MenedzerBD::sprawdzBlad(int kod, const std::string& operacja) {
    if (kod != SQLITE_OK) {
        std::string bladWiadomosc = sqlite3_errmsg(polaczenie);
        throw WyjatekBazyDanych(operacja + ": " + bladWiadomosc);
    }
}

void MenedzerBD::bindParameters(sqlite3_stmt* stmt, const std::vector<ParamZapytania>& parametry) {
    for (const auto& param : parametry) {
        int kod = sqlite3_bind_text(stmt, param.first, param.second.c_str(), -1, SQLITE_STATIC);
        if (kod != SQLITE_OK) {
            throw WyjatekBazyDanych("B³¹d bindowania parametru: " + std::string(sqlite3_errmsg(polaczenie)));
        }
    }
}