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

struct Zajecia {
    int id;
    QString nazwa;
    QString trener;
    int maksUczestnikow;
    QString data;           // format YYYY-MM-DD
    QString czas;           // format HH:MM
    int czasTrwania;        // w minutach
    QString opis;
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

    // Sprawdzanie czy email już istnieje
    static bool emailExists(const QString& email, int excludeId = -1);

    // Wyszukiwanie klientów po nazwisku
    static QList<Klient> searchKlienciByNazwisko(const QString& nazwisko);

    // Liczba wszystkich klientów
    static int getKlienciCount();

    // === CRUD dla ZAJĘĆ ===

    // Dodawanie nowych zajęć
    static bool addZajecia(const QString& nazwa,
                           const QString& trener = QString(),
                           int maksUczestnikow = 20,
                           const QString& data = QString(),
                           const QString& czas = QString(),
                           int czasTrwania = 60,
                           const QString& opis = QString());

    // Pobieranie wszystkich zajęć
    static QList<Zajecia> getAllZajecia();

    // Pobieranie zajęć po ID
    static Zajecia getZajeciaById(int id);

    // Aktualizacja danych zajęć
    static bool updateZajecia(int id,
                              const QString& nazwa,
                              const QString& trener = QString(),
                              int maksUczestnikow = 20,
                              const QString& data = QString(),
                              const QString& czas = QString(),
                              int czasTrwania = 60,
                              const QString& opis = QString());

    // Usuwanie zajęć
    static bool deleteZajecia(int id);

    // === Pomocnicze metody dla zajęć ===

    // Wyszukiwanie zajęć po nazwie
    static QList<Zajecia> searchZajeciaByNazwa(const QString& nazwa);

    // Wyszukiwanie zajęć po trenerze
    static QList<Zajecia> searchZajeciaByTrener(const QString& trener);

    // Pobieranie zajęć w konkretnym dniu
    static QList<Zajecia> getZajeciaByData(const QString& data);

    // Liczba wszystkich zajęć
    static int getZajeciaCount();

    // Sprawdzanie czy nazwa zajęć już istnieje (dla tego samego dnia i czasu)
    static bool zajeciaExist(const QString& nazwa, const QString& data, const QString& czas, int excludeId = -1);

private:
    DatabaseManager() = default;
    static QSqlDatabase db;

    // Pomocnicze metody do konwersji QSqlQuery na struktury
    static Klient queryToKlient(class QSqlQuery& query);
    static Zajecia queryToZajecia(class QSqlQuery& query);
};

#endif // DATABASEMANAGER_H
