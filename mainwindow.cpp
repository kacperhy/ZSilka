#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "DatabaseManager.h"
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QDebug>
#include <QHeaderView>
#include <QMessageBox>
#include <QDate>
#include <QTime>
#include <QApplication>
#include <QBrush>
#include <QColor>

// ==================== KONSTRUKTOR I DESTRUKTOR ====================

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , aktualnieEdytowanyKlientId(-1)
    , aktualnieEdytowaneZajeciaId(-1)
    , aktualnieWybranaRezerwacjaId(-1)
    , aktualnieEdytowanyKarnetId(-1)  // <- To powinno być w liście inicjalizacyjnej
{
    ui->setupUi(this);
    setupUI();
    setupConnections();
    setupTableKlienci();
    setupTableZajecia();
    setupTableRezerwacje();
    setupTableKarnety();

    // Załaduj dane do wszystkich tabel
    odswiezListeKlientow();
    odswiezListeZajec();
    odswiezListeRezerwacji();
    odswiezListeKarnetow();
}

MainWindow::~MainWindow()
{
    delete ui;
}

// ==================== TWORZENIE TABEL BAZY DANYCH ====================

void MainWindow::createTablesIfNotExist() {
    QSqlQuery query(DatabaseManager::instance());

    // 1) Tabela klientów
    if (!query.exec(R"(
        CREATE TABLE IF NOT EXISTS klient (
            id              INTEGER PRIMARY KEY AUTOINCREMENT,
            imie            TEXT    NOT NULL,
            nazwisko        TEXT    NOT NULL,
            email           TEXT,
            telefon         TEXT,
            dataUrodzenia   TEXT,
            dataRejestracji TEXT    NOT NULL,
            uwagi           TEXT
        )
    )")) {
        qWarning() << "Błąd tworzenia tabeli 'klient':" << query.lastError().text();
    }

    // 2) Tabela zajęć
    if (!query.exec(R"(
        CREATE TABLE IF NOT EXISTS zajecia (
            id                INTEGER PRIMARY KEY AUTOINCREMENT,
            nazwa             TEXT    NOT NULL,
            trener            TEXT,
            maksUczestnikow   INTEGER,
            data              TEXT,   -- w formacie 'YYYY-MM-DD'
            czas              TEXT,   -- np. 'HH:MM'
            czasTrwania       INTEGER, -- w minutach
            opis              TEXT
        )
    )")) {
        qWarning() << "Błąd tworzenia tabeli 'zajecia':" << query.lastError().text();
    }

    // 3) Tabela karnetów (powiązana z klientem przez idKlienta)
    if (!query.exec(R"(
        CREATE TABLE IF NOT EXISTS karnet (
            id               INTEGER PRIMARY KEY AUTOINCREMENT,
            idKlienta        INTEGER    NOT NULL,
            typ              TEXT,
            dataRozpoczecia  TEXT,
            dataZakonczenia  TEXT,
            cena             REAL,
            czyAktywny       INTEGER,   -- 0 lub 1
            FOREIGN KEY(idKlienta) REFERENCES klient(id)
        )
    )")) {
        qWarning() << "Błąd tworzenia tabeli 'karnet':" << query.lastError().text();
    }

    // 4) Tabela rezerwacji (powiązana z klientem i zajęciami)
    if (!query.exec(R"(
        CREATE TABLE IF NOT EXISTS rezerwacja (
            id               INTEGER PRIMARY KEY AUTOINCREMENT,
            idKlienta        INTEGER    NOT NULL,
            idZajec          INTEGER    NOT NULL,
            dataRezerwacji   TEXT,      -- format 'YYYY-MM-DD HH:MM:SS'
            status           TEXT,
            FOREIGN KEY(idKlienta) REFERENCES klient(id),
            FOREIGN KEY(idZajec)   REFERENCES zajecia(id)
        )
    )")) {
        qWarning() << "Błąd tworzenia tabeli 'rezerwacja':" << query.lastError().text();
    }
}

// ==================== SLOTS DLA REZERWACJI ====================

void MainWindow::dodajRezerwacje() {
    if (!walidujFormularzRezerwacji()) {
        return;
    }

    // Pobierz ID klienta i zajęć z ComboBoxów
    int idKlienta = ui->comboBoxKlientRezerwacji->currentData().toInt();
    int idZajec = ui->comboBoxZajeciaRezerwacji->currentData().toInt();
    QString status = ui->comboBoxStatusRezerwacji->currentText();

    if (idKlienta <= 0 || idZajec <= 0) {
        pokazKomunikat("Błąd", "Wybierz prawidłowego klienta i zajęcia.", QMessageBox::Warning);
        return;
    }

    bool sukces = DatabaseManager::addRezerwacja(idKlienta, idZajec, status);

    if (sukces) {
        pokazKomunikat("Sukces", "Rezerwacja została dodana pomyślnie!", QMessageBox::Information);
        wyczyscFormularzRezerwacji();
        odswiezListeRezerwacji();
        // Odśwież też zajęcia, żeby zaktualizować liczby miejsc
        zaladujZajeciaDoComboBox();
        ui->statusbar->showMessage("Dodano nową rezerwację", 3000);
    } else {
        pokazKomunikat("Błąd", "Nie udało się dodać rezerwacji.\nSprawdź czy:\n• Klient nie ma już rezerwacji na te zajęcia\n• Nie przekroczono limitu uczestników", QMessageBox::Warning);
    }
}

void MainWindow::anulujRezerwacje() {
    if (aktualnieWybranaRezerwacjaId <= 0) {
        pokazKomunikat("Błąd", "Nie wybrano rezerwacji do anulowania.", QMessageBox::Warning);
        return;
    }

    QMessageBox::StandardButton odpowiedz = QMessageBox::question(
        this,
        "Potwierdzenie",
        "Czy na pewno chcesz anulować wybraną rezerwację?\n\nTa operacja jest nieodwracalna!",
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
        );

    if (odpowiedz != QMessageBox::Yes) {
        return;
    }

    // Możemy usunąć rezerwację lub zmienić status na "anulowana"
    // Używam zmiany statusu, żeby zachować historię
    bool sukces = DatabaseManager::updateRezerwacjaStatus(aktualnieWybranaRezerwacjaId, "anulowana");

    if (sukces) {
        pokazKomunikat("Sukces", "Rezerwacja została anulowana!", QMessageBox::Information);
        odswiezListeRezerwacji();
        zaladujZajeciaDoComboBox(); // Odśwież dostępne miejsca
        aktualizujPrzyciskAnuluj();
        ui->statusbar->showMessage("Anulowano rezerwację", 3000);
    } else {
        pokazKomunikat("Błąd", "Nie udało się anulować rezerwacji.", QMessageBox::Critical);
    }
}

void MainWindow::wyczyscFormularzRezerwacji() {
    ui->comboBoxKlientRezerwacji->setCurrentIndex(-1);
    ui->comboBoxZajeciaRezerwacji->setCurrentIndex(-1);
    ui->comboBoxStatusRezerwacji->setCurrentIndex(0); // "aktywna"

    wyczyscInfoZajec();
    aktualnieWybranaRezerwacjaId = -1;
    aktualizujPrzyciskAnuluj();
}

void MainWindow::filtrujRezerwacje() {
    QString statusFilter = ui->comboBoxFilterStatusRezerwacje->currentText();

    QList<Rezerwacja> wszystkieRezerwacje = DatabaseManager::getAllRezerwacje();
    QList<Rezerwacja> przefiltrowane;

    for (const Rezerwacja& r : wszystkieRezerwacje) {
        if (statusFilter == "Wszystkie rezerwacje" ||
            (statusFilter == "Tylko aktywne" && r.status == "aktywna") ||
            (statusFilter == "Tylko anulowane" && r.status == "anulowana")) {
            przefiltrowane.append(r);
        }
    }

    zaladujRezerwacjeDoTabeli(przefiltrowane);
    ui->statusbar->showMessage(QString("Przefiltrowano do %1 rezerwacji").arg(przefiltrowane.size()), 3000);
}

void MainWindow::odswiezListeRezerwacji() {
    QList<Rezerwacja> rezerwacje = DatabaseManager::getAllRezerwacje();
    zaladujRezerwacjeDoTabeli(rezerwacje);
    aktualizujLicznikRezerwacji();
    zaladujKlientowDoComboBox();
    zaladujZajeciaDoComboBox();
    ui->statusbar->showMessage("Lista rezerwacji odświeżona", 2000);
}

void MainWindow::rezerwacjaWybrana() {
    int aktualnyWiersz = ui->tableWidgetRezerwacje->currentRow();

    if (aktualnyWiersz < 0) {
        aktualnieWybranaRezerwacjaId = -1;
        aktualizujPrzyciskAnuluj();
        return;
    }

    QTableWidgetItem* idItem = ui->tableWidgetRezerwacje->item(aktualnyWiersz, 0);
    if (!idItem) {
        return;
    }

    aktualnieWybranaRezerwacjaId = idItem->text().toInt();
    aktualizujPrzyciskAnuluj();
}

void MainWindow::zajeciaRezerwacjiWybrane() {
    aktualizujInfoZajec();
}

void MainWindow::pokazStatystyki() {
    QList<QPair<QString, int>> popularne = DatabaseManager::getNajpopularniejszeZajecia(5);

    QString tekst = "🏆 Najpopularniejsze zajęcia:\n\n";
    if (popularne.isEmpty()) {
        tekst += "Brak danych do wyświetlenia.";
    } else {
        for (int i = 0; i < popularne.size(); i++) {
            tekst += QString("%1. %2 - %3 rezerwacji\n")
            .arg(i + 1)
                .arg(popularne[i].first)
                .arg(popularne[i].second);
        }
    }

    pokazKomunikat("Statystyki zajęć", tekst, QMessageBox::Information);
}

void MainWindow::pokazAktywnychKlientow() {
    QList<QPair<QString, int>> aktywni = DatabaseManager::getNajaktywniejszychKlientow(5);

    QString tekst = "🥇 Najaktywniejszi klienci:\n\n";
    if (aktywni.isEmpty()) {
        tekst += "Brak danych do wyświetlenia.";
    } else {
        for (int i = 0; i < aktywni.size(); i++) {
            tekst += QString("%1. %2 - %3 rezerwacji\n")
            .arg(i + 1)
                .arg(aktywni[i].first)
                .arg(aktywni[i].second);
        }
    }

    pokazKomunikat("Najaktywniejszi klienci", tekst, QMessageBox::Information);
}

// ==================== SETUP METODY ====================

void MainWindow::setupUI() {
    // Ustaw domyślne daty
    ui->dateEditUrodzenia->setDate(QDate::currentDate().addYears(-18));
    ui->dateEditZajecia->setDate(QDate::currentDate());
    ui->dateEditFilterZajecia->setDate(QDate::currentDate());

    // Ustaw domyślny czas
    ui->timeEditZajecia->setTime(QTime(9, 0)); // 09:00

    // Ustaw domyślne wartości dla rezerwacji
    ui->comboBoxStatusRezerwacji->setCurrentIndex(0); // "aktywna"
    wyczyscInfoZajec();

    // Ustaw tryby dodawania na starcie
    ustawTrybDodawaniaKlienta();
    ustawTrybDodawaniaZajec();
    aktualnieWybranaRezerwacjaId = -1;

    // Ustaw status bar
    ui->statusbar->showMessage("Gotowy");

    // Ustaw pierwszą zakładkę jako aktywną
    ui->tabWidget->setCurrentIndex(0);

    // Ustaw domyślne wartości dla karnetów
    ui->dateEditRozpocKarnetu->setDate(QDate::currentDate());
    ui->dateEditZakonKarnetu->setDate(QDate::currentDate().addMonths(1));
    ui->doubleSpinBoxCenaKarnetu->setValue(150.0);
    ui->comboBoxStatusKarnetu->setCurrentIndex(0); // Aktywny
    wyczyscInfoKlienta();
    ustawTrybDodawaniaKarnetu();
}

void MainWindow::setupConnections() {
    // === PRZYCISKI KLIENTÓW ===
    connect(ui->pushButtonDodajKlienta, &QPushButton::clicked, this, &MainWindow::dodajKlienta);
    connect(ui->pushButtonEdytujKlienta, &QPushButton::clicked, this, &MainWindow::edytujKlienta);
    connect(ui->pushButtonUsunKlienta, &QPushButton::clicked, this, &MainWindow::usunKlienta);
    connect(ui->pushButtonWyczyscKlienta, &QPushButton::clicked, this, &MainWindow::wyczyscFormularzKlienta);
    connect(ui->pushButtonSearchKlienci, &QPushButton::clicked, this, &MainWindow::wyszukajKlientow);
    connect(ui->pushButtonShowAllKlienci, &QPushButton::clicked, this, &MainWindow::pokazWszystkichKlientow);
    connect(ui->pushButtonOdswiezKlienci, &QPushButton::clicked, this, &MainWindow::odswiezListeKlientow);

    // === WYSZUKIWANIE KLIENTÓW ENTER ===
    connect(ui->lineEditSearchKlienci, &QLineEdit::returnPressed, this, &MainWindow::wyszukajKlientow);

    // === TABELA KLIENTÓW ===
    connect(ui->tableWidgetKlienci, &QTableWidget::itemSelectionChanged, this, &MainWindow::klientWybrany);

    // === PRZYCISKI ZAJĘĆ ===
    connect(ui->pushButtonDodajZajecia, &QPushButton::clicked, this, &MainWindow::dodajZajecia);
    connect(ui->pushButtonEdytujZajecia, &QPushButton::clicked, this, &MainWindow::edytujZajecia);
    connect(ui->pushButtonUsunZajecia, &QPushButton::clicked, this, &MainWindow::usunZajecia);
    connect(ui->pushButtonWyczyscZajecia, &QPushButton::clicked, this, &MainWindow::wyczyscFormularzZajec);
    connect(ui->pushButtonSearchZajecia, &QPushButton::clicked, this, &MainWindow::wyszukajZajecia);
    connect(ui->pushButtonShowAllZajecia, &QPushButton::clicked, this, &MainWindow::pokazWszystkieZajecia);
    connect(ui->pushButtonOdswiezZajecia, &QPushButton::clicked, this, &MainWindow::odswiezListeZajec);
    connect(ui->pushButtonFilterByDate, &QPushButton::clicked, this, &MainWindow::filtrujZajeciaPoData);

    // === WYSZUKIWANIE ZAJĘĆ ENTER ===
    connect(ui->lineEditSearchZajecia, &QLineEdit::returnPressed, this, &MainWindow::wyszukajZajecia);

    // === TABELA ZAJĘĆ ===
    connect(ui->tableWidgetZajecia, &QTableWidget::itemSelectionChanged, this, &MainWindow::zajeciaWybrane);

    // === PRZYCISKI REZERWACJI ===
    connect(ui->pushButtonDodajRezerwacje, &QPushButton::clicked, this, &MainWindow::dodajRezerwacje);
    connect(ui->pushButtonAnulujRezerwacje, &QPushButton::clicked, this, &MainWindow::anulujRezerwacje);
    connect(ui->pushButtonWyczyscRezerwacje, &QPushButton::clicked, this, &MainWindow::wyczyscFormularzRezerwacji);
    connect(ui->pushButtonOdswiezRezerwacje, &QPushButton::clicked, this, &MainWindow::odswiezListeRezerwacji);
    connect(ui->pushButtonFilterRezerwacje, &QPushButton::clicked, this, &MainWindow::filtrujRezerwacje);
    connect(ui->pushButtonPokazStatystyki, &QPushButton::clicked, this, &MainWindow::pokazStatystyki);
    connect(ui->pushButtonPokazAktywnych, &QPushButton::clicked, this, &MainWindow::pokazAktywnychKlientow);

    // === COMBOBOX REZERWACJI ===
    connect(ui->comboBoxZajeciaRezerwacji, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::zajeciaRezerwacjiWybrane);

    // === TABELA REZERWACJI ===
    connect(ui->tableWidgetRezerwacje, &QTableWidget::itemSelectionChanged, this, &MainWindow::rezerwacjaWybrana);

    // === MENU ===
    connect(ui->actionZamknij, &QAction::triggered, this, &MainWindow::zamknijAplikacje);
    connect(ui->actionOProgramie, &QAction::triggered, this, &MainWindow::oProgramie);

    // === PRZYCISKI KARNETÓW ===
    connect(ui->pushButtonDodajKarnet, &QPushButton::clicked, this, &MainWindow::dodajKarnet);
    connect(ui->pushButtonEdytujKarnet, &QPushButton::clicked, this, &MainWindow::edytujKarnet);
    connect(ui->pushButtonUsunKarnet, &QPushButton::clicked, this, &MainWindow::usunKarnet);
    connect(ui->pushButtonWyczyscKarnet, &QPushButton::clicked, this, &MainWindow::wyczyscFormularzKarnetu);
    connect(ui->pushButtonOdswiezKarnety, &QPushButton::clicked, this, &MainWindow::odswiezListeKarnetow);
    connect(ui->pushButtonFilterKarnety, &QPushButton::clicked, this, &MainWindow::filtrujKarnety);
    connect(ui->pushButtonPokazStatystykiKarnety, &QPushButton::clicked, this, &MainWindow::pokazStatystykiKarnetow);
    connect(ui->pushButtonPokazWygasajace, &QPushButton::clicked, this, &MainWindow::pokazWygasajaceKarnety);

    // === COMBOBOX KARNETÓW ===
    connect(ui->comboBoxKlientKarnetu, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::klientKarnetuWybrany);
    connect(ui->comboBoxTypKarnetu, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::obliczCeneKarnetu);

    // === TABELA KARNETÓW ===
    connect(ui->tableWidgetKarnety, &QTableWidget::itemSelectionChanged, this, &MainWindow::karnetWybrany);
}

void MainWindow::setupTableKlienci() {
    // Konfiguracja tabeli klientów
    ui->tableWidgetKlienci->setColumnCount(7);

    QStringList headers = {"ID", "Imię", "Nazwisko", "Email", "Telefon", "Data urodzenia", "Data rejestracji"};
    ui->tableWidgetKlienci->setHorizontalHeaderLabels(headers);

    // Ukryj kolumnę ID
    ui->tableWidgetKlienci->setColumnHidden(0, true);

    // Ustaw tryb zaznaczania całych wierszy
    ui->tableWidgetKlienci->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidgetKlienci->setSelectionMode(QAbstractItemView::SingleSelection);

    // Ustaw szerokości kolumn
    QHeaderView* header = ui->tableWidgetKlienci->horizontalHeader();
    header->setStretchLastSection(true);
    header->resizeSection(1, 120); // Imię
    header->resizeSection(2, 120); // Nazwisko
    header->resizeSection(3, 200); // Email
    header->resizeSection(4, 120); // Telefon
    header->resizeSection(5, 120); // Data urodzenia
}

void MainWindow::setupTableZajecia() {
    // Konfiguracja tabeli zajęć
    ui->tableWidgetZajecia->setColumnCount(8);

    QStringList headers = {"ID", "Nazwa", "Trener", "Data", "Godzina", "Czas trwania", "Limit", "Opis"};
    ui->tableWidgetZajecia->setHorizontalHeaderLabels(headers);

    // Ukryj kolumnę ID
    ui->tableWidgetZajecia->setColumnHidden(0, true);

    // Ustaw tryb zaznaczania całych wierszy
    ui->tableWidgetZajecia->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidgetZajecia->setSelectionMode(QAbstractItemView::SingleSelection);

    // Ustaw szerokości kolumn
    QHeaderView* header = ui->tableWidgetZajecia->horizontalHeader();
    header->setStretchLastSection(true);
    header->resizeSection(1, 120); // Nazwa
    header->resizeSection(2, 140); // Trener
    header->resizeSection(3, 100); // Data
    header->resizeSection(4, 80);  // Godzina
    header->resizeSection(5, 90);  // Czas trwania
    header->resizeSection(6, 60);  // Limit
}

void MainWindow::setupTableRezerwacje() {
    // Konfiguracja tabeli rezerwacji
    ui->tableWidgetRezerwacje->setColumnCount(8);

    QStringList headers = {"ID", "Klient", "Zajęcia", "Trener", "Data zajęć", "Godzina", "Data rezerwacji", "Status"};
    ui->tableWidgetRezerwacje->setHorizontalHeaderLabels(headers);

    // Ukryj kolumnę ID
    ui->tableWidgetRezerwacje->setColumnHidden(0, true);

    // Ustaw tryb zaznaczania całych wierszy
    ui->tableWidgetRezerwacje->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidgetRezerwacje->setSelectionMode(QAbstractItemView::SingleSelection);

    // Ustaw szerokości kolumn
    QHeaderView* header = ui->tableWidgetRezerwacje->horizontalHeader();
    header->setStretchLastSection(true);
    header->resizeSection(1, 150); // Klient
    header->resizeSection(2, 120); // Zajęcia
    header->resizeSection(3, 140); // Trener
    header->resizeSection(4, 100); // Data zajęć
    header->resizeSection(5, 80);  // Godzina
    header->resizeSection(6, 140); // Data rezerwacji
}

// ==================== SLOTS DLA KLIENTÓW ====================

void MainWindow::dodajKlienta() {
    if (!walidujFormularzKlienta()) {
        return;
    }

    Klient klient = pobierzDaneKlientaZFormularza();

    bool sukces = DatabaseManager::addKlient(
        klient.imie,
        klient.nazwisko,
        klient.email,
        klient.telefon,
        klient.dataUrodzenia,
        klient.uwagi
        );

    if (sukces) {
        pokazKomunikat("Sukces", "Klient został dodany pomyślnie!", QMessageBox::Information);
        wyczyscFormularzKlienta();
        odswiezListeKlientow();
        ui->statusbar->showMessage("Dodano nowego klienta", 3000);
    } else {
        pokazKomunikat("Błąd", "Nie udało się dodać klienta.\nSprawdź czy email nie jest już używany.", QMessageBox::Warning);
    }
}

void MainWindow::edytujKlienta() {
    if (aktualnieEdytowanyKlientId <= 0) {
        pokazKomunikat("Błąd", "Nie wybrano klienta do edycji.", QMessageBox::Warning);
        return;
    }

    if (!walidujFormularzKlienta()) {
        return;
    }

    Klient klient = pobierzDaneKlientaZFormularza();

    bool sukces = DatabaseManager::updateKlient(
        aktualnieEdytowanyKlientId,
        klient.imie,
        klient.nazwisko,
        klient.email,
        klient.telefon,
        klient.dataUrodzenia,
        klient.uwagi
        );

    if (sukces) {
        pokazKomunikat("Sukces", "Dane klienta zostały zaktualizowane!", QMessageBox::Information);
        odswiezListeKlientow();
        ustawTrybDodawaniaKlienta();
        ui->statusbar->showMessage("Zaktualizowano dane klienta", 3000);
    } else {
        pokazKomunikat("Błąd", "Nie udało się zaktualizować danych klienta.\nSprawdź czy email nie jest już używany.", QMessageBox::Warning);
    }
}

void MainWindow::usunKlienta() {
    if (aktualnieEdytowanyKlientId <= 0) {
        pokazKomunikat("Błąd", "Nie wybrano klienta do usunięcia.", QMessageBox::Warning);
        return;
    }

    QMessageBox::StandardButton odpowiedz = QMessageBox::question(
        this,
        "Potwierdzenie",
        QString("Czy na pewno chcesz usunąć klienta:\n%1 %2?\n\nTa operacja jest nieodwracalna!")
            .arg(ui->lineEditImie->text())
            .arg(ui->lineEditNazwisko->text()),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
        );

    if (odpowiedz != QMessageBox::Yes) {
        return;
    }

    bool sukces = DatabaseManager::deleteKlient(aktualnieEdytowanyKlientId);

    if (sukces) {
        pokazKomunikat("Sukces", "Klient został usunięty!", QMessageBox::Information);
        wyczyscFormularzKlienta();
        odswiezListeKlientow();
        ustawTrybDodawaniaKlienta();
        ui->statusbar->showMessage("Usunięto klienta", 3000);
    } else {
        pokazKomunikat("Błąd", "Nie udało się usunąć klienta.", QMessageBox::Critical);
    }
}

void MainWindow::wyczyscFormularzKlienta() {
    ui->lineEditImie->clear();
    ui->lineEditNazwisko->clear();
    ui->lineEditEmail->clear();
    ui->lineEditTelefon->clear();
    ui->dateEditUrodzenia->setDate(QDate::currentDate().addYears(-18));
    ui->textEditUwagi->clear();

    ustawTrybDodawaniaKlienta();
}

void MainWindow::wyszukajKlientow() {
    QString nazwisko = ui->lineEditSearchKlienci->text().trimmed();

    if (nazwisko.isEmpty()) {
        pokazWszystkichKlientow();
        return;
    }

    QList<Klient> klienci = DatabaseManager::searchKlienciByNazwisko(nazwisko);
    zaladujKlientowDoTabeli(klienci);

    ui->statusbar->showMessage(QString("Znaleziono %1 klientów").arg(klienci.size()), 3000);
}

void MainWindow::pokazWszystkichKlientow() {
    ui->lineEditSearchKlienci->clear();
    odswiezListeKlientow();
}

void MainWindow::odswiezListeKlientow() {
    QList<Klient> klienci = DatabaseManager::getAllKlienci();
    zaladujKlientowDoTabeli(klienci);
    aktualizujLicznikKlientow();
    ui->statusbar->showMessage("Lista klientów odświeżona", 2000);
}

void MainWindow::klientWybrany() {
    int aktualnyWiersz = ui->tableWidgetKlienci->currentRow();

    if (aktualnyWiersz < 0) {
        ustawTrybDodawaniaKlienta();
        return;
    }

    QTableWidgetItem* idItem = ui->tableWidgetKlienci->item(aktualnyWiersz, 0);
    if (!idItem) {
        return;
    }

    int klientId = idItem->text().toInt();
    Klient klient = DatabaseManager::getKlientById(klientId);

    if (klient.id > 0) {
        zaladujKlientaDoFormularza(klient);
        ustawTrybEdycjiKlienta();
        aktualnieEdytowanyKlientId = klient.id;
    }
}

// ==================== SLOTS DLA ZAJĘĆ ====================

void MainWindow::dodajZajecia() {
    if (!walidujFormularzZajec()) {
        return;
    }

    Zajecia zajecia = pobierzDaneZajecZFormularza();

    bool sukces = DatabaseManager::addZajecia(
        zajecia.nazwa,
        zajecia.trener,
        zajecia.maksUczestnikow,
        zajecia.data,
        zajecia.czas,
        zajecia.czasTrwania,
        zajecia.opis
        );

    if (sukces) {
        pokazKomunikat("Sukces", "Zajęcia zostały dodane pomyślnie!", QMessageBox::Information);
        wyczyscFormularzZajec();
        odswiezListeZajec();
        ui->statusbar->showMessage("Dodano nowe zajęcia", 3000);
    } else {
        pokazKomunikat("Błąd", "Nie udało się dodać zajęć.\nSprawdź czy zajęcia o tej nazwie, dacie i czasie już nie istnieją.", QMessageBox::Warning);
    }
}

void MainWindow::edytujZajecia() {
    if (aktualnieEdytowaneZajeciaId <= 0) {
        pokazKomunikat("Błąd", "Nie wybrano zajęć do edycji.", QMessageBox::Warning);
        return;
    }

    if (!walidujFormularzZajec()) {
        return;
    }

    Zajecia zajecia = pobierzDaneZajecZFormularza();

    bool sukces = DatabaseManager::updateZajecia(
        aktualnieEdytowaneZajeciaId,
        zajecia.nazwa,
        zajecia.trener,
        zajecia.maksUczestnikow,
        zajecia.data,
        zajecia.czas,
        zajecia.czasTrwania,
        zajecia.opis
        );

    if (sukces) {
        pokazKomunikat("Sukces", "Dane zajęć zostały zaktualizowane!", QMessageBox::Information);
        odswiezListeZajec();
        ustawTrybDodawaniaZajec();
        ui->statusbar->showMessage("Zaktualizowano dane zajęć", 3000);
    } else {
        pokazKomunikat("Błąd", "Nie udało się zaktualizować danych zajęć.", QMessageBox::Warning);
    }
}

void MainWindow::usunZajecia() {
    if (aktualnieEdytowaneZajeciaId <= 0) {
        pokazKomunikat("Błąd", "Nie wybrano zajęć do usunięcia.", QMessageBox::Warning);
        return;
    }

    QMessageBox::StandardButton odpowiedz = QMessageBox::question(
        this,
        "Potwierdzenie",
        QString("Czy na pewno chcesz usunąć zajęcia:\n%1?\n\nTa operacja jest nieodwracalna!")
            .arg(ui->lineEditNazwaZajec->text()),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
        );

    if (odpowiedz != QMessageBox::Yes) {
        return;
    }

    bool sukces = DatabaseManager::deleteZajecia(aktualnieEdytowaneZajeciaId);

    if (sukces) {
        pokazKomunikat("Sukces", "Zajęcia zostały usunięte!", QMessageBox::Information);
        wyczyscFormularzZajec();
        odswiezListeZajec();
        ustawTrybDodawaniaZajec();
        ui->statusbar->showMessage("Usunięto zajęcia", 3000);
    } else {
        pokazKomunikat("Błąd", "Nie udało się usunąć zajęć.", QMessageBox::Critical);
    }
}

void MainWindow::wyczyscFormularzZajec() {
    ui->lineEditNazwaZajec->clear();
    ui->lineEditTrener->clear();
    ui->dateEditZajecia->setDate(QDate::currentDate());
    ui->timeEditZajecia->setTime(QTime(9, 0));
    ui->spinBoxCzasTrwania->setValue(60);
    ui->spinBoxMaksUczestnikow->setValue(20);
    ui->textEditOpisZajec->clear();

    ustawTrybDodawaniaZajec();
}

void MainWindow::wyszukajZajecia() {
    QString fraza = ui->lineEditSearchZajecia->text().trimmed();

    if (fraza.isEmpty()) {
        pokazWszystkieZajecia();
        return;
    }

    QList<Zajecia> zajecia;

    // Sprawdź typ wyszukiwania
    int typWyszukiwania = ui->comboBoxSearchTypeZajecia->currentIndex();
    if (typWyszukiwania == 0) {
        // Wyszukiwanie po nazwie
        zajecia = DatabaseManager::searchZajeciaByNazwa(fraza);
    } else {
        // Wyszukiwanie po trenerze
        zajecia = DatabaseManager::searchZajeciaByTrener(fraza);
    }

    zaladujZajeciaDoTabeli(zajecia);

    QString typTekst = (typWyszukiwania == 0) ? "nazwie" : "trenerze";
    ui->statusbar->showMessage(QString("Znaleziono %1 zajęć po %2").arg(zajecia.size()).arg(typTekst), 3000);
}

void MainWindow::pokazWszystkieZajecia() {
    ui->lineEditSearchZajecia->clear();
    odswiezListeZajec();
}

void MainWindow::filtrujZajeciaPoData() {
    QString data = ui->dateEditFilterZajecia->date().toString("yyyy-MM-dd");
    QList<Zajecia> zajecia = DatabaseManager::getZajeciaByData(data);
    zaladujZajeciaDoTabeli(zajecia);

    ui->statusbar->showMessage(QString("Znaleziono %1 zajęć w dniu %2").arg(zajecia.size()).arg(data), 3000);
}

void MainWindow::odswiezListeZajec() {
    QList<Zajecia> zajecia = DatabaseManager::getAllZajecia();
    zaladujZajeciaDoTabeli(zajecia);
    aktualizujLicznikZajec();
    ui->statusbar->showMessage("Lista zajęć odświeżona", 2000);
}

void MainWindow::zajeciaWybrane() {
    int aktualnyWiersz = ui->tableWidgetZajecia->currentRow();

    if (aktualnyWiersz < 0) {
        ustawTrybDodawaniaZajec();
        return;
    }

    QTableWidgetItem* idItem = ui->tableWidgetZajecia->item(aktualnyWiersz, 0);
    if (!idItem) {
        return;
    }

    int zajeciaId = idItem->text().toInt();
    Zajecia zajecia = DatabaseManager::getZajeciaById(zajeciaId);

    if (zajecia.id > 0) {
        zaladujZajeciaDoFormularza(zajecia);
        ustawTrybEdycjiZajec();
        aktualnieEdytowaneZajeciaId = zajecia.id;
    }
}

// ==================== SLOTS MENU ====================

void MainWindow::zamknijAplikacje() {
    QApplication::quit();
}

void MainWindow::oProgramie() {
    QMessageBox::about(this, "O programie",
                       "Zarządzanie Siłownią v3.0\n\n"
                       "Kompletny system do zarządzania siłownią z pełną obsługą rezerwacji.\n\n"
                       "Funkcje:\n"
                       "• Zarządzanie klientami (dodawanie, edycja, usuwanie)\n"
                       "• Zarządzanie zajęciami grupowymi\n"
                       "• System rezerwacji z kontrolą limitów uczestników\n"
                       "• Automatyczne sprawdzanie dostępności miejsc\n"
                       "• Wyszukiwanie i filtrowanie danych\n"
                       "• Statystyki najpopularniejszych zajęć\n"
                       "• Raporty najaktywniejszych klientów\n\n"
                       "Technologia: Qt + SQLite"
                       );
}

// ==================== METODY POMOCNICZE - KLIENCI ====================

