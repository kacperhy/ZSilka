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

struct Rezerwacja {
    int id;
    int idKlienta;
    int idZajec;
    QString dataRezerwacji; // format YYYY-MM-DD HH:MM:SS
    QString status;         // "aktywna", "anulowana", itp.

    // Dodatkowe informacje (z joinów)
    QString imieKlienta;
    QString nazwiskoKlienta;
    QString nazwaZajec;
    QString trenerZajec;
    QString dataZajec;
    QString czasZajec;
};

class DatabaseManager {
public:
    // === Podstawowe metody połączenia ===
    static bool connect(const QString& path);
    static void disconnect();
    static QSqlDatabase& instance();

    // === CRUD dla KLIENTÓW ===
    static bool addKlient(const QString& imie,
                          const QString& nazwisko,
                          const QString& email = QString(),
                          const QString& telefon = QString(),
                          const QString& dataUrodzenia = QString(),
                          const QString& uwagi = QString());
    static QList<Klient> getAllKlienci();
    static Klient getKlientById(int id);
    static bool updateKlient(int id,
                             const QString& imie,
                             const QString& nazwisko,
                             const QString& email = QString(),
                             const QString& telefon = QString(),
                             const QString& dataUrodzenia = QString(),
                             const QString& uwagi = QString());
    static bool deleteKlient(int id);
    static bool emailExists(const QString& email, int excludeId = -1);
    static QList<Klient> searchKlienciByNazwisko(const QString& nazwisko);
    static int getKlienciCount();

    // === CRUD dla ZAJĘĆ ===
    static bool addZajecia(const QString& nazwa,
                           const QString& trener = QString(),
                           int maksUczestnikow = 20,
                           const QString& data = QString(),
                           const QString& czas = QString(),
                           int czasTrwania = 60,
                           const QString& opis = QString());
    static QList<Zajecia> getAllZajecia();
    static Zajecia getZajeciaById(int id);
    static bool updateZajecia(int id,
                              const QString& nazwa,
                              const QString& trener = QString(),
                              int maksUczestnikow = 20,
                              const QString& data = QString(),
                              const QString& czas = QString(),
                              int czasTrwania = 60,
                              const QString& opis = QString());
    static bool deleteZajecia(int id);
    static QList<Zajecia> searchZajeciaByNazwa(const QString& nazwa);
    static QList<Zajecia> searchZajeciaByTrener(const QString& trener);
    static QList<Zajecia> getZajeciaByData(const QString& data);
    static int getZajeciaCount();
    static bool zajeciaExist(const QString& nazwa, const QString& data, const QString& czas, int excludeId = -1);

    // === CRUD dla REZERWACJI ===

    // Dodawanie nowej rezerwacji
    static bool addRezerwacja(int idKlienta, int idZajec, const QString& status = "aktywna");

    // Pobieranie wszystkich rezerwacji (z informacjami o klientach i zajęciach)
    static QList<Rezerwacja> getAllRezerwacje();

    // Pobieranie rezerwacji po ID
    static Rezerwacja getRezerwacjaById(int id);

    // Aktualizacja statusu rezerwacji
    static bool updateRezerwacjaStatus(int id, const QString& status);

    // Usuwanie rezerwacji (anulowanie)
    static bool deleteRezerwacja(int id);

    // === Pomocnicze metody dla rezerwacji ===

    // Sprawdzanie czy klient ma już rezerwację na te zajęcia
    static bool klientMaRezerwacje(int idKlienta, int idZajec);

    // Pobieranie liczby aktywnych rezerwacji na zajęcia
    static int getIloscAktywnychRezerwacji(int idZajec);

    // Sprawdzanie czy można dodać rezerwację (czy nie przekroczono limitu)
    static bool moznaZarezerwowac(int idZajec);

    // Pobieranie rezerwacji konkretnego klienta
    static QList<Rezerwacja> getRezerwacjeKlienta(int idKlienta);

    // Pobieranie rezerwacji dla konkretnych zajęć
    static QList<Rezerwacja> getRezerwacjeZajec(int idZajec);

    // Pobieranie zajęć dostępnych do rezerwacji (z wolnymi miejscami)
    static QList<Zajecia> getZajeciaDostepneDoRezerwacji();

    // Pobieranie liczby wszystkich rezerwacji
    static int getRezerwacjeCount();

    // === Metody raportowe ===

    // Najpopularniejsze zajęcia (według liczby rezerwacji)
    static QList<QPair<QString, int>> getNajpopularniejszeZajecia(int limit = 10);

    // Najaktywniejszi klienci (według liczby rezerwacji)
    static QList<QPair<QString, int>> getNajaktywniejszychKlientow(int limit = 10);

private:
    DatabaseManager() = default;
    static QSqlDatabase db;

    // Pomocnicze metody do konwersji QSqlQuery na struktury
    static Klient queryToKlient(class QSqlQuery& query);
    static Zajecia queryToZajecia(class QSqlQuery& query);
    static Rezerwacja queryToRezerwacja(class QSqlQuery& query);
};

#endif // DATABASEMANAGER_H
