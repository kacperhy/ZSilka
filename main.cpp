#include <QApplication>
#include "mainwindow.h"
#include "DatabaseManager.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    // 1) Ustalamy ścieżkę do pliku bazy (np. wewnątrz katalogu aplikacji)
    QString dbPath = QApplication::applicationDirPath() + "/gym.db";
    if (!DatabaseManager::connect(dbPath)) {
        return -1; // wyjdź, jeśli nie udało się otworzyć bazy
    }

    MainWindow w;
    w.createTablesIfNotExist(); // tworzymy tabele (jeśli nie istnieją)
    w.show();

    int ret = a.exec();
    DatabaseManager::disconnect();
    return ret;
}
