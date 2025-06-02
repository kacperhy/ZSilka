#include "menedzer_bd.h"
#include <iostream>
#include <sstream>

MenedzerBD::MenedzerBD(const std::string& sciezkaPliku) : sciezkaPliku(sciezkaPliku), polaczenie(nullptr) {
}

MenedzerBD::~MenedzerBD() {
    zamknij();
}

void MenedzerBD::otworz() {
    if (polaczenie != nullptr) {
        return; // Już otwarte
    }

    int kod = sqlite3_open(sciezkaPliku.c_str(), &polaczenie);
    sprawdzBlad(kod, "Otwieranie bazy danych");

    // Włącz foreign keys
    wykonajZapytanie("PRAGMA foreign_keys = ON;");
}

void MenedzerBD::zamknij() {
    if (polaczenie != nullptr) {
        sqlite3_close(polaczenie);
        polaczenie = nullptr;
    }
}

bool MenedzerBD::czyOtwarta() const {
    return polaczenie != nullptr;
}

void MenedzerBD::wykonajZapytanie(const std::string& zapytanie) {
    if (!czyOtwarta()) {
        throw WyjatekBazyDanych("Baza danych nie jest otwarta");
    }

    char* errorMsg = nullptr;
    int kod = sqlite3_exec(polaczenie, zapytanie.c_str(), nullptr, nullptr, &errorMsg);

    if (kod != SQLITE_OK) {
        std::string error = errorMsg ? errorMsg : "Nieznany błąd";
        sqlite3_free(errorMsg);
        throw WyjatekBazyDanych("Błąd wykonania zapytania: " + error);
    }
}

int MenedzerBD::wykonajZapytanieZwracajaceId(const std::string& zapytanie) {
    wykonajZapytanie(zapytanie);
    return static_cast<int>(sqlite3_last_insert_rowid(polaczenie));
}

TabelaBD MenedzerBD::pobierzDane(const std::string& zapytanie) {
    return pobierzDaneZParametrami(zapytanie, {});
}

WierszBD MenedzerBD::pobierzWiersz(const std::string& zapytanie) {
    return pobierzWierszZParametrami(zapytanie, {});
}

std::string MenedzerBD::pobierzWartosc(const std::string& zapytanie) {
    return pobierzWartoscZParametrami(zapytanie, {});
}

TabelaBD MenedzerBD::pobierzDaneZParametrami(const std::string& zapytanie,
    const std::vector<ParamZapytania>& parametry) {

    if (!czyOtwarta()) {
        throw WyjatekBazyDanych("Baza danych nie jest otwarta");
    }

    sqlite3_stmt* stmt;
    int kod = sqlite3_prepare_v2(polaczenie, zapytanie.c_str(), -1, &stmt, nullptr);
    sprawdzBlad(kod, "Przygotowanie zapytania");

    // Binduj parametry
    bindParameters(stmt, parametry);

    TabelaBD wyniki;

    while ((kod = sqlite3_step(stmt)) == SQLITE_ROW) {
        WierszBD wiersz;
        int liczbKolumn = sqlite3_column_count(stmt);

        for (int i = 0; i < liczbKolumn; i++) {
            const char* wartosc = reinterpret_cast<const char*>(sqlite3_column_text(stmt, i));
            wiersz.push_back(wartosc ? wartosc : "");
        }

        wyniki.push_back(wiersz);
    }

    if (kod != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        sprawdzBlad(kod, "Wykonanie zapytania");
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
    // Tworzenie tabel
    
    // Tabela klientów
    std::string sqlKlienci = R"(
        CREATE TABLE IF NOT EXISTS clients (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            first_name TEXT NOT NULL,
            last_name TEXT NOT NULL,
            email TEXT,
            phone TEXT,
            birth_date TEXT,
            registration_date TEXT NOT NULL,
            notes TEXT
        )
    )";

    // Tabela karnetów
    std::string sqlKarnety = R"(
        CREATE TABLE IF NOT EXISTS memberships (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            client_id INTEGER NOT NULL,
            type TEXT NOT NULL,
            start_date TEXT NOT NULL,
            end_date TEXT NOT NULL,
            price REAL NOT NULL,
            is_active INTEGER NOT NULL DEFAULT 1,
            FOREIGN KEY (client_id) REFERENCES clients (id) ON DELETE CASCADE
        )
    )";

    // Tabela zajęć
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

    // Tabela rezerwacji
    std::string sqlRezerwacje = R"(
        CREATE TABLE IF NOT EXISTS reservations (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            client_id INTEGER NOT NULL,
            class_id INTEGER NOT NULL,
            reservation_date TEXT NOT NULL,
            status TEXT NOT NULL DEFAULT 'potwierdzona',
            FOREIGN KEY (client_id) REFERENCES clients (id) ON DELETE CASCADE,
            FOREIGN KEY (class_id) REFERENCES gym_classes (id) ON DELETE CASCADE
        )
    )";

    // Wykonaj tworzenie tabel
    wykonajZapytanie(sqlKlienci);
    wykonajZapytanie(sqlKarnety);
    wykonajZapytanie(sqlZajecia);
    wykonajZapytanie(sqlRezerwacje);

    // Tworzenie indeksów dla wydajności
    wykonajZapytanie("CREATE INDEX IF NOT EXISTS idx_clients_email ON clients(email);");
    wykonajZapytanie("CREATE INDEX IF NOT EXISTS idx_memberships_client ON memberships(client_id);");
    wykonajZapytanie("CREATE INDEX IF NOT EXISTS idx_memberships_dates ON memberships(start_date, end_date);");
    wykonajZapytanie("CREATE INDEX IF NOT EXISTS idx_classes_date ON gym_classes(date);");
    wykonajZapytanie("CREATE INDEX IF NOT EXISTS idx_reservations_client ON reservations(client_id);");
    wykonajZapytanie("CREATE INDEX IF NOT EXISTS idx_reservations_class ON reservations(class_id);");

    std::cout << "Baza danych została zainicjalizowana pomyślnie." << std::endl;
}

void MenedzerBD::sprawdzBlad(int kod, const std::string& operacja) {
    if (kod != SQLITE_OK) {
        std::string error = sqlite3_errmsg(polaczenie);
        throw WyjatekBazyDanych(operacja + ": " + error);
    }
}

void MenedzerBD::bindParameters(sqlite3_stmt* stmt, const std::vector<ParamZapytania>& parametry) {
    for (const auto& param : parametry) {
        int indeks = param.first;
        const std::string& wartosc = param.second;

        int kod = sqlite3_bind_text(stmt, indeks, wartosc.c_str(), -1, SQLITE_STATIC);
        if (kod != SQLITE_OK) {
            throw WyjatekBazyDanych("Błąd bindowania parametru " + std::to_string(indeks));
        }
    }
}