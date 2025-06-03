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

// ==================== KONSTRUKTOR I DESTRUKTOR ====================

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , aktualnieEdytowanyKlientId(-1)
    , aktualnieEdytowaneZajeciaId(-1)
{
    ui->setupUi(this);
    setupUI();
    setupConnections();
    setupTableKlienci();
    setupTableZajecia();

    // Załaduj dane do obu tabel
    odswiezListeKlientow();
    odswiezListeZajec();
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

// ==================== SETUP METODY ====================

void MainWindow::setupUI() {
    // Ustaw domyślne daty
    ui->dateEditUrodzenia->setDate(QDate::currentDate().addYears(-18));
    ui->dateEditZajecia->setDate(QDate::currentDate());
    ui->dateEditFilterZajecia->setDate(QDate::currentDate());

    // Ustaw domyślny czas
    ui->timeEditZajecia->setTime(QTime(9, 0)); // 09:00

    // Ustaw tryby dodawania na starcie
    ustawTrybDodawaniaKlienta();
    ustawTrybDodawaniaZajec();

    // Ustaw status bar
    ui->statusbar->showMessage("Gotowy");

    // Ustaw pierwszą zakładkę jako aktywną
    ui->tabWidget->setCurrentIndex(0);
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

    // === MENU ===
    connect(ui->actionZamknij, &QAction::triggered, this, &MainWindow::zamknijAplikacje);
    connect(ui->actionOProgramie, &QAction::triggered, this, &MainWindow::oProgramie);
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
                       "Zarządzanie Siłownią v2.0\n\n"
                       "System do zarządzania klientami, zajęciami i karnetami siłowni.\n\n"
                       "Funkcje:\n"
                       "• Zarządzanie klientami\n"
                       "• Zarządzanie zajęciami grupowymi\n"
                       "• Wyszukiwanie i filtrowanie\n"
                       "• Dodawanie, edycja i usuwanie danych\n\n"
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

// ==================== METODY OGÓLNE ====================

void MainWindow::pokazKomunikat(const QString& tytul, const QString& tresc, QMessageBox::Icon typ) {
    QMessageBox msgBox(this);
    msgBox.setWindowTitle(tytul);
    msgBox.setText(tresc);
    msgBox.setIcon(typ);
    msgBox.exec();
}
