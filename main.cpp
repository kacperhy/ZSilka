#include <QApplication>
#include "mainwindow.h"
#include "DatabaseManager.h"
#include <QDebug>
#include <QDir>

// Funkcja do dodawania przykładowych klientów
void dodajPrzykladowychKlientow() {
    qDebug() << "=== Dodawanie przykładowych klientów ===";

    if (DatabaseManager::getKlienciCount() > 0) {
        qDebug() << "Baza już zawiera klientów, pomijam dodawanie";
        return;
    }

    DatabaseManager::addKlient("Jan", "Kowalski", "jan.kowalski@email.com", "123456789", "1990-05-15", "Nowy członek");
    DatabaseManager::addKlient("Anna", "Nowak", "anna.nowak@email.com", "987654321", "1985-03-22", "");
    DatabaseManager::addKlient("Piotr", "Wiśniewski", "piotr.w@email.com", "555123456", "1992-11-08", "Regularny klient");
    DatabaseManager::addKlient("Maria", "Kowalczyk", "", "111222333", "1988-07-12", "Bez emaila");
    DatabaseManager::addKlient("Tomasz", "Zieliński", "tomasz.z@email.com", "", "1995-01-30", "Bez telefonu");
    DatabaseManager::addKlient("Katarzyna", "Lewandowska", "kasia.l@email.com", "777888999", "1993-09-12", "Miłośniczka jogi");

    qDebug() << "Dodano" << DatabaseManager::getKlienciCount() << "klientów do bazy";
}

// Funkcja do dodawania przykładowych zajęć
void dodajPrzykladoweZajecia() {
    qDebug() << "=== Dodawanie przykładowych zajęć ===";

    if (DatabaseManager::getZajeciaCount() > 0) {
        qDebug() << "Baza już zawiera zajęcia, pomijam dodawanie";
        return;
    }

    DatabaseManager::addZajecia("Aerobik", "Anna Nowakiewicz", 15, "2025-06-04", "09:00", 60, "Zajęcia cardio dla początkujących");
    DatabaseManager::addZajecia("CrossFit", "Marcin Silny", 12, "2025-06-04", "10:30", 90, "Intensywny trening funkcjonalny");
    DatabaseManager::addZajecia("Yoga", "Zen Master", 20, "2025-06-04", "18:00", 75, "Relaksacyjne zajęcia jogi");
    DatabaseManager::addZajecia("Pilates", "Anna Nowakiewicz", 10, "2025-06-05", "08:00", 60, "Wzmacnianie mięśni głębokich");
    DatabaseManager::addZajecia("Spinning", "Jakub Rowerzysta", 16, "2025-06-05", "19:00", 45, "Zajęcia na rowerach stacjonarnych");
    DatabaseManager::addZajecia("Zumba", "Maria Taniec", 25, "2025-06-06", "17:30", 60, "Taneczne cardio");
    DatabaseManager::addZajecia("TRX", "Marcin Silny", 8, "2025-06-06", "20:00", 50, "Trening z użyciem pasów TRX");
    DatabaseManager::addZajecia("Aqua Aerobik", "Monika Wodna", 12, "2025-06-07", "11:00", 45, "Zajęcia w wodzie");

    qDebug() << "Dodano" << DatabaseManager::getZajeciaCount() << "zajęć do bazy";
}

