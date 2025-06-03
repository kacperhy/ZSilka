#include <QApplication>
#include "mainwindow.h"
#include "DatabaseManager.h"
#include <QDebug>
#include <QDir>

// Funkcja do dodawania przykładowych klientów
void dodajPrzykladowychKlientow() {
    qDebug() << "=== Dodawanie przykładowych klientów ===";

    // Sprawdź czy już mamy jakichś klientów
    if (DatabaseManager::getKlienciCount() > 0) {
        qDebug() << "Baza już zawiera klientów, pomijam dodawanie";
        return;
    }

    // Dodaj przykładowych klientów
    DatabaseManager::addKlient("Jan", "Kowalski", "jan.kowalski@email.com", "123456789", "1990-05-15", "Nowy członek");
    DatabaseManager::addKlient("Anna", "Nowak", "anna.nowak@email.com", "987654321", "1985-03-22", "");
    DatabaseManager::addKlient("Piotr", "Wiśniewski", "piotr.w@email.com", "555123456", "1992-11-08", "Regularny klient");
    DatabaseManager::addKlient("Maria", "Kowalczyk", "", "111222333", "1988-07-12", "Bez emaila");
    DatabaseManager::addKlient("Tomasz", "Zieliński", "tomasz.z@email.com", "", "1995-01-30", "Bez telefonu");

    qDebug() << "Dodano" << DatabaseManager::getKlienciCount() << "klientów do bazy";
}

// Funkcja do dodawania przykładowych zajęć
void dodajPrzykladoweZajecia() {
    qDebug() << "=== Dodawanie przykładowych zajęć ===";

    // Sprawdź czy już mamy jakieś zajęcia
    if (DatabaseManager::getZajeciaCount() > 0) {
        qDebug() << "Baza już zawiera zajęcia, pomijam dodawanie";
        return;
    }

    // Dodaj przykładowe zajęcia
    DatabaseManager::addZajecia("Aerobik", "Anna Nowakiewicz", 15, "2025-06-04", "09:00", 60, "Zajęcia cardio dla początkujących");
    DatabaseManager::addZajecia("CrossFit", "Marcin Silny", 12, "2025-06-04", "10:30", 90, "Intensywny trening funkcjonalny");
    DatabaseManager::addZajecia("Yoga", "Zen Master", 20, "2025-06-04", "18:00", 75, "Relaksacyjne zajęcia jogi");
    DatabaseManager::addZajecia("Pilates", "Anna Nowakiewicz", 10, "2025-06-05", "08:00", 60, "Wzmacnianie mięśni głębokich");
    DatabaseManager::addZajecia("Spinning", "Jakub Rowerzysta", 16, "2025-06-05", "19:00", 45, "Zajęcia na rowerach stacjonarnych");
    DatabaseManager::addZajecia("Zumba", "Maria Taniec", 25, "2025-06-06", "17:30", 60, "Taneczne cardio");
    DatabaseManager::addZajecia("TRX", "Marcin Silny", 8, "2025-06-06", "20:00", 50, "Trening z użyciem pasów TRX");

    qDebug() << "Dodano" << DatabaseManager::getZajeciaCount() << "zajęć do bazy";
}

// Funkcja do testowania funkcjonalności CRUD dla zajęć
void testujFunkcjonalnoscZajec() {
    qDebug() << "\n=== Test funkcjonalności CRUD dla zajęć ===";

    // Test pobierania wszystkich zajęć
    QList<Zajecia> zajecia = DatabaseManager::getAllZajecia();
    qDebug() << "Liczba zajęć w bazie:" << zajecia.size();

    // Wyświetl pierwszych kilka zajęć
    for (int i = 0; i < qMin(3, zajecia.size()); i++) {
        const Zajecia& z = zajecia[i];
        qDebug() << QString("Zajęcia %1: %2 (%3) - %4 %5, limit: %6, czas: %7 min")
                        .arg(z.id)
                        .arg(z.nazwa)
                        .arg(z.trener.isEmpty() ? "brak trenera" : z.trener)
                        .arg(z.data.isEmpty() ? "brak daty" : z.data)
                        .arg(z.czas.isEmpty() ? "brak czasu" : z.czas)
                        .arg(z.maksUczestnikow)
                        .arg(z.czasTrwania);
    }

    // Test wyszukiwania po nazwie
    QList<Zajecia> aerobik = DatabaseManager::searchZajeciaByNazwa("Aerobik");
    qDebug() << "Zajęcia zawierające 'Aerobik':" << aerobik.size();

    // Test wyszukiwania po trenerze
    QList<Zajecia> annaZajecia = DatabaseManager::searchZajeciaByTrener("Anna");
    qDebug() << "Zajęcia prowadzone przez 'Anna':" << annaZajecia.size();

    // Test pobierania zajęć z konkretnego dnia
    QList<Zajecia> zajeciaDzisiaj = DatabaseManager::getZajeciaByData("2025-06-04");
    qDebug() << "Zajęcia z dnia 2025-06-04:" << zajeciaDzisiaj.size();
}

// Funkcja do testowania funkcjonalności CRUD dla klientów
void testujFunkcjonalnoscKlientow() {
    qDebug() << "\n=== Test funkcjonalności CRUD dla klientów ===";

    // Test pobierania wszystkich klientów
    QList<Klient> klienci = DatabaseManager::getAllKlienci();
    qDebug() << "Liczba klientów w bazie:" << klienci.size();

    // Wyświetl pierwszych kilku klientów
    for (int i = 0; i < qMin(3, klienci.size()); i++) {
        const Klient& k = klienci[i];
        qDebug() << QString("Klient %1: %2 %3, email: %4, telefon: %5")
                        .arg(k.id)
                        .arg(k.imie)
                        .arg(k.nazwisko)
                        .arg(k.email.isEmpty() ? "brak" : k.email)
                        .arg(k.telefon.isEmpty() ? "brak" : k.telefon);
    }

    // Test wyszukiwania
    QList<Klient> kowalscyKlienci = DatabaseManager::searchKlienciByNazwisko("Kowal");
    qDebug() << "Klienci z nazwiskiem zawierającym 'Kowal':" << kowalscyKlienci.size();
}

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    // 1) Debug - sprawdź ścieżki
    QString appDir = QApplication::applicationDirPath();
    QString dbPath = appDir + "/gym.db";

    qDebug() << "=== INFORMACJE O ŚCIEŻKACH ===";
    qDebug() << "Katalog aplikacji:" << appDir;
    qDebug() << "Ścieżka do bazy:" << dbPath;

    // 2) Połącz z bazą danych
    if (!DatabaseManager::connect(dbPath)) {
        qCritical() << "Nie udało się połączyć z bazą danych!";
        return -1;
    }

    // 3) Utwórz tabele
    MainWindow w;
    w.createTablesIfNotExist();

    // 4) Dodaj przykładowe dane
    dodajPrzykladowychKlientow();
    dodajPrzykladoweZajecia();

    // 5) Przetestuj funkcjonalność
    testujFunkcjonalnoscKlientow();
    testujFunkcjonalnoscZajec();

    // 6) Pokaż okno
    w.show();

    int ret = a.exec();
    DatabaseManager::disconnect();
    return ret;
}