void MainWindow::zaladujKlientowDoTabeli(const QList<Klient>& klienci) {
    ui->tableWidgetKlienci->setRowCount(klienci.size());

    for (int i = 0; i < klienci.size(); ++i) {
        const Klient& k = klienci[i];

        ui->tableWidgetKlienci->setItem(i, 0, new QTableWidgetItem(QString::number(k.id)));
        ui->tableWidgetKlienci->setItem(i, 1, new QTableWidgetItem(k.imie));
        ui->tableWidgetKlienci->setItem(i, 2, new QTableWidgetItem(k.nazwisko));
        ui->tableWidgetKlienci->setItem(i, 3, new QTableWidgetItem(k.email));
        ui->tableWidgetKlienci->setItem(i, 4, new QTableWidgetItem(k.telefon));
        ui->tableWidgetKlienci->setItem(i, 5, new QTableWidgetItem(k.dataUrodzenia));
        ui->tableWidgetKlienci->setItem(i, 6, new QTableWidgetItem(k.dataRejestracji));
    }
}

void MainWindow::zaladujKlientaDoFormularza(const Klient& klient) {
    ui->lineEditImie->setText(klient.imie);
    ui->lineEditNazwisko->setText(klient.nazwisko);
    ui->lineEditEmail->setText(klient.email);
    ui->lineEditTelefon->setText(klient.telefon);
    ui->textEditUwagi->setPlainText(klient.uwagi);

    if (!klient.dataUrodzenia.isEmpty()) {
        QDate data = QDate::fromString(klient.dataUrodzenia, "yyyy-MM-dd");
        if (data.isValid()) {
            ui->dateEditUrodzenia->setDate(data);
        }
    }
}

Klient MainWindow::pobierzDaneKlientaZFormularza() {
    Klient klient = {};

    klient.imie = ui->lineEditImie->text().trimmed();
    klient.nazwisko = ui->lineEditNazwisko->text().trimmed();
    klient.email = ui->lineEditEmail->text().trimmed();
    klient.telefon = ui->lineEditTelefon->text().trimmed();
    klient.dataUrodzenia = ui->dateEditUrodzenia->date().toString("yyyy-MM-dd");
    klient.uwagi = ui->textEditUwagi->toPlainText().trimmed();

    return klient;
}

bool MainWindow::walidujFormularzKlienta() {
    QString bledy = "";

    if (ui->lineEditImie->text().trimmed().isEmpty()) {
        bledy += "• Imię jest wymagane\n";
    }

    if (ui->lineEditNazwisko->text().trimmed().isEmpty()) {
        bledy += "• Nazwisko jest wymagane\n";
    }

    QString email = ui->lineEditEmail->text().trimmed();
    if (!email.isEmpty() && !email.contains("@")) {
        bledy += "• Email ma nieprawidłowy format\n";
    }

    if (!bledy.isEmpty()) {
        pokazKomunikat("Błąd walidacji", "Formularz zawiera błędy:\n\n" + bledy, QMessageBox::Warning);
        return false;
    }

    return true;
}

void MainWindow::ustawTrybDodawaniaKlienta() {
    aktualnieEdytowanyKlientId = -1;

    ui->pushButtonDodajKlienta->setEnabled(true);
    ui->pushButtonEdytujKlienta->setEnabled(false);
    ui->pushButtonUsunKlienta->setEnabled(false);

    ui->labelFormularzKlientaTitle->setText("Dodaj nowego klienta");

    ui->tableWidgetKlienci->clearSelection();
}

void MainWindow::ustawTrybEdycjiKlienta() {
    ui->pushButtonDodajKlienta->setEnabled(false);
    ui->pushButtonEdytujKlienta->setEnabled(true);
    ui->pushButtonUsunKlienta->setEnabled(true);

    ui->labelFormularzKlientaTitle->setText("Edytuj klienta");
}

void MainWindow::aktualizujLicznikKlientow() {
    int liczba = DatabaseManager::getKlienciCount();
    ui->labelLiczbaKlientow->setText(QString("Liczba klientów: %1").arg(liczba));
}

// ==================== METODY POMOCNICZE - ZAJĘCIA ====================

void MainWindow::zaladujZajeciaDoTabeli(const QList<Zajecia>& zajecia) {
    ui->tableWidgetZajecia->setRowCount(zajecia.size());

    for (int i = 0; i < zajecia.size(); ++i) {
        const Zajecia& z = zajecia[i];

        ui->tableWidgetZajecia->setItem(i, 0, new QTableWidgetItem(QString::number(z.id)));
        ui->tableWidgetZajecia->setItem(i, 1, new QTableWidgetItem(z.nazwa));
        ui->tableWidgetZajecia->setItem(i, 2, new QTableWidgetItem(z.trener));
        ui->tableWidgetZajecia->setItem(i, 3, new QTableWidgetItem(z.data));
        ui->tableWidgetZajecia->setItem(i, 4, new QTableWidgetItem(z.czas));
        ui->tableWidgetZajecia->setItem(i, 5, new QTableWidgetItem(QString("%1 min").arg(z.czasTrwania)));
        ui->tableWidgetZajecia->setItem(i, 6, new QTableWidgetItem(QString::number(z.maksUczestnikow)));
        ui->tableWidgetZajecia->setItem(i, 7, new QTableWidgetItem(z.opis));
    }
}

void MainWindow::zaladujZajeciaDoFormularza(const Zajecia& zajecia) {
    ui->lineEditNazwaZajec->setText(zajecia.nazwa);
    ui->lineEditTrener->setText(zajecia.trener);
    ui->spinBoxMaksUczestnikow->setValue(zajecia.maksUczestnikow);
    ui->spinBoxCzasTrwania->setValue(zajecia.czasTrwania);
    ui->textEditOpisZajec->setPlainText(zajecia.opis);

    if (!zajecia.data.isEmpty()) {
        QDate data = QDate::fromString(zajecia.data, "yyyy-MM-dd");
        if (data.isValid()) {
            ui->dateEditZajecia->setDate(data);
        }
    }

    if (!zajecia.czas.isEmpty()) {
        QTime czas = QTime::fromString(zajecia.czas, "HH:mm");
        if (czas.isValid()) {
            ui->timeEditZajecia->setTime(czas);
        }
    }
}

Zajecia MainWindow::pobierzDaneZajecZFormularza() {
    Zajecia zajecia = {};

    zajecia.nazwa = ui->lineEditNazwaZajec->text().trimmed();
    zajecia.trener = ui->lineEditTrener->text().trimmed();
    zajecia.maksUczestnikow = ui->spinBoxMaksUczestnikow->value();
    zajecia.data = ui->dateEditZajecia->date().toString("yyyy-MM-dd");
    zajecia.czas = ui->timeEditZajecia->time().toString("HH:mm");
    zajecia.czasTrwania = ui->spinBoxCzasTrwania->value();
    zajecia.opis = ui->textEditOpisZajec->toPlainText().trimmed();

    return zajecia;
}

bool MainWindow::walidujFormularzZajec() {
    QString bledy = "";

    if (ui->lineEditNazwaZajec->text().trimmed().isEmpty()) {
        bledy += "• Nazwa zajęć jest wymagana\n";
    }

    if (!bledy.isEmpty()) {
        pokazKomunikat("Błąd walidacji", "Formularz zawiera błędy:\n\n" + bledy, QMessageBox::Warning);
        return false;
    }

    return true;
}

void MainWindow::ustawTrybDodawaniaZajec() {
    aktualnieEdytowaneZajeciaId = -1;

    ui->pushButtonDodajZajecia->setEnabled(true);
    ui->pushButtonEdytujZajecia->setEnabled(false);
    ui->pushButtonUsunZajecia->setEnabled(false);

    ui->labelFormularzZajeciaTitle->setText("Dodaj nowe zajęcia");

    ui->tableWidgetZajecia->clearSelection();
}

void MainWindow::ustawTrybEdycjiZajec() {
    ui->pushButtonDodajZajecia->setEnabled(false);
    ui->pushButtonEdytujZajecia->setEnabled(true);
    ui->pushButtonUsunZajecia->setEnabled(true);

    ui->labelFormularzZajeciaTitle->setText("Edytuj zajęcia");
}

void MainWindow::aktualizujLicznikZajec() {
    int liczba = DatabaseManager::getZajeciaCount();
    ui->labelLiczbaZajec->setText(QString("Liczba zajęć: %1").arg(liczba));
}

// ==================== METODY POMOCNICZE - REZERWACJE ====================

