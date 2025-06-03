#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "DatabaseManager.h"
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QDebug>
#include <QHeaderView>
#include <QMessageBox>
#include <QDate>
#include <QApplication>

// === KONSTRUKTOR I DESTRUKTOR ===

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , aktualnieEdytowanyKlientId(-1)
{
    ui->setupUi(this);
    setupUI();
    setupConnections();
    setupTable();
    odswiezListeKlientow();
}

MainWindow::~MainWindow()
{
    delete ui;
}

// === TWORZENIE TABEL ===

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

// === SETUP METODY ===

void MainWindow::setupUI() {
    // Ustaw domyślną datę urodzenia na dzisiaj minus 18 lat
    ui->dateEditUrodzenia->setDate(QDate::currentDate().addYears(-18));

    // Ustaw tryb dodawania na starcie
    ustawTrybDodawania();

    // Ustaw status bar
    ui->statusbar->showMessage("Gotowy");
}

void MainWindow::setupConnections() {
    // === Przyciski formularza ===
    connect(ui->pushButtonDodaj, &QPushButton::clicked, this, &MainWindow::dodajKlienta);
    connect(ui->pushButtonEdytuj, &QPushButton::clicked, this, &MainWindow::edytujKlienta);
    connect(ui->pushButtonUsun, &QPushButton::clicked, this, &MainWindow::usunKlienta);
    connect(ui->pushButtonWyczysc, &QPushButton::clicked, this, &MainWindow::wyczyscFormularz);

    // === Przyciski wyszukiwania ===
    connect(ui->pushButtonSearch, &QPushButton::clicked, this, &MainWindow::wyszukajKlientow);
    connect(ui->pushButtonShowAll, &QPushButton::clicked, this, &MainWindow::pokazWszystkichKlientow);
    connect(ui->pushButtonOdswiez, &QPushButton::clicked, this, &MainWindow::odswiezListeKlientow);

    // === Wyszukiwanie Enter ===
    connect(ui->lineEditSearch, &QLineEdit::returnPressed, this, &MainWindow::wyszukajKlientow);

    // === Tabela ===
    connect(ui->tableWidgetKlienci, &QTableWidget::itemSelectionChanged, this, &MainWindow::klientWybrany);

    // === Menu ===
    connect(ui->actionZamknij, &QAction::triggered, this, &MainWindow::zamknijAplikacje);
    connect(ui->actionOProgramie, &QAction::triggered, this, &MainWindow::oProgramie);
}

void MainWindow::setupTable() {
    // Konfiguracja tabeli klientów
    ui->tableWidgetKlienci->setColumnCount(7);

    QStringList headers = {"ID", "Imię", "Nazwisko", "Email", "Telefon", "Data urodzenia", "Data rejestracji"};
    ui->tableWidgetKlienci->setHorizontalHeaderLabels(headers);

    // Ukryj kolumnę ID
    ui->tableWidgetKlienci->setColumnHidden(0, true);

    // Ustaw tryb zaznaczania całych wierszy
    ui->tableWidgetKlienci->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidgetKlienci->setSelectionMode(QAbstractItemView::SingleSelection);

    // Ustaw nagłówki tabeli
    QHeaderView* header = ui->tableWidgetKlienci->horizontalHeader();
    header->setStretchLastSection(true);
    header->resizeSection(1, 120); // Imię
    header->resizeSection(2, 120); // Nazwisko
    header->resizeSection(3, 200); // Email
    header->resizeSection(4, 120); // Telefon
    header->resizeSection(5, 120); // Data urodzenia
}

// === SLOTS DLA ZARZĄDZANIA KLIENTAMI ===

void MainWindow::dodajKlienta() {
    if (!walidujFormularz()) {
        return;
    }

    Klient klient = pobierzDaneZFormularza();

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
        wyczyscFormularz();
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

    if (!walidujFormularz()) {
        return;
    }

    Klient klient = pobierzDaneZFormularza();

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
        ustawTrybDodawania();
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

    // Potwierdzenie usunięcia
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
        wyczyscFormularz();
        odswiezListeKlientow();
        ustawTrybDodawania();
        ui->statusbar->showMessage("Usunięto klienta", 3000);
    } else {
        pokazKomunikat("Błąd", "Nie udało się usunąć klienta.", QMessageBox::Critical);
    }
}

void MainWindow::wyczyscFormularz() {
    ui->lineEditImie->clear();
    ui->lineEditNazwisko->clear();
    ui->lineEditEmail->clear();
    ui->lineEditTelefon->clear();
    ui->dateEditUrodzenia->setDate(QDate::currentDate().addYears(-18));
    ui->textEditUwagi->clear();

    ustawTrybDodawania();
}

// === SLOTS DLA WYSZUKIWANIA ===

