#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QtSql/QSqlDatabase>

class DatabaseManager {
public:
    // Próbuje otworzyć/utworzyć plik bazy SQLite o podanej ścieżce.
    // Zwraca true, jeśli udało się połączyć.
    static bool connect(const QString& path);

    // Zamyka połączenie i usuwa je z puli
    static void disconnect();

    // Zwraca referencję do instancji QSqlDatabase
    static QSqlDatabase& instance();

private:
    DatabaseManager() = default;
    static QSqlDatabase db;
};

#endif // DATABASEMANAGER_H