void MainWindow::zaladujRezerwacjeDoTabeli(const QList<Rezerwacja>& rezerwacje) {
    ui->tableWidgetRezerwacje->setRowCount(rezerwacje.size());

    for (int i = 0; i < rezerwacje.size(); ++i) {
        const Rezerwacja& r = rezerwacje[i];

        ui->tableWidgetRezerwacje->setItem(i, 0, new QTableWidgetItem(QString::number(r.id)));
        ui->tableWidgetRezerwacje->setItem(i, 1, new QTableWidgetItem(QString("%1 %2").arg(r.imieKlienta).arg(r.nazwiskoKlienta)));
        ui->tableWidgetRezerwacje->setItem(i, 2, new QTableWidgetItem(r.nazwaZajec));
        ui->tableWidgetRezerwacje->setItem(i, 3, new QTableWidgetItem(r.trenerZajec));
        ui->tableWidgetRezerwacje->setItem(i, 4, new QTableWidgetItem(r.dataZajec));
        ui->tableWidgetRezerwacje->setItem(i, 5, new QTableWidgetItem(r.czasZajec));

        // Formatuj datę rezerwacji (pokaż tylko datę, bez godziny)
        QString dataRezerwacji = r.dataRezerwacji;
        if (dataRezerwacji.length() > 10) {
            dataRezerwacji = dataRezerwacji.left(10); // Tylko YYYY-MM-DD
        }
        ui->tableWidgetRezerwacje->setItem(i, 6, new QTableWidgetItem(dataRezerwacji));

        // Status z kolorowym tłem
        QTableWidgetItem* statusItem = new QTableWidgetItem(r.status);
        if (r.status == "aktywna") {
            statusItem->setBackground(QBrush(QColor(144, 238, 144))); // Jasny zielony
        } else if (r.status == "anulowana") {
            statusItem->setBackground(QBrush(QColor(255, 182, 193))); // Jasny czerwony
        }
        ui->tableWidgetRezerwacje->setItem(i, 7, statusItem);
    }
}

void MainWindow::zaladujKlientowDoComboBox() {
    ui->comboBoxKlientRezerwacji->clear();

    QList<Klient> klienci = DatabaseManager::getAllKlienci();
    for (const Klient& k : klienci) {
        QString tekst = QString("%1 %2").arg(k.imie).arg(k.nazwisko);
        if (!k.email.isEmpty()) {
            tekst += QString(" (%1)").arg(k.email);
        }
        ui->comboBoxKlientRezerwacji->addItem(tekst, k.id);
    }

    ui->comboBoxKlientRezerwacji->setCurrentIndex(-1); // Nic nie wybrane
}

void MainWindow::zaladujZajeciaDoComboBox() {
    ui->comboBoxZajeciaRezerwacji->clear();

    QList<Zajecia> dostepneZajecia = DatabaseManager::getZajeciaDostepneDoRezerwacji();
    for (const Zajecia& z : dostepneZajecia) {
        int aktualne = DatabaseManager::getIloscAktywnychRezerwacji(z.id);

        QString tekst = QString("%1 - %2 %3 (%4/%5 miejsc)")
                            .arg(z.nazwa)
                            .arg(z.data)
                            .arg(z.czas)
                            .arg(aktualne)
                            .arg(z.maksUczestnikow);

        if (!z.trener.isEmpty()) {
            tekst += QString(" [%1]").arg(z.trener);
        }

        ui->comboBoxZajeciaRezerwacji->addItem(tekst, z.id);
    }

    ui->comboBoxZajeciaRezerwacji->setCurrentIndex(-1); // Nic nie wybrane
}

void MainWindow::aktualizujInfoZajec() {
    int zajeciaId = ui->comboBoxZajeciaRezerwacji->currentData().toInt();

    if (zajeciaId <= 0) {
        wyczyscInfoZajec();
        return;
    }

    Zajecia zajecia = DatabaseManager::getZajeciaById(zajeciaId);
    if (zajecia.id <= 0) {
        wyczyscInfoZajec();
        return;
    }

    int aktualne = DatabaseManager::getIloscAktywnychRezerwacji(zajeciaId);
    int wolne = zajecia.maksUczestnikow - aktualne;

    ui->labelInfoNazwaZajec->setText(QString("Nazwa: %1").arg(zajecia.nazwa));
    ui->labelInfoTrener->setText(QString("Trener: %1").arg(zajecia.trener.isEmpty() ? "Brak" : zajecia.trener));
    ui->labelInfoDataCzas->setText(QString("Termin: %1 %2 (%3 min)")
                                       .arg(zajecia.data)
                                       .arg(zajecia.czas)
                                       .arg(zajecia.czasTrwania));

    QString tekstMiejsca = QString("Wolne miejsca: %1/%2").arg(wolne).arg(zajecia.maksUczestnikow);
    ui->labelInfoMiejsca->setText(tekstMiejsca);

    // Zmień kolor w zależności od dostępności
    if (wolne <= 0) {
        ui->labelInfoMiejsca->setStyleSheet("color: red; font-weight: bold;");
    } else if (wolne <= 3) {
        ui->labelInfoMiejsca->setStyleSheet("color: orange; font-weight: bold;");
    } else {
        ui->labelInfoMiejsca->setStyleSheet("color: green; font-weight: bold;");
    }
}

void MainWindow::wyczyscInfoZajec() {
    ui->labelInfoNazwaZajec->setText("Nazwa: -");
    ui->labelInfoTrener->setText("Trener: -");
    ui->labelInfoDataCzas->setText("Termin: -");
    ui->labelInfoMiejsca->setText("Wolne miejsca: -");
    ui->labelInfoMiejsca->setStyleSheet(""); // Usuń kolorowanie
}

bool MainWindow::walidujFormularzRezerwacji() {
    QString bledy = "";

    if (ui->comboBoxKlientRezerwacji->currentIndex() < 0) {
        bledy += "• Wybierz klienta\n";
    }

    if (ui->comboBoxZajeciaRezerwacji->currentIndex() < 0) {
        bledy += "• Wybierz zajęcia\n";
    }

    if (!bledy.isEmpty()) {
        pokazKomunikat("Błąd walidacji", "Formularz zawiera błędy:\n\n" + bledy, QMessageBox::Warning);
        return false;
    }

    return true;
}

void MainWindow::aktualizujLicznikRezerwacji() {
    int liczba = DatabaseManager::getRezerwacjeCount();
    ui->labelLiczbaRezerwacji->setText(QString("Liczba rezerwacji: %1").arg(liczba));
}

void MainWindow::aktualizujPrzyciskAnuluj() {
    bool czyWybrane = (aktualnieWybranaRezerwacjaId > 0);
    ui->pushButtonAnulujRezerwacje->setEnabled(czyWybrane);

    if (czyWybrane) {
        // Sprawdź status wybranej rezerwacji
        Rezerwacja r = DatabaseManager::getRezerwacjaById(aktualnieWybranaRezerwacjaId);
        if (r.status == "anulowana") {
            ui->pushButtonAnulujRezerwacje->setText("Już anulowana");
            ui->pushButtonAnulujRezerwacje->setEnabled(false);
        } else {
            ui->pushButtonAnulujRezerwacje->setText("Anuluj rezerwację");
            ui->pushButtonAnulujRezerwacje->setEnabled(true);
        }
    } else {
        ui->pushButtonAnulujRezerwacje->setText("Anuluj rezerwację");
    }
}

// ==================== METODY OGÓLNE ====================

void MainWindow::pokazKomunikat(const QString& tytul, const QString& tresc, QMessageBox::Icon typ) {
    QMessageBox msgBox(this);
    msgBox.setWindowTitle(tytul);
    msgBox.setText(tresc);
    msgBox.setIcon(typ);
    msgBox.exec();
}

// ==================== SLOTS DLA KARNETÓW ====================

void MainWindow::dodajKarnet() {
    if (!walidujFormularzKarnetu()) {
        return;
    }

    Karnet karnet = pobierzDaneKarnetuZFormularza();

    bool sukces = DatabaseManager::addKarnet(
        karnet.idKlienta,
        karnet.typ,
        karnet.dataRozpoczecia,
        karnet.dataZakonczenia,
        karnet.cena,
        karnet.czyAktywny
        );

    if (sukces) {
        pokazKomunikat("Sukces", "Karnet został dodany pomyślnie!", QMessageBox::Information);
        wyczyscFormularzKarnetu();
        odswiezListeKarnetow();
        ui->statusbar->showMessage("Dodano nowy karnet", 3000);
    } else {
        pokazKomunikat("Błąd", "Nie udało się dodać karnetu.\nSprawdź czy klient nie ma już aktywnego karnetu tego typu.", QMessageBox::Warning);
    }
}