void MainWindow::wyszukajKlientow() {
    QString nazwisko = ui->lineEditSearch->text().trimmed();

    if (nazwisko.isEmpty()) {
        pokazWszystkichKlientow();
        return;
    }

    QList<Klient> klienci = DatabaseManager::searchKlienciByNazwisko(nazwisko);
    zaladujKlientowDoTabeli(klienci);

    ui->statusbar->showMessage(QString("Znaleziono %1 klientów").arg(klienci.size()), 3000);
}

void MainWindow::pokazWszystkichKlientow() {
    ui->lineEditSearch->clear();
    odswiezListeKlientow();
}

void MainWindow::odswiezListeKlientow() {
    QList<Klient> klienci = DatabaseManager::getAllKlienci();
    zaladujKlientowDoTabeli(klienci);
    aktualizujLicznikKlientow();
    ui->statusbar->showMessage("Lista klientów odświeżona", 2000);
}

// === SLOTS DLA TABELI ===

void MainWindow::klientWybrany() {
    int aktualnyWiersz = ui->tableWidgetKlienci->currentRow();

    if (aktualnyWiersz < 0) {
        ustawTrybDodawania();
        return;
    }

    // Pobierz ID klienta z ukrytej kolumny
    QTableWidgetItem* idItem = ui->tableWidgetKlienci->item(aktualnyWiersz, 0);
    if (!idItem) {
        return;
    }

    int klientId = idItem->text().toInt();
    Klient klient = DatabaseManager::getKlientById(klientId);

    if (klient.id > 0) {
        zaladujKlientaDoFormularza(klient);
        ustawTrybEdycji();
        aktualnieEdytowanyKlientId = klient.id;
    }
}

// === SLOTS MENU ===

void MainWindow::zamknijAplikacje() {
    QApplication::quit();
}

void MainWindow::oProgramie() {
    QMessageBox::about(this, "O programie",
                       "Zarządzanie Siłownią v1.0\n\n"
                       "System do zarządzania klientami, karnetami i zajęciami siłowni.\n\n"
                       "Funkcje:\n"
                       "• Zarządzanie klientami\n"
                       "• Wyszukiwanie i filtrowanie\n"
                       "• Dodawanie, edycja i usuwanie danych\n\n"
                       "Autor: Twoje Imię\n"
                       "Technologia: Qt + SQLite"
                       );
}

// === METODY POMOCNICZE ===

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

    // Data urodzenia
    if (!klient.dataUrodzenia.isEmpty()) {
        QDate data = QDate::fromString(klient.dataUrodzenia, "yyyy-MM-dd");
        if (data.isValid()) {
            ui->dateEditUrodzenia->setDate(data);
        }
    }
}

Klient MainWindow::pobierzDaneZFormularza() {
    Klient klient = {};

    klient.imie = ui->lineEditImie->text().trimmed();
    klient.nazwisko = ui->lineEditNazwisko->text().trimmed();
    klient.email = ui->lineEditEmail->text().trimmed();
    klient.telefon = ui->lineEditTelefon->text().trimmed();
    klient.dataUrodzenia = ui->dateEditUrodzenia->date().toString("yyyy-MM-dd");
    klient.uwagi = ui->textEditUwagi->toPlainText().trimmed();

    return klient;
}

bool MainWindow::walidujFormularz() {
    QString bledy = "";

    // Sprawdź wymagane pola
    if (ui->lineEditImie->text().trimmed().isEmpty()) {
        bledy += "• Imię jest wymagane\n";
    }

    if (ui->lineEditNazwisko->text().trimmed().isEmpty()) {
        bledy += "• Nazwisko jest wymagane\n";
    }

    // Sprawdź email (jeśli podano)
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

void MainWindow::ustawTrybDodawania() {
    aktualnieEdytowanyKlientId = -1;

    ui->pushButtonDodaj->setEnabled(true);
    ui->pushButtonEdytuj->setEnabled(false);
    ui->pushButtonUsun->setEnabled(false);

    ui->labelFormularzTitle->setText("Dodaj nowego klienta");

    // Wyczyść zaznaczenie w tabeli
    ui->tableWidgetKlienci->clearSelection();
}

void MainWindow::ustawTrybEdycji() {
    ui->pushButtonDodaj->setEnabled(false);
    ui->pushButtonEdytuj->setEnabled(true);
    ui->pushButtonUsun->setEnabled(true);

    ui->labelFormularzTitle->setText("Edytuj klienta");
}

void MainWindow::pokazKomunikat(const QString& tytul, const QString& tresc, QMessageBox::Icon typ) {
    QMessageBox msgBox(this);
    msgBox.setWindowTitle(tytul);
    msgBox.setText(tresc);
    msgBox.setIcon(typ);
    msgBox.exec();
}

void MainWindow::aktualizujLicznikKlientow() {
    int liczba = DatabaseManager::getKlienciCount();
    ui->labelLiczbaKlientow->setText(QString("Liczba klientów: %1").arg(liczba));
}
