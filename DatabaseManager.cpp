#include "DatabaseManager.h"
#include <QtSql/QSqlError>
#include <QDebug>

QSqlDatabase DatabaseManager::db = QSqlDatabase();

bool DatabaseManager::connect(const QString& path) {
    // Jeśli już wcześniej dodaliśmy połączenie o tej nazwie, pobieramy je
    if (QSqlDatabase::contains("gym_connection")) {
        db = QSqlDatabase::database("gym_connection");
    } else {
        // Dodajemy nowe połączenie SQLite
        db = QSqlDatabase::addDatabase("QSQLITE", "gym_connection");
        db.setDatabaseName(path);
    }

    if (!db.open()) {
        qWarning() << "Nie udało się otworzyć bazy danych:" << db.lastError().text();
        return false;
    }
    return true;
}

void DatabaseManager::disconnect() {
    if (db.isOpen()) {
        db.close();
    }
    QSqlDatabase::removeDatabase("gym_connection");
}

QSqlDatabase& DatabaseManager::instance() {
    return db;
}
