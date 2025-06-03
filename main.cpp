#include <QApplication>
#include "mainwindow.h"
#include "DatabaseManager.h"
#include <QDebug>
#include <QDir>

// Funkcja do dodawania przykładowych danych testowych
void dodajPrzykladoweDane() {
    qDebug() << "=== Dodawanie przykładowych danych ===";

    // Sprawdź czy już mamy jakichś klientów
    if (DatabaseManager::getKlienciCount() > 0) {
        qDebug() << "Baza już zawiera klientów, pomijam dodawanie przykładowych danych";
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

// Funkcja do testowania funkcjonalności CRUD
void testujFunkcjonalnosc() {
    qDebug() << "\n=== Test funkcjonalności CRUD ===";

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

    // Test pobierania konkretnego klienta
    if (!klienci.isEmpty()) {
        Klient pierwszy = DatabaseManager::getKlientById(klienci.first().id);
        qDebug() << "Pobrany klient o ID" << pierwszy.id << ":" << pierwszy.imie << pierwszy.nazwisko;
    }
}

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    // 1) Debug - sprawdź ścieżki
    QString appDir = QApplication::applicationDirPath();
    QString dbPath = appDir + "/gym.db";

    qDebug() << "=== INFORMACJE O ŚCIEŻKACH ===";
    qDebug() << "Katalog aplikacji:" << appDir;
    qDebug() << "Ścieżka do bazy:" << dbPath;
    qDebug() << "Katalog roboczy:" << QDir::currentPath();

    // Sprawdź czy plik bazy istnieje przed połączeniem
    QFileInfo dbFile(dbPath);
    qDebug() << "Plik bazy istnieje przed połączeniem:" << dbFile.exists();
    if (dbFile.exists()) {
        qDebug() << "Rozmiar pliku bazy:" << dbFile.size() << "bajtów";
        qDebug() << "Ostatnia modyfikacja:" << dbFile.lastModified().toString();
    }

    // 2) Połącz z bazą danych
    if (!DatabaseManager::connect(dbPath)) {
        qCritical() << "Nie udało się połączyć z bazą danych!";
        return -1;
    }

    // 3) Utwórz tabele
    MainWindow w;
    w.createTablesIfNotExist();

    // 4) Sprawdź ponownie czy plik istnieje po utworzeniu tabel
    qDebug() << "\n=== PO UTWORZENIU TABEL ===";
    dbFile.refresh();
    qDebug() << "Plik bazy istnieje po utworzeniu tabel:" << dbFile.exists();
    if (dbFile.exists()) {
        qDebug() << "Rozmiar pliku bazy:" << dbFile.size() << "bajtów";
    }

    // 5) Dodaj przykładowe dane (tylko przy pierwszym uruchomieniu)
    dodajPrzykladoweDane();

    // 6) Sprawdź rozmiar pliku po dodaniu danych
    qDebug() << "\n=== PO DODANIU DANYCH ===";
    dbFile.refresh();
    if (dbFile.exists()) {
        qDebug() << "Rozmiar pliku bazy po dodaniu danych:" << dbFile.size() << "bajtów";
    }

    // 7) Przetestuj funkcjonalność
    testujFunkcjonalnosc();

    // 8) Pokaż okno
    w.show();

    int ret = a.exec();

    // 9) Końcowy debug
    qDebug() << "\n=== PRZED ZAMKNIĘCIEM ===";
    dbFile.refresh();
    if (dbFile.exists()) {
        qDebug() << "Końcowy rozmiar pliku bazy:" << dbFile.size() << "bajtów";
    }

    DatabaseManager::disconnect();
    return ret;
}