// Funkcja do dodawania przykładowych rezerwacji
void dodajPrzykladoweRezerwacje() {
    qDebug() << "=== Dodawanie przykładowych rezerwacji ===";

    if (DatabaseManager::getRezerwacjeCount() > 0) {
        qDebug() << "Baza już zawiera rezerwacje, pomijam dodawanie";
        return;
    }

    // Pobierz wszystkich klientów i zajęcia
    QList<Klient> klienci = DatabaseManager::getAllKlienci();
    QList<Zajecia> zajecia = DatabaseManager::getAllZajecia();

    if (klienci.isEmpty() || zajecia.isEmpty()) {
        qWarning() << "Brak klientów lub zajęć do dodania rezerwacji";
        return;
    }

    // Dodaj przykładowe rezerwacje
    // Jan Kowalski zapisuje się na Aerobik i CrossFit
    if (klienci.size() > 0 && zajecia.size() > 0) {
        DatabaseManager::addRezerwacja(klienci[0].id, zajecia[0].id, "aktywna"); // Jan -> Aerobik
        if (zajecia.size() > 1) {
            DatabaseManager::addRezerwacja(klienci[0].id, zajecia[1].id, "aktywna"); // Jan -> CrossFit
        }
    }

    // Anna Nowak zapisuje się na Yoga i Pilates
    if (klienci.size() > 1 && zajecia.size() > 2) {
        DatabaseManager::addRezerwacja(klienci[1].id, zajecia[2].id, "aktywna"); // Anna -> Yoga
        if (zajecia.size() > 3) {
            DatabaseManager::addRezerwacja(klienci[1].id, zajecia[3].id, "aktywna"); // Anna -> Pilates
        }
    }

    // Piotr Wiśniewski zapisuje się na Spinning
    if (klienci.size() > 2 && zajecia.size() > 4) {
        DatabaseManager::addRezerwacja(klienci[2].id, zajecia[4].id, "aktywna"); // Piotr -> Spinning
    }

    // Maria Kowalczyk zapisuje się na Zumba
    if (klienci.size() > 3 && zajecia.size() > 5) {
        DatabaseManager::addRezerwacja(klienci[3].id, zajecia[5].id, "aktywna"); // Maria -> Zumba
    }

    // Tomasz Zieliński zapisuje się na TRX
    if (klienci.size() > 4 && zajecia.size() > 6) {
        DatabaseManager::addRezerwacja(klienci[4].id, zajecia[6].id, "aktywna"); // Tomasz -> TRX
    }

    // Katarzyna Lewandowska zapisuje się na Yoga (miłośniczka jogi)
    if (klienci.size() > 5 && zajecia.size() > 2) {
        DatabaseManager::addRezerwacja(klienci[5].id, zajecia[2].id, "aktywna"); // Katarzyna -> Yoga
    }

    // Dodaj jeszcze kilka rezerwacji dla różnorodności
    if (klienci.size() > 0 && zajecia.size() > 5) {
        DatabaseManager::addRezerwacja(klienci[0].id, zajecia[5].id, "aktywna"); // Jan -> Zumba
    }
    if (klienci.size() > 1 && zajecia.size() > 4) {
        DatabaseManager::addRezerwacja(klienci[1].id, zajecia[4].id, "aktywna"); // Anna -> Spinning
    }

    // Dodaj jedną anulowaną rezerwację dla demonstracji
    if (klienci.size() > 2 && zajecia.size() > 0) {
        DatabaseManager::addRezerwacja(klienci[2].id, zajecia[0].id, "anulowana"); // Piotr -> Aerobik (anulowana)
    }

    qDebug() << "Dodano" << DatabaseManager::getRezerwacjeCount() << "rezerwacji do bazy";
}

