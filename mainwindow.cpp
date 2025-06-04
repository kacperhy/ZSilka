// Dodaj te include na górze pliku mainwindow.cpp (po istniejących)
#include <QFileDialog>
#include <QProgressDialog>
#include <QStandardPaths>
#include <QDesktopServices>
#include <QUrl>

// Dodaj te połączenia w setupConnections() (przed końcowym })

// === MENU CSV ===
connect(ui->actionEksportKlienciCSV, &QAction::triggered, this, &MainWindow::eksportKlienciCSV);
connect(ui->actionEksportZajeciaCSV, &QAction::triggered, this, &MainWindow::eksportZajeciaCSV);
connect(ui->actionEksportRezerwacjeCSV, &QAction::triggered, this, &MainWindow::eksportRezerwacjeCSV);
connect(ui->actionEksportKarnetyCSV, &QAction::triggered, this, &MainWindow::eksportKarnetyCSV);
connect(ui->actionEksportWszystkieCSV, &QAction::triggered, this, &MainWindow::eksportWszystkieCSV);

connect(ui->actionImportKlienciCSV, &QAction::triggered, this, &MainWindow::importKlienciCSV);
connect(ui->actionImportZajeciaCSV, &QAction::triggered, this, &MainWindow::importZajeciaCSV);
connect(ui->actionImportRezerwacjeCSV, &QAction::triggered, this, &MainWindow::importRezerwacjeCSV);
connect(ui->actionImportKarnetyCSV, &QAction::triggered, this, &MainWindow::importKarnetyCSV);

// Dodaj te funkcje na końcu pliku mainwindow.cpp (przed końcowym })

// ==================== SLOTS DLA EKSPORTU CSV ====================

void MainWindow::eksportKlienciCSV() {
    QString fileName = getCSVSaveFileName("klienci.csv", "Eksportuj klientów do CSV");
    if (fileName.isEmpty()) {
        return;
    }

    if (DatabaseManager::exportKlienciToCSV(fileName)) {
        pokazKomunikat("Sukces",
                       QString("Pomyślnie wyeksportowano %1 klientów do pliku:\n%2")
                           .arg(DatabaseManager::getKlienciCount())
                           .arg(fileName),
                       QMessageBox::Information);

        // Zapytaj czy otworzyć folder
        QMessageBox::StandardButton odpowiedz = QMessageBox::question(
            this,
            "Otwórz folder",
            "Czy chcesz otworzyć folder z wyeksportowanym plikiem?",
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No
            );

        if (odpowiedz == QMessageBox::Yes) {
            QFileInfo fileInfo(fileName);
            QDesktopServices::openUrl(QUrl::fromLocalFile(fileInfo.absolutePath()));
        }
    } else {
        pokazKomunikat("Błąd",
                       "Nie udało się wyeksportować klientów do pliku CSV.",
                       QMessageBox::Critical);
    }
}

void MainWindow::eksportZajeciaCSV() {
    QString fileName = getCSVSaveFileName("zajecia.csv", "Eksportuj zajęcia do CSV");
    if (fileName.isEmpty()) {
        return;
    }

    if (DatabaseManager::exportZajeciaToCSV(fileName)) {
        pokazKomunikat("Sukces",
                       QString("Pomyślnie wyeksportowano %1 zajęć do pliku:\n%2")
                           .arg(DatabaseManager::getZajeciaCount())
                           .arg(fileName),
                       QMessageBox::Information);
    } else {
        pokazKomunikat("Błąd",
                       "Nie udało się wyeksportować zajęć do pliku CSV.",
                       QMessageBox::Critical);
    }
}

void MainWindow::eksportRezerwacjeCSV() {
    QString fileName = getCSVSaveFileName("rezerwacje.csv", "Eksportuj rezerwacje do CSV");
    if (fileName.isEmpty()) {
        return;
    }

    if (DatabaseManager::exportRezerwacjeToCSV(fileName)) {
        pokazKomunikat("Sukces",
                       QString("Pomyślnie wyeksportowano %1 rezerwacji do pliku:\n%2")
                           .arg(DatabaseManager::getRezerwacjeCount())
                           .arg(fileName),
                       QMessageBox::Information);
    } else {
        pokazKomunikat("Błąd",
                       "Nie udało się wyeksportować rezerwacji do pliku CSV.",
                       QMessageBox::Critical);
    }
}

