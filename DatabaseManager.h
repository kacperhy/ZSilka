#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QtSql/QSqlDatabase>
#include <QVariantList>
#include <QList>

struct Klient {
    int id;
    QString imie;
    QString nazwisko;
    QString email;
    QString telefon;
    QString dataUrodzenia;
    QString dataRejestracji;
    QString uwagi;
};

class DatabaseManager {
public:
    // === Podstawowe metody połączenia ===
    static bool connect(const QString& path);
    static void disconnect();
    static QSqlDatabase& instance();

    // === CRUD dla KLIENTÓW ===

    // Dodawanie nowego klienta
    static bool addKlient(const QString& imie,
                          const QString& nazwisko,
                          const QString& email = QString(),
                          const QString& telefon = QString(),
                          const QString& dataUrodzenia = QString(),
                          const QString& uwagi = QString());

    // Pobieranie wszystkich klientów
    static QList<Klient> getAllKlienci();

    // Pobieranie klienta po ID
    static Klient getKlientById(int id);

    // Aktualizacja danych klienta
    static bool updateKlient(int id,
                             const QString& imie,
                             const QString& nazwisko,
                             const QString& email = QString(),
                             const QString& telefon = QString(),
                             const QString& dataUrodzenia = QString(),
                             const QString& uwagi = QString());

    // Usuwanie klienta
    static bool deleteKlient(int id);

    // === Pomocnicze metody ===

    // Sprawdzanie czy email już istnieje
    static bool emailExists(const QString& email, int excludeId = -1);

    // Wyszukiwanie klientów po nazwisku
    static QList<Klient> searchKlienciByNazwisko(const QString& nazwisko);

    // Liczba wszystkich klientów
    static int getKlienciCount();

private:
    DatabaseManager() = default;
    static QSqlDatabase db;

    // Pomocnicza metoda do konwersji QSqlQuery na strukturę Klient
    static Klient queryToKlient(class QSqlQuery& query);
};

#endif // DATABASEMANAGER_H