// Funkcja do testowania funkcjonalności rezerwacji
void testujFunkcjonalnoscRezerwacji() {
    qDebug() << "\n=== Test funkcjonalności rezerwacji ===";

    // Test pobierania wszystkich rezerwacji
    QList<Rezerwacja> rezerwacje = DatabaseManager::getAllRezerwacje();
    qDebug() << "Liczba rezerwacji w bazie:" << rezerwacje.size();

    // Wyświetl pierwsze kilka rezerwacji
    for (int i = 0; i < qMin(3, rezerwacje.size()); i++) {
        const Rezerwacja& r = rezerwacje[i];
        qDebug() << QString("Rezerwacja %1: %2 %3 -> %4 (%5 %6), status: %7")
                        .arg(r.id)
                        .arg(r.imieKlienta)
                        .arg(r.nazwiskoKlienta)
                        .arg(r.nazwaZajec)
                        .arg(r.dataZajec)
                        .arg(r.czasZajec)
                        .arg(r.status);
    }

    // Test sprawdzania dostępności zajęć
    QList<Zajecia> dostepne = DatabaseManager::getZajeciaDostepneDoRezerwacji();
    qDebug() << "Zajęcia dostępne do rezerwacji:" << dostepne.size();

    // Test liczenia aktywnych rezerwacji
    if (!dostepne.isEmpty()) {
        int aktualne = DatabaseManager::getIloscAktywnychRezerwacji(dostepne.first().id);
        qDebug() << QString("Zajęcia '%1' mają %2 aktywnych rezerwacji")
                        .arg(dostepne.first().nazwa)
                        .arg(aktualne);
    }

    // Test raportów
    QList<QPair<QString, int>> popularne = DatabaseManager::getNajpopularniejszeZajecia(3);
    qDebug() << "Najpopularniejsze zajęcia:";
    for (const auto& para : popularne) {
        qDebug() << QString("  %1: %2 rezerwacji").arg(para.first).arg(para.second);
    }

    QList<QPair<QString, int>> aktywni = DatabaseManager::getNajaktywniejszychKlientow(3);
    qDebug() << "Najaktywniejszi klienci:";
    for (const auto& para : aktywni) {
        qDebug() << QString("  %1: %2 rezerwacji").arg(para.first).arg(para.second);
    }
}

// Funkcja do testowania funkcjonalności CRUD dla zajęć
void testujFunkcjonalnoscZajec() {
    qDebug() << "\n=== Test funkcjonalności CRUD dla zajęć ===";

    QList<Zajecia> zajecia = DatabaseManager::getAllZajecia();
    qDebug() << "Liczba zajęć w bazie:" << zajecia.size();

    for (int i = 0; i < qMin(3, zajecia.size()); i++) {
        const Zajecia& z = zajecia[i];
        int aktualne = DatabaseManager::getIloscAktywnychRezerwacji(z.id);
        qDebug() << QString("Zajęcia %1: %2 (%3) - %4 %5, zapisy: %6/%7")
                        .arg(z.id)
                        .arg(z.nazwa)
                        .arg(z.trener.isEmpty() ? "brak trenera" : z.trener)
                        .arg(z.data.isEmpty() ? "brak daty" : z.data)
                        .arg(z.czas.isEmpty() ? "brak czasu" : z.czas)
                        .arg(aktualne)
                        .arg(z.maksUczestnikow);
    }
}

// Funkcja do testowania funkcjonalności CRUD dla klientów
void testujFunkcjonalnoscKlientow() {
    qDebug() << "\n=== Test funkcjonalności CRUD dla klientów ===";

    QList<Klient> klienci = DatabaseManager::getAllKlienci();
    qDebug() << "Liczba klientów w bazie:" << klienci.size();

    for (int i = 0; i < qMin(3, klienci.size()); i++) {
        const Klient& k = klienci[i];
        QList<Rezerwacja> rezerwacje = DatabaseManager::getRezerwacjeKlienta(k.id);
        qDebug() << QString("Klient %1: %2 %3, email: %4, rezerwacje: %5")
                        .arg(k.id)
                        .arg(k.imie)
                        .arg(k.nazwisko)
                        .arg(k.email.isEmpty() ? "brak" : k.email)
                        .arg(rezerwacje.size());
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
    dodajPrzykladoweRezerwacje();

    // 5) Przetestuj funkcjonalność
    testujFunkcjonalnoscKlientow();
    testujFunkcjonalnoscZajec();
    testujFunkcjonalnoscRezerwacji();

    // 6) Pokaż okno
    w.show();

    int ret = a.exec();
    DatabaseManager::disconnect();
    return ret;
}