void MainWindow::edytujKarnet() {
    if (aktualnieEdytowanyKarnetId <= 0) {
        pokazKomunikat("Błąd", "Nie wybrano karnetu do edycji.", QMessageBox::Warning);
        return;
    }

    if (!walidujFormularzKarnetu()) {
        return;
    }

    Karnet karnet = pobierzDaneKarnetuZFormularza();

    bool sukces = DatabaseManager::updateKarnet(
        aktualnieEdytowanyKarnetId,
        karnet.idKlienta,
        karnet.typ,
        karnet.dataRozpoczecia,
        karnet.dataZakonczenia,
        karnet.cena,
        karnet.czyAktywny
        );

    if (sukces) {
        pokazKomunikat("Sukces", "Dane karnetu zostały zaktualizowane!", QMessageBox::Information);
        odswiezListeKarnetow();
        ustawTrybDodawaniaKarnetu();
        ui->statusbar->showMessage("Zaktualizowano dane karnetu", 3000);
    } else {
        pokazKomunikat("Błąd", "Nie udało się zaktualizować danych karnetu.", QMessageBox::Warning);
    }
}

void MainWindow::usunKarnet() {
    if (aktualnieEdytowanyKarnetId <= 0) {
        pokazKomunikat("Błąd", "Nie wybrano karnetu do usunięcia.", QMessageBox::Warning);
        return;
    }

    QMessageBox::StandardButton odpowiedz = QMessageBox::question(
        this,
        "Potwierdzenie",
        QString("Czy na pewno chcesz usunąć karnet?\n\nTa operacja jest nieodwracalna!"),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
        );

    if (odpowiedz != QMessageBox::Yes) {
        return;
    }

    bool sukces = DatabaseManager::deleteKarnet(aktualnieEdytowanyKarnetId);

    if (sukces) {
        pokazKomunikat("Sukces", "Karnet został usunięty!", QMessageBox::Information);
        wyczyscFormularzKarnetu();
        odswiezListeKarnetow();
        ustawTrybDodawaniaKarnetu();
        ui->statusbar->showMessage("Usunięto karnet", 3000);
    } else {
        pokazKomunikat("Błąd", "Nie udało się usunąć karnetu.", QMessageBox::Critical);
    }
}

void MainWindow::wyczyscFormularzKarnetu() {
    ui->comboBoxKlientKarnetu->setCurrentIndex(-1);
    ui->comboBoxTypKarnetu->setCurrentIndex(0);
    ui->dateEditRozpocKarnetu->setDate(QDate::currentDate());
    ui->dateEditZakonKarnetu->setDate(QDate::currentDate().addMonths(1));
    ui->doubleSpinBoxCenaKarnetu->setValue(150.0);
    ui->comboBoxStatusKarnetu->setCurrentIndex(0); // Aktywny

    wyczyscInfoKlienta();
    ustawTrybDodawaniaKarnetu();
}

void MainWindow::filtrujKarnety() {
    QString typFilter = ui->comboBoxFilterTypKarnetu->currentText();
    QString statusFilter = ui->comboBoxFilterStatusKarnetu->currentText();

    QList<Karnet> wszystkieKarnety = DatabaseManager::getAllKarnety();
    QList<Karnet> przefiltrowane;

    for (const Karnet& k : wszystkieKarnety) {
        bool pasuje = true;

        // Filtr typu
        if (typFilter == "Tylko normalne" && k.typ != "normalny") {
            pasuje = false;
        } else if (typFilter == "Tylko studenckie" && k.typ != "studencki") {
            pasuje = false;
        }

        // Filtr statusu
        if (statusFilter == "Tylko aktywne" && !k.czyAktywny) {
            pasuje = false;
        } else if (statusFilter == "Tylko nieaktywne" && k.czyAktywny) {
            pasuje = false;
        }

        if (pasuje) {
            przefiltrowane.append(k);
        }
    }

    zaladujKarnetyDoTabeli(przefiltrowane);
    ui->statusbar->showMessage(QString("Przefiltrowano do %1 karnetów").arg(przefiltrowane.size()), 3000);
}

void MainWindow::odswiezListeKarnetow() {
    QList<Karnet> karnety = DatabaseManager::getAllKarnety();
    zaladujKarnetyDoTabeli(karnety);
    aktualizujLicznikKarnetow();
    zaladujKlientowDoComboBoxKarnetu();
    ui->statusbar->showMessage("Lista karnetów odświeżona", 2000);
}

void MainWindow::karnetWybrany() {
    int aktualnyWiersz = ui->tableWidgetKarnety->currentRow();

    if (aktualnyWiersz < 0) {
        ustawTrybDodawaniaKarnetu();
        return;
    }

    QTableWidgetItem* idItem = ui->tableWidgetKarnety->item(aktualnyWiersz, 0);
    if (!idItem) {
        return;
    }

    int karnetId = idItem->text().toInt();
    Karnet karnet = DatabaseManager::getKarnetById(karnetId);

    if (karnet.id > 0) {
        zaladujKarnetDoFormularza(karnet);
        ustawTrybEdycjiKarnetu();
        aktualnieEdytowanyKarnetId = karnet.id;
    }
}

void MainWindow::klientKarnetuWybrany() {
    aktualizujInfoKlienta();
}

void MainWindow::pokazStatystykiKarnetow() {
    QList<QPair<QString, int>> statystyki = DatabaseManager::getStatystykiKarnetow();
    double przychody = DatabaseManager::getCalkowitePrzychodyZKarnetow();
    int aktywne = DatabaseManager::getLiczbaAktywnychKarnetow();

    QString tekst = "📊 Statystyki karnetów:\n\n";

    tekst += QString("Aktywnych karnetów: %1\n").arg(aktywne);
    tekst += QString("Łączne przychody: %.2f zł\n\n").arg(przychody);

    if (!statystyki.isEmpty()) {
        tekst += "Karnety wg typów:\n";
        for (const auto& stat : statystyki) {
            tekst += QString("• %1: %2 karnetów\n").arg(stat.first).arg(stat.second);
        }
    } else {
        tekst += "Brak aktywnych karnetów.";
    }

    pokazKomunikat("Statystyki karnetów", tekst, QMessageBox::Information);
}

void MainWindow::pokazWygasajaceKarnety() {
    QString dzisiaj = QDate::currentDate().toString("yyyy-MM-dd");
    QString za30dni = QDate::currentDate().addDays(30).toString("yyyy-MM-dd");

    QList<Karnet> wygasajace = DatabaseManager::getKarnetyWygasajace(dzisiaj, za30dni);

    QString tekst = "⚠️ Karnety wygasające w ciągu 30 dni:\n\n";

    if (wygasajace.isEmpty()) {
        tekst += "Brak karnetów wygasających w najbliższym czasie.";
    } else {
        for (const Karnet& k : wygasajace) {
            tekst += QString("• %1 %2 (%3)\n  Typ: %4, wygasa: %5\n\n")
                         .arg(k.imieKlienta)
                         .arg(k.nazwiskoKlienta)
                         .arg(k.emailKlienta.isEmpty() ? "brak email" : k.emailKlienta)
                         .arg(k.typ)
                         .arg(k.dataZakonczenia);
        }
    }

    pokazKomunikat("Wygasające karnety", tekst, QMessageBox::Warning);
}

// ==================== METODY POMOCNICZE - KARNETY ====================

void MainWindow::setupTableKarnety() {
    // Konfiguracja tabeli karnetów
    ui->tableWidgetKarnety->setColumnCount(8);

    QStringList headers = {"ID", "Klient", "Email", "Typ", "Data rozpoczęcia", "Data zakończenia", "Cena", "Status"};
    ui->tableWidgetKarnety->setHorizontalHeaderLabels(headers);

    // Ukryj kolumnę ID
    ui->tableWidgetKarnety->setColumnHidden(0, true);

    // Ustaw tryb zaznaczania całych wierszy
    ui->tableWidgetKarnety->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidgetKarnety->setSelectionMode(QAbstractItemView::SingleSelection);

    // Ustaw szerokości kolumn
    QHeaderView* header = ui->tableWidgetKarnety->horizontalHeader();
    header->setStretchLastSection(true);
    header->resizeSection(1, 150); // Klient
    header->resizeSection(2, 200); // Email
    header->resizeSection(3, 100); // Typ
    header->resizeSection(4, 120); // Data rozpoczęcia
    header->resizeSection(5, 120); // Data zakończenia
    header->resizeSection(6, 80);  // Cena
}