void MainWindow::eksportKarnetyCSV() {
    QString fileName = getCSVSaveFileName("karnety.csv", "Eksportuj karnety do CSV");
    if (fileName.isEmpty()) {
        return;
    }

    if (DatabaseManager::exportKarnetyToCSV(fileName)) {
        pokazKomunikat("Sukces",
                       QString("Pomyślnie wyeksportowano %1 karnetów do pliku:\n%2")
                           .arg(DatabaseManager::getKarnetyCount())
                           .arg(fileName),
                       QMessageBox::Information);
    } else {
        pokazKomunikat("Błąd",
                       "Nie udało się wyeksportować karnetów do pliku CSV.",
                       QMessageBox::Critical);
    }
}

void MainWindow::eksportWszystkieCSV() {
    QString dirPath = getDirectoryPath("Wybierz katalog do eksportu wszystkich danych");
    if (dirPath.isEmpty()) {
        return;
    }

    QProgressDialog progress("Eksportowanie danych...", "Anuluj", 0, 4, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.setAutoClose(true);
    progress.setAutoReset(true);

    progress.setValue(0);
    progress.setLabelText("Eksportowanie klientów...");
    QApplication::processEvents();

    bool success = true;
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd_HH-mm-ss");
    QStringList exportedFiles;

    // Eksport klientów
    QString klienciFile = dirPath + "/klienci_" + timestamp + ".csv";
    if (DatabaseManager::exportKlienciToCSV(klienciFile)) {
        exportedFiles << klienciFile;
    } else {
        success = false;
    }
    progress.setValue(1);

    if (progress.wasCanceled() || !success) {
        return;
    }

    // Eksport zajęć
    progress.setLabelText("Eksportowanie zajęć...");
    QApplication::processEvents();
    QString zajeciaFile = dirPath + "/zajecia_" + timestamp + ".csv";
    if (DatabaseManager::exportZajeciaToCSV(zajeciaFile)) {
        exportedFiles << zajeciaFile;
    } else {
        success = false;
    }
    progress.setValue(2);

    if (progress.wasCanceled() || !success) {
        return;
    }

    // Eksport rezerwacji
    progress.setLabelText("Eksportowanie rezerwacji...");
    QApplication::processEvents();
    QString rezerwacjeFile = dirPath + "/rezerwacje_" + timestamp + ".csv";
    if (DatabaseManager::exportRezerwacjeToCSV(rezerwacjeFile)) {
        exportedFiles << rezerwacjeFile;
    } else {
        success = false;
    }
    progress.setValue(3);

    if (progress.wasCanceled() || !success) {
        return;
    }

    // Eksport karnetów
    progress.setLabelText("Eksportowanie karnetów...");
    QApplication::processEvents();
    QString karnetyFile = dirPath + "/karnety_" + timestamp + ".csv";
    if (DatabaseManager::exportKarnetyToCSV(karnetyFile)) {
        exportedFiles << karnetyFile;
    } else {
        success = false;
    }
    progress.setValue(4);

    if (success) {
        QString message = "Pomyślnie wyeksportowano wszystkie dane!\n\nWyeksportowane pliki:\n";
        for (const QString& file : exportedFiles) {
            QFileInfo info(file);
            message += "• " + info.fileName() + "\n";
        }

        pokazKomunikat("Sukces", message, QMessageBox::Information);

        // Zapytaj czy otworzyć folder
        QMessageBox::StandardButton odpowiedz = QMessageBox::question(
            this,
            "Otwórz folder",
            "Czy chcesz otworzyć folder z wyeksportowanymi plikami?",
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::Yes
            );

        if (odpowiedz == QMessageBox::Yes) {
            QDesktopServices::openUrl(QUrl::fromLocalFile(dirPath));
        }
    } else {
        pokazKomunikat("Błąd",
                       "Wystąpił błąd podczas eksportowania danych.",
                       QMessageBox::Critical);
    }
}

// ==================== SLOTS DLA IMPORTU CSV ====================

void MainWindow::importKlienciCSV() {
    QString fileName = getCSVOpenFileName("Importuj klientów z CSV");
    if (fileName.isEmpty()) {
        return;
    }

    // Ostrzeżenie o możliwych duplikatach
    QMessageBox::StandardButton odpowiedz = QMessageBox::question(
        this,
        "Potwierdź import",
        "Import klientów z pliku CSV.\n\n"
        "Uwaga: Klienci z istniejącymi adresami email zostaną pominięci.\n\n"
        "Czy kontynuować?",
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::Yes
        );

    if (odpowiedz != QMessageBox::Yes) {
        return;
    }

    QProgressDialog progress("Importowanie klientów...", "Anuluj", 0, 0, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.show();
    QApplication::processEvents();

    QPair<int, QStringList> result = DatabaseManager::importKlienciFromCSV(fileName);
    progress.close();

    pokazRaportImportu("Import klientów", result.first, result.second);

    if (result.first > 0) {
        odswiezListeKlientow();
        zaladujKlientowDoComboBox();
        zaladujKlientowDoComboBoxKarnetu();
    }
}

void MainWindow::importZajeciaCSV() {
    QString fileName = getCSVOpenFileName("Importuj zajęcia z CSV");
    if (fileName.isEmpty()) {
        return;
    }

    QMessageBox::StandardButton odpowiedz = QMessageBox::question(
        this,
        "Potwierdź import",
        "Import zajęć z pliku CSV.\n\n"
        "Uwaga: Zajęcia z istniejącymi terminami zostaną pominięte.\n\n"
        "Czy kontynuować?",
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::Yes
        );

    if (odpowiedz != QMessageBox::Yes) {
        return;
    }

    QProgressDialog progress("Importowanie zajęć...", "Anuluj", 0, 0, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.show();
    QApplication::processEvents();

    QPair<int, QStringList> result = DatabaseManager::importZajeciaFromCSV(fileName);
    progress.close();

    pokazRaportImportu("Import zajęć", result.first, result.second);

    if (result.first > 0) {
        odswiezListeZajec();
        zaladujZajeciaDoComboBox();
    }
}

void MainWindow::importRezerwacjeCSV() {
    QString fileName = getCSVOpenFileName("Importuj rezerwacje z CSV");
    if (fileName.isEmpty()) {
        return;
    }

    QMessageBox::StandardButton odpowiedz = QMessageBox::question(
        this,
        "Potwierdź import",
        "Import rezerwacji z pliku CSV.\n\n"
        "Uwaga: Rezerwacje będą dodane tylko jeśli istnieją odpowiednie klienci i zajęcia.\n\n"
        "Czy kontynuować?",
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::Yes
        );

    if (odpowiedz != QMessageBox::Yes) {
        return;
    }

    QProgressDialog progress("Importowanie rezerwacji...", "Anuluj", 0, 0, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.show();
    QApplication::processEvents();

    QPair<int, QStringList> result = DatabaseManager::importRezerwacjeFromCSV(fileName);
    progress.close();

    pokazRaportImportu("Import rezerwacji", result.first, result.second);

    if (result.first > 0) {
        odswiezListeRezerwacji();
    }
}

void MainWindow::importKarnetyCSV() {
    QString fileName = getCSVOpenFileName("Importuj karnety z CSV");
    if (fileName.isEmpty()) {
        return;
    }

    QMessageBox::StandardButton odpowiedz = QMessageBox::question(
        this,
        "Potwierdź import",
        "Import karnetów z pliku CSV.\n\n"
        "Uwaga: Karnety będą dodane tylko jeśli istnieją odpowiedni klienci.\n"
        "Aktywne karnety tego samego typu dla klienta zostaną pominięte.\n\n"
        "Czy kontynuować?",
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::Yes
        );

    if (odpowiedz != QMessageBox::Yes) {
        return;
    }

    QProgressDialog progress("Importowanie karnetów...", "Anuluj", 0, 0, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.show();
    QApplication::processEvents();

    QPair<int, QStringList> result = DatabaseManager::importKarnetyFromCSV(fileName);
    progress.close();

    pokazRaportImportu("Import karnetów", result.first, result.second);

    if (result.first > 0) {
        odswiezListeKarnetow();
    }
}

// ==================== METODY POMOCNICZE CSV ====================

QString MainWindow::getCSVSaveFileName(const QString& defaultName, const QString& title) {
    QString documentsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    QString defaultPath = documentsPath + "/" + defaultName;

    return QFileDialog::getSaveFileName(
        this,
        title,
        defaultPath,
        "Pliki CSV (*.csv);;Wszystkie pliki (*.*)"
        );
}

QString MainWindow::getCSVOpenFileName(const QString& title) {
    QString documentsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);

    return QFileDialog::getOpenFileName(
        this,
        title,
        documentsPath,
        "Pliki CSV (*.csv);;Wszystkie pliki (*.*)"
        );
}

QString MainWindow::getDirectoryPath(const QString& title) {
    QString documentsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);

    return QFileDialog::getExistingDirectory(
        this,
        title,
        documentsPath,
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
        );
}

void MainWindow::pokazRaportImportu(const QString& tytul, int zaimportowane, const QStringList& bledy) {
    QString message;

    if (zaimportowane > 0) {
        message += QString("✅ Pomyślnie zaimportowano: %1 rekordów\n\n").arg(zaimportowane);
    } else {
        message += "❌ Nie zaimportowano żadnych rekordów\n\n";
    }

    if (!bledy.isEmpty()) {
        message += QString("⚠️ Błędy (%1):\n").arg(bledy.size());

        // Pokaż maksymalnie 10 pierwszych błędów
        int maxErrors = qMin(10, bledy.size());
        for (int i = 0; i < maxErrors; i++) {
            message += "• " + bledy[i] + "\n";
        }

        if (bledy.size() > maxErrors) {
            message += QString("... i %1 kolejnych błędów\n").arg(bledy.size() - maxErrors);
        }
    }

    QMessageBox msgBox(this);
    msgBox.setWindowTitle(tytul);
    msgBox.setText(message);

    if (zaimportowane > 0 && bledy.isEmpty()) {
        msgBox.setIcon(QMessageBox::Information);
    } else if (zaimportowane > 0) {
        msgBox.setIcon(QMessageBox::Warning);
    } else {
        msgBox.setIcon(QMessageBox::Critical);
    }

    // Dodaj przycisk do zapisania raportu błędów
    if (!bledy.isEmpty()) {
        QPushButton* saveButton = msgBox.addButton("Zapisz raport błędów", QMessageBox::ActionRole);
        msgBox.addButton(QMessageBox::Ok);

        msgBox.exec();

        if (msgBox.clickedButton() == saveButton) {
            QString fileName = getCSVSaveFileName("raport_bledow.txt", "Zapisz raport błędów");
            if (!fileName.isEmpty()) {
                QFile file(fileName);
                if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                    QTextStream stream(&file);
                    stream.setCodec("UTF-8");
                    stream << tytul << " - Raport błędów\n";
                    stream << QString("Data: %1\n\n").arg(QDateTime::currentDateTime().toString());
                    stream << QString("Zaimportowano: %1 rekordów\n").arg(zaimportowane);
                    stream << QString("Błędy: %1\n\n").arg(bledy.size());

                    for (const QString& blad : bledy) {
                        stream << blad << "\n";
                    }

                    file.close();
                    pokazKomunikat("Zapisano", "Raport błędów został zapisany do pliku:\n" + fileName);
                }
            }
        }
    } else {
        msgBox.exec();
    }
}
