#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "DatabaseManager.h"
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QDebug>

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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}