void MainWindow::zaladujKarnetyDoTabeli(const QList<Karnet>& karnety) {
    ui->tableWidgetKarnety->setRowCount(karnety.size());

    for (int i = 0; i < karnety.size(); ++i) {
        const Karnet& k = karnety[i];

        ui->tableWidgetKarnety->setItem(i, 0, new QTableWidgetItem(QString::number(k.id)));
        ui->tableWidgetKarnety->setItem(i, 1, new QTableWidgetItem(QString("%1 %2").arg(k.imieKlienta).arg(k.nazwiskoKlienta)));
        ui->tableWidgetKarnety->setItem(i, 2, new QTableWidgetItem(k.emailKlienta));
        ui->tableWidgetKarnety->setItem(i, 3, new QTableWidgetItem(k.typ));
        ui->tableWidgetKarnety->setItem(i, 4, new QTableWidgetItem(k.dataRozpoczecia));
        ui->tableWidgetKarnety->setItem(i, 5, new QTableWidgetItem(k.dataZakonczenia));
        ui->tableWidgetKarnety->setItem(i, 6, new QTableWidgetItem(QString("%1 zł").arg(k.cena, 0, 'f', 2)));

        // Status z kolorowym tłem
        QTableWidgetItem* statusItem = new QTableWidgetItem(k.czyAktywny ? "Aktywny" : "Nieaktywny");
        if (k.czyAktywny) {
            statusItem->setBackground(QBrush(QColor(144, 238, 144))); // Jasny zielony
        } else {
            statusItem->setBackground(QBrush(QColor(255, 182, 193))); // Jasny czerwony
        }
        ui->tableWidgetKarnety->setItem(i, 7, statusItem);
    }
}

void MainWindow::zaladujKarnetDoFormularza(const Karnet& karnet) {
    // Znajdź i ustaw klienta w ComboBox
    for (int i = 0; i < ui->comboBoxKlientKarnetu->count(); i++) {
        if (ui->comboBoxKlientKarnetu->itemData(i).toInt() == karnet.idKlienta) {
            ui->comboBoxKlientKarnetu->setCurrentIndex(i);
            break;
        }
    }

    // Ustaw typ karnetu
    int typIndex = ui->comboBoxTypKarnetu->findText(karnet.typ);
    if (typIndex >= 0) {
        ui->comboBoxTypKarnetu->setCurrentIndex(typIndex);
    }

    // Ustaw daty
    if (!karnet.dataRozpoczecia.isEmpty()) {
        QDate dataRozp = QDate::fromString(karnet.dataRozpoczecia, "yyyy-MM-dd");
        if (dataRozp.isValid()) {
            ui->dateEditRozpocKarnetu->setDate(dataRozp);
        }
    }

    if (!karnet.dataZakonczenia.isEmpty()) {
        QDate dataZak = QDate::fromString(karnet.dataZakonczenia, "yyyy-MM-dd");
        if (dataZak.isValid()) {
            ui->dateEditZakonKarnetu->setDate(dataZak);
        }
    }

    // Ustaw cenę
    ui->doubleSpinBoxCenaKarnetu->setValue(karnet.cena);

    // Ustaw status
    ui->comboBoxStatusKarnetu->setCurrentIndex(karnet.czyAktywny ? 0 : 1);

    // Aktualizuj informacje o kliencie
    aktualizujInfoKlienta();
}

void MainWindow::zaladujKlientowDoComboBoxKarnetu() {
    ui->comboBoxKlientKarnetu->clear();

    QList<Klient> klienci = DatabaseManager::getAllKlienci();
    for (const Klient& k : klienci) {
        QString tekst = QString("%1 %2").arg(k.imie).arg(k.nazwisko);
        if (!k.email.isEmpty()) {
            tekst += QString(" (%1)").arg(k.email);
        }
        ui->comboBoxKlientKarnetu->addItem(tekst, k.id);
    }

    ui->comboBoxKlientKarnetu->setCurrentIndex(-1); // Nic nie wybrane
}

Karnet MainWindow::pobierzDaneKarnetuZFormularza() {
    Karnet karnet = {};

    karnet.idKlienta = ui->comboBoxKlientKarnetu->currentData().toInt();
    karnet.typ = ui->comboBoxTypKarnetu->currentText();
    karnet.dataRozpoczecia = ui->dateEditRozpocKarnetu->date().toString("yyyy-MM-dd");
    karnet.dataZakonczenia = ui->dateEditZakonKarnetu->date().toString("yyyy-MM-dd");
    karnet.cena = ui->doubleSpinBoxCenaKarnetu->value();
    karnet.czyAktywny = (ui->comboBoxStatusKarnetu->currentIndex() == 0);

    return karnet;
}

bool MainWindow::walidujFormularzKarnetu() {
    QString bledy = "";

    if (ui->comboBoxKlientKarnetu->currentIndex() < 0) {
        bledy += "• Wybierz klienta\n";
    }

    if (ui->dateEditRozpocKarnetu->date() >= ui->dateEditZakonKarnetu->date()) {
        bledy += "• Data zakończenia musi być późniejsza niż data rozpoczęcia\n";
    }

    if (ui->doubleSpinBoxCenaKarnetu->value() <= 0) {
        bledy += "• Cena musi być większa od zera\n";
    }

    if (!bledy.isEmpty()) {
        pokazKomunikat("Błąd walidacji", "Formularz zawiera błędy:\n\n" + bledy, QMessageBox::Warning);
        return false;
    }

    return true;
}

void MainWindow::ustawTrybDodawaniaKarnetu() {
    aktualnieEdytowanyKarnetId = -1;

    ui->pushButtonDodajKarnet->setEnabled(true);
    ui->pushButtonEdytujKarnet->setEnabled(false);
    ui->pushButtonUsunKarnet->setEnabled(false);

    ui->labelFormularzKarnetuTitle->setText("Dodaj nowy karnet");

    ui->tableWidgetKarnety->clearSelection();
}

void MainWindow::ustawTrybEdycjiKarnetu() {
    ui->pushButtonDodajKarnet->setEnabled(false);
    ui->pushButtonEdytujKarnet->setEnabled(true);
    ui->pushButtonUsunKarnet->setEnabled(true);

    ui->labelFormularzKarnetuTitle->setText("Edytuj karnet");
}

void MainWindow::aktualizujLicznikKarnetow() {
    int liczba = DatabaseManager::getKarnetyCount();
    ui->labelLiczbaKarnetow->setText(QString("Liczba karnetów: %1").arg(liczba));
}

void MainWindow::aktualizujInfoKlienta() {
    int klientId = ui->comboBoxKlientKarnetu->currentData().toInt();

    if (klientId <= 0) {
        wyczyscInfoKlienta();
        return;
    }

    Klient klient = DatabaseManager::getKlientById(klientId);
    if (klient.id <= 0) {
        wyczyscInfoKlienta();
        return;
    }

    QList<Karnet> aktywneKarnety = DatabaseManager::getAktywneKarnetyKlienta(klientId);

    ui->labelInfoImieNazwisko->setText(QString("Klient: %1 %2").arg(klient.imie).arg(klient.nazwisko));
    ui->labelInfoEmailKlient->setText(QString("Email: %1").arg(klient.email.isEmpty() ? "Brak" : klient.email));
    ui->labelInfoAktywneKarnety->setText(QString("Aktywne karnety: %1").arg(aktywneKarnety.size()));

    // Zmień kolor w zależności od liczby karnetów
    if (aktywneKarnety.size() > 1) {
        ui->labelInfoAktywneKarnety->setStyleSheet("color: orange; font-weight: bold;");
    } else if (aktywneKarnety.size() == 1) {
        ui->labelInfoAktywneKarnety->setStyleSheet("color: green; font-weight: bold;");
    } else {
        ui->labelInfoAktywneKarnety->setStyleSheet("color: gray; font-weight: bold;");
    }
}

void MainWindow::wyczyscInfoKlienta() {
    ui->labelInfoImieNazwisko->setText("Klient: -");
    ui->labelInfoEmailKlient->setText("Email: -");
    ui->labelInfoAktywneKarnety->setText("Aktywne karnety: -");
    ui->labelInfoAktywneKarnety->setStyleSheet(""); // Usuń kolorowanie
}

void MainWindow::obliczCeneKarnetu() {
    QString typ = ui->comboBoxTypKarnetu->currentText();

    if (typ == "normalny") {
        ui->doubleSpinBoxCenaKarnetu->setValue(150.0);
    } else if (typ == "studencki") {
        ui->doubleSpinBoxCenaKarnetu->setValue(100.0);
    }
}
