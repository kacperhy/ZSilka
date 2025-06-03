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

struct Karnet {
    int id;
    int idKlienta;
    QString typ;            // "normalny", "studencki"
    QString dataRozpoczecia; // format YYYY-MM-DD
    QString dataZakonczenia; // format YYYY-MM-DD
    double cena;
    bool czyAktywny;        // true/false

    // Dodatkowe informacje (z joinów)
    QString imieKlienta;
    QString nazwiskoKlienta;
    QString emailKlienta;
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
    static bool addRezerwacja(int idKlienta, int idZajec, const QString& status = "aktywna");
    static QList<Rezerwacja> getAllRezerwacje();
    static Rezerwacja getRezerwacjaById(int id);
    static bool updateRezerwacjaStatus(int id, const QString& status);
    static bool deleteRezerwacja(int id);

    // === Pomocnicze metody dla rezerwacji ===
    static bool klientMaRezerwacje(int idKlienta, int idZajec);
    static int getIloscAktywnychRezerwacji(int idZajec);
    static bool moznaZarezerwowac(int idZajec);
    static QList<Rezerwacja> getRezerwacjeKlienta(int idKlienta);
    static QList<Rezerwacja> getRezerwacjeZajec(int idZajec);
    static QList<Zajecia> getZajeciaDostepneDoRezerwacji();
    static int getRezerwacjeCount();

    // === CRUD dla KARNETÓW ===

    // Dodawanie nowego karnetu
    static bool addKarnet(int idKlienta,
                          const QString& typ,
                          const QString& dataRozpoczecia,
                          const QString& dataZakonczenia,
                          double cena,
                          bool czyAktywny = true);

    // Pobieranie wszystkich karnetów (z informacjami o klientach)
    static QList<Karnet> getAllKarnety();

    // Pobieranie karnetu po ID
    static Karnet getKarnetById(int id);

    // Aktualizacja karnetu
    static bool updateKarnet(int id,
                             int idKlienta,
                             const QString& typ,
                             const QString& dataRozpoczecia,
                             const QString& dataZakonczenia,
                             double cena,
                             bool czyAktywny);

    // Usuwanie karnetu
    static bool deleteKarnet(int id);

    // === Pomocnicze metody dla karnetów ===

    // Pobieranie karnetów konkretnego klienta
    static QList<Karnet> getKarnetyKlienta(int idKlienta);

    // Pobieranie aktywnych karnetów klienta
    static QList<Karnet> getAktywneKarnetyKlienta(int idKlienta);

    // Sprawdzanie czy klient ma aktywny karnet
    static bool klientMaAktywnyKarnet(int idKlienta);

    // Filtrowanie karnetów po typie
    static QList<Karnet> getKarnetyByTyp(const QString& typ);

    // Filtrowanie karnetów po statusie (aktywne/nieaktywne)
    static QList<Karnet> getKarnetyByStatus(bool czyAktywny);

    // Pobieranie karnetów wygasających w określonym terminie
    static QList<Karnet> getKarnetyWygasajace(const QString& dataOd, const QString& dataDo);

    // Pobieranie liczby wszystkich karnetów
    static int getKarnetyCount();

    // Sprawdzanie czy można dodać karnet (czy klient nie ma już aktywnego karnetu tego typu)
    static bool moznaUtworzycKarnet(int idKlienta, const QString& typ);

    // === Metody raportowe ===
    static QList<QPair<QString, int>> getNajpopularniejszeZajecia(int limit = 10);
    static QList<QPair<QString, int>> getNajaktywniejszychKlientow(int limit = 10);

    // Statystyki karnetów
    static QList<QPair<QString, int>> getStatystykiKarnetow(); // [typ, liczba]
    static double getCalkowitePrzychodyZKarnetow();
    static int getLiczbaAktywnychKarnetow();

private:
    DatabaseManager() = default;
    static QSqlDatabase db;

    // Pomocnicze metody do konwersji QSqlQuery na struktury
    static Klient queryToKlient(class QSqlQuery& query);
    static Zajecia queryToZajecia(class QSqlQuery& query);
    static Rezerwacja queryToRezerwacja(class QSqlQuery& query);
    static Karnet queryToKarnet(class QSqlQuery& query);
};

#endif // DATABASEMANAGER_H
