// Dodaj te include na górze pliku DatabaseManager.cpp (po istniejących)
#include <QTextStream>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>

// Dodaj te funkcje na końcu pliku DatabaseManager.cpp (przed końcowym })

// === FUNKCJE EKSPORTU CSV ===

bool DatabaseManager::exportKlienciToCSV(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Nie można otworzyć pliku do zapisu:" << filePath;
        return false;
    }

    QTextStream stream(&file);
    stream.setCodec("UTF-8");

    // Nagłówek CSV
    QStringList headers = {"ID", "Imie", "Nazwisko", "Email", "Telefon", "DataUrodzenia", "DataRejestracji", "Uwagi"};
    stream << formatCSVHeader(headers) << "\n";

    // Dane
    QList<Klient> klienci = getAllKlienci();
    for (const Klient& k : klienci) {
        QStringList row;
        row << QString::number(k.id)
            << k.imie
            << k.nazwisko
            << k.email
            << k.telefon
            << k.dataUrodzenia
            << k.dataRejestracji
            << k.uwagi;

        stream << formatCSVRow(row) << "\n";
    }

    file.close();
    qDebug() << "Wyeksportowano" << klienci.size() << "klientów do:" << filePath;
    return true;
}

bool DatabaseManager::exportZajeciaToCSV(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Nie można otworzyć pliku do zapisu:" << filePath;
        return false;
    }

    QTextStream stream(&file);
    stream.setCodec("UTF-8");

    // Nagłówek CSV
    QStringList headers = {"ID", "Nazwa", "Trener", "MaksUczestnikow", "Data", "Czas", "CzasTrwania", "Opis"};
    stream << formatCSVHeader(headers) << "\n";

    // Dane
    QList<Zajecia> zajecia = getAllZajecia();
    for (const Zajecia& z : zajecia) {
        QStringList row;
        row << QString::number(z.id)
            << z.nazwa
            << z.trener
            << QString::number(z.maksUczestnikow)
            << z.data
            << z.czas
            << QString::number(z.czasTrwania)
            << z.opis;

        stream << formatCSVRow(row) << "\n";
    }

    file.close();
    qDebug() << "Wyeksportowano" << zajecia.size() << "zajęć do:" << filePath;
    return true;
}

bool DatabaseManager::exportRezerwacjeToCSV(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Nie można otworzyć pliku do zapisu:" << filePath;
        return false;
    }

    QTextStream stream(&file);
    stream.setCodec("UTF-8");

    // Nagłówek CSV
    QStringList headers = {"ID", "IdKlienta", "IdZajec", "ImieKlienta", "NazwiskoKlienta",
                           "NazwaZajec", "TrenerZajec", "DataZajec", "CzasZajec",
                           "DataRezerwacji", "Status"};
    stream << formatCSVHeader(headers) << "\n";

    // Dane
    QList<Rezerwacja> rezerwacje = getAllRezerwacje();
    for (const Rezerwacja& r : rezerwacje) {
        QStringList row;
        row << QString::number(r.id)
            << QString::number(r.idKlienta)
            << QString::number(r.idZajec)
            << r.imieKlienta
            << r.nazwiskoKlienta
            << r.nazwaZajec
            << r.trenerZajec
            << r.dataZajec
            << r.czasZajec
            << r.dataRezerwacji
            << r.status;

        stream << formatCSVRow(row) << "\n";
    }

    file.close();
    qDebug() << "Wyeksportowano" << rezerwacje.size() << "rezerwacji do:" << filePath;
    return true;
}

bool DatabaseManager::exportKarnetyToCSV(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Nie można otworzyć pliku do zapisu:" << filePath;
        return false;
    }

    QTextStream stream(&file);
    stream.setCodec("UTF-8");

    // Nagłówek CSV
    QStringList headers = {"ID", "IdKlienta", "ImieKlienta", "NazwiskoKlienta", "EmailKlienta",
                           "Typ", "DataRozpoczecia", "DataZakonczenia", "Cena", "CzyAktywny"};
    stream << formatCSVHeader(headers) << "\n";

    // Dane
    QList<Karnet> karnety = getAllKarnety();
    for (const Karnet& k : karnety) {
        QStringList row;
        row << QString::number(k.id)
            << QString::number(k.idKlienta)
            << k.imieKlienta
            << k.nazwiskoKlienta
            << k.emailKlienta
            << k.typ
            << k.dataRozpoczecia
            << k.dataZakonczenia
            << QString::number(k.cena, 'f', 2)
            << (k.czyAktywny ? "1" : "0");

        stream << formatCSVRow(row) << "\n";
    }

    file.close();
    qDebug() << "Wyeksportowano" << karnety.size() << "karnetów do:" << filePath;
    return true;
}

bool DatabaseManager::exportAllToCSV(const QString& dirPath) {
    QDir dir(dirPath);
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            qWarning() << "Nie można utworzyć katalogu:" << dirPath;
            return false;
        }
    }

    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd_HH-mm-ss");

    bool success = true;
    success &= exportKlienciToCSV(dirPath + "/klienci_" + timestamp + ".csv");
    success &= exportZajeciaToCSV(dirPath + "/zajecia_" + timestamp + ".csv");
    success &= exportRezerwacjeToCSV(dirPath + "/rezerwacje_" + timestamp + ".csv");
    success &= exportKarnetyToCSV(dirPath + "/karnety_" + timestamp + ".csv");

    if (success) {
        qDebug() << "Pomyślnie wyeksportowano wszystkie dane do:" << dirPath;
    }

    return success;
}

// === FUNKCJE IMPORTU CSV ===

QPair<int, QStringList> DatabaseManager::importKlienciFromCSV(const QString& filePath) {
    QFile file(filePath);
    QStringList errors;
    int importedCount = 0;

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        errors << "Nie można otworzyć pliku: " + filePath;
        return qMakePair(0, errors);
    }

    QTextStream stream(&file);
    stream.setCodec("UTF-8");

    // Pomiń nagłówek
    if (!stream.atEnd()) {
        stream.readLine();
    }

    int lineNumber = 1;
    while (!stream.atEnd()) {
        lineNumber++;
        QString line = stream.readLine().trimmed();
        if (line.isEmpty()) continue;

        QStringList fields = parseCSVLine(line);
        if (fields.size() < 7) {  // Minimalna liczba pól (bez uwag)
            errors << QString("Linia %1: Za mało pól (%2, oczekiwano minimum 7)").arg(lineNumber).arg(fields.size());
            continue;
        }

        QString errorMsg;
        if (!validateKlientCSVRow(fields, errorMsg)) {
            errors << QString("Linia %1: %2").arg(lineNumber).arg(errorMsg);
            continue;
        }

        // Pola: ID, Imie, Nazwisko, Email, Telefon, DataUrodzenia, DataRejestracji, Uwagi
        QString imie = fields[1].trimmed();
        QString nazwisko = fields[2].trimmed();
        QString email = fields[3].trimmed();
        QString telefon = fields[4].trimmed();
        QString dataUrodzenia = fields[5].trimmed();
        QString uwagi = fields.size() > 7 ? fields[7].trimmed() : "";

        // Sprawdź czy email już istnieje (jeśli nie jest pusty)
        if (!email.isEmpty() && emailExists(email)) {
            errors << QString("Linia %1: Email %2 już istnieje w bazie").arg(lineNumber).arg(email);
            continue;
        }

        if (addKlient(imie, nazwisko, email, telefon, dataUrodzenia, uwagi)) {
            importedCount++;
        } else {
            errors << QString("Linia %1: Błąd dodawania klienta do bazy").arg(lineNumber);
        }
    }

    file.close();
    qDebug() << "Zaimportowano" << importedCount << "klientów z" << (lineNumber - 1) << "wierszy";
    return qMakePair(importedCount, errors);
}

QPair<int, QStringList> DatabaseManager::importZajeciaFromCSV(const QString& filePath) {
    QFile file(filePath);
    QStringList errors;
    int importedCount = 0;

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        errors << "Nie można otworzyć pliku: " + filePath;
        return qMakePair(0, errors);
    }

    QTextStream stream(&file);
    stream.setCodec("UTF-8");

    // Pomiń nagłówek
    if (!stream.atEnd()) {
        stream.readLine();
    }

    int lineNumber = 1;
    while (!stream.atEnd()) {
        lineNumber++;
        QString line = stream.readLine().trimmed();
        if (line.isEmpty()) continue;

        QStringList fields = parseCSVLine(line);
        if (fields.size() < 7) {  // Minimalna liczba pól (bez opisu)
            errors << QString("Linia %1: Za mało pól (%2, oczekiwano minimum 7)").arg(lineNumber).arg(fields.size());
            continue;
        }

        QString errorMsg;
        if (!validateZajeciaCSVRow(fields, errorMsg)) {
            errors << QString("Linia %1: %2").arg(lineNumber).arg(errorMsg);
            continue;
        }

        // Pola: ID, Nazwa, Trener, MaksUczestnikow, Data, Czas, CzasTrwania, Opis
        QString nazwa = fields[1].trimmed();
        QString trener = fields[2].trimmed();
        int maksUczestnikow = fields[3].toInt();
        QString data = fields[4].trimmed();
        QString czas = fields[5].trimmed();
        int czasTrwania = fields[6].toInt();
        QString opis = fields.size() > 7 ? fields[7].trimmed() : "";

        // Sprawdź czy zajęcia już istnieją
        if (!data.isEmpty() && !czas.isEmpty() && zajeciaExist(nazwa, data, czas)) {
            errors << QString("Linia %1: Zajęcia '%2' już istnieją w tym terminie").arg(lineNumber).arg(nazwa);
            continue;
        }

        if (addZajecia(nazwa, trener, maksUczestnikow, data, czas, czasTrwania, opis)) {
            importedCount++;
        } else {
            errors << QString("Linia %1: Błąd dodawania zajęć do bazy").arg(lineNumber);
        }
    }

    file.close();
    qDebug() << "Zaimportowano" << importedCount << "zajęć z" << (lineNumber - 1) << "wierszy";
    return qMakePair(importedCount, errors);
}

QPair<int, QStringList> DatabaseManager::importRezerwacjeFromCSV(const QString& filePath) {
    QFile file(filePath);
    QStringList errors;
    int importedCount = 0;

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        errors << "Nie można otworzyć pliku: " + filePath;
        return qMakePair(0, errors);
    }

    QTextStream stream(&file);
    stream.setCodec("UTF-8");

    // Pomiń nagłówek
    if (!stream.atEnd()) {
        stream.readLine();
    }

    int lineNumber = 1;
    while (!stream.atEnd()) {
        lineNumber++;
        QString line = stream.readLine().trimmed();
        if (line.isEmpty()) continue;

        QStringList fields = parseCSVLine(line);
        if (fields.size() < 11) {
            errors << QString("Linia %1: Za mało pól (%2, oczekiwano 11)").arg(lineNumber).arg(fields.size());
            continue;
        }

        QString errorMsg;
        if (!validateRezerwacjaCSVRow(fields, errorMsg)) {
            errors << QString("Linia %1: %2").arg(lineNumber).arg(errorMsg);
            continue;
        }

        // Pola: ID, IdKlienta, IdZajec, ImieKlienta, NazwiskoKlienta, NazwaZajec, TrenerZajec, DataZajec, CzasZajec, DataRezerwacji, Status
        int idKlienta = fields[1].toInt();
        int idZajec = fields[2].toInt();
        QString status = fields[10].trimmed();

        // Sprawdź czy klient i zajęcia istnieją
        Klient klient = getKlientById(idKlienta);
        if (klient.id <= 0) {
            errors << QString("Linia %1: Nie znaleziono klienta o ID %2").arg(lineNumber).arg(idKlienta);
            continue;
        }

        Zajecia zajecia = getZajeciaById(idZajec);
        if (zajecia.id <= 0) {
            errors << QString("Linia %1: Nie znaleziono zajęć o ID %2").arg(lineNumber).arg(idZajec);
            continue;
        }

        // Sprawdź czy klient już ma rezerwację na te zajęcia (tylko dla aktywnych)
        if (status == "aktywna" && klientMaRezerwacje(idKlienta, idZajec)) {
            errors << QString("Linia %1: Klient już ma aktywną rezerwację na te zajęcia").arg(lineNumber);
            continue;
        }

        if (addRezerwacja(idKlienta, idZajec, status)) {
            importedCount++;
        } else {
            errors << QString("Linia %1: Błąd dodawania rezerwacji do bazy").arg(lineNumber);
        }
    }

    file.close();
    qDebug() << "Zaimportowano" << importedCount << "rezerwacji z" << (lineNumber - 1) << "wierszy";
    return qMakePair(importedCount, errors);
}

QPair<int, QStringList> DatabaseManager::importKarnetyFromCSV(const QString& filePath) {
    QFile file(filePath);
    QStringList errors;
    int importedCount = 0;

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        errors << "Nie można otworzyć pliku: " + filePath;
        return qMakePair(0, errors);
    }

    QTextStream stream(&file);
    stream.setCodec("UTF-8");

    // Pomiń nagłówek
    if (!stream.atEnd()) {
        stream.readLine();
    }

    int lineNumber = 1;
    while (!stream.atEnd()) {
        lineNumber++;
        QString line = stream.readLine().trimmed();
        if (line.isEmpty()) continue;

        QStringList fields = parseCSVLine(line);
        if (fields.size() < 10) {
            errors << QString("Linia %1: Za mało pól (%2, oczekiwano 10)").arg(lineNumber).arg(fields.size());
            continue;
        }

        QString errorMsg;
        if (!validateKarnetCSVRow(fields, errorMsg)) {
            errors << QString("Linia %1: %2").arg(lineNumber).arg(errorMsg);
            continue;
        }

        // Pola: ID, IdKlienta, ImieKlienta, NazwiskoKlienta, EmailKlienta, Typ, DataRozpoczecia, DataZakonczenia, Cena, CzyAktywny
        int idKlienta = fields[1].toInt();
        QString typ = fields[5].trimmed();
        QString dataRozpoczecia = fields[6].trimmed();
        QString dataZakonczenia = fields[7].trimmed();
        double cena = fields[8].toDouble();
        bool czyAktywny = (fields[9].trimmed() == "1");

        // Sprawdź czy klient istnieje
        Klient klient = getKlientById(idKlienta);
        if (klient.id <= 0) {
            errors << QString("Linia %1: Nie znaleziono klienta o ID %2").arg(lineNumber).arg(idKlienta);
            continue;
        }

        // Sprawdź czy można utworzyć karnet (tylko dla aktywnych)
        if (czyAktywny && !moznaUtworzycKarnet(idKlienta, typ)) {
            errors << QString("Linia %1: Klient już ma aktywny karnet typu '%2'").arg(lineNumber).arg(typ);
            continue;
        }

        if (addKarnet(idKlienta, typ, dataRozpoczecia, dataZakonczenia, cena, czyAktywny)) {
            importedCount++;
        } else {
            errors << QString("Linia %1: Błąd dodawania karnetu do bazy").arg(lineNumber);
        }
    }

    file.close();
    qDebug() << "Zaimportowano" << importedCount << "karnetów z" << (lineNumber - 1) << "wierszy";
    return qMakePair(importedCount, errors);
}

// === FUNKCJE POMOCNICZE CSV ===

QString DatabaseManager::escapeCSVField(const QString& field) {
    QString escaped = field;

    // Jeśli pole zawiera przecinek, cudzysłów lub znak nowej linii, owiń w cudzysłowy
    if (escaped.contains(',') || escaped.contains('"') || escaped.contains('\n') || escaped.contains('\r')) {
        // Podwój cudzysłowy wewnętrzne
        escaped.replace('"', "\"\"");
        // Owiń w cudzysłowy
        escaped = "\"" + escaped + "\"";
    }

    return escaped;
}

QStringList DatabaseManager::parseCSVLine(const QString& line) {
    QStringList result;
    QString currentField;
    bool inQuotes = false;
    bool quoteNext = false;

    for (int i = 0; i < line.length(); i++) {
        QChar c = line[i];

        if (quoteNext) {
            currentField += c;
            quoteNext = false;
        } else if (c == '"') {
            if (inQuotes) {
                if (i + 1 < line.length() && line[i + 1] == '"') {
                    // Podwójny cudzysłów - dodaj jeden
                    currentField += '"';
                    quoteNext = true;
                } else {
                    // Koniec pola w cudzysłowach
                    inQuotes = false;
                }
            } else {
                // Początek pola w cudzysłowach
                inQuotes = true;
            }
        } else if (c == ',' && !inQuotes) {
            // Separator pól
            result << currentField;
            currentField.clear();
        } else {
            currentField += c;
        }
    }

    // Dodaj ostatnie pole
    result << currentField;

    return result;
}

QString DatabaseManager::formatCSVHeader(const QStringList& headers) {
    QStringList escapedHeaders;
    for (const QString& header : headers) {
        escapedHeaders << escapeCSVField(header);
    }
    return escapedHeaders.join(",");
}

QString DatabaseManager::formatCSVRow(const QStringList& values) {
    QStringList escapedValues;
    for (const QString& value : values) {
        escapedValues << escapeCSVField(value);
    }
    return escapedValues.join(",");
}

// === FUNKCJE WALIDACJI CSV ===

bool DatabaseManager::validateKlientCSVRow(const QStringList& row, QString& errorMsg) {
    if (row.size() < 7) {
        errorMsg = "Za mało kolumn";
        return false;
    }

    // Walidacja imienia
    if (row[1].trimmed().isEmpty()) {
        errorMsg = "Imię nie może być puste";
        return false;
    }

    // Walidacja nazwiska
    if (row[2].trimmed().isEmpty()) {
        errorMsg = "Nazwisko nie może być puste";
        return false;
    }

    // Walidacja emaila (jeśli nie jest pusty)
    QString email = row[3].trimmed();
    if (!email.isEmpty() && !email.contains("@")) {
        errorMsg = "Nieprawidłowy format emaila";
        return false;
    }

    // Walidacja daty urodzenia (jeśli nie jest pusta)
    QString dataUrodzenia = row[5].trimmed();
    if (!dataUrodzenia.isEmpty()) {
        QDate data = QDate::fromString(dataUrodzenia, "yyyy-MM-dd");
        if (!data.isValid()) {
            errorMsg = "Nieprawidłowy format daty urodzenia (oczekiwano yyyy-MM-dd)";
            return false;
        }
    }

    return true;
}

bool DatabaseManager::validateZajeciaCSVRow(const QStringList& row, QString& errorMsg) {
    if (row.size() < 7) {
        errorMsg = "Za mało kolumn";
        return false;
    }

    // Walidacja nazwy
    if (row[1].trimmed().isEmpty()) {
        errorMsg = "Nazwa zajęć nie może być pusta";
        return false;
    }

    // Walidacja maksymalnej liczby uczestników
    bool ok;
    int maksUczestnikow = row[3].toInt(&ok);
    if (!ok || maksUczestnikow <= 0) {
        errorMsg = "Nieprawidłowa maksymalna liczba uczestników";
        return false;
    }

    // Walidacja daty (jeśli nie jest pusta)
    QString data = row[4].trimmed();
    if (!data.isEmpty()) {
        QDate dataObj = QDate::fromString(data, "yyyy-MM-dd");
        if (!dataObj.isValid()) {
            errorMsg = "Nieprawidłowy format daty (oczekiwano yyyy-MM-dd)";
            return false;
        }
    }

    // Walidacja czasu (jeśli nie jest pusty)
    QString czas = row[5].trimmed();
    if (!czas.isEmpty()) {
        QTime czasObj = QTime::fromString(czas, "HH:mm");
        if (!czasObj.isValid()) {
            errorMsg = "Nieprawidłowy format czasu (oczekiwano HH:mm)";
            return false;
        }
    }

    // Walidacja czasu trwania
    int czasTrwania = row[6].toInt(&ok);
    if (!ok || czasTrwania <= 0) {
        errorMsg = "Nieprawidłowy czas trwania";
        return false;
    }

    return true;
}

bool DatabaseManager::validateRezerwacjaCSVRow(const QStringList& row, QString& errorMsg) {
    if (row.size() < 11) {
        errorMsg = "Za mało kolumn";
        return false;
    }

    // Walidacja ID klienta
    bool ok;
    int idKlienta = row[1].toInt(&ok);
    if (!ok || idKlienta <= 0) {
        errorMsg = "Nieprawidłowe ID klienta";
        return false;
    }

    // Walidacja ID zajęć
    int idZajec = row[2].toInt(&ok);
    if (!ok || idZajec <= 0) {
        errorMsg = "Nieprawidłowe ID zajęć";
        return false;
    }

    // Walidacja statusu
    QString status = row[10].trimmed().toLower();
    if (status != "aktywna" && status != "anulowana") {
        errorMsg = "Nieprawidłowy status (oczekiwano 'aktywna' lub 'anulowana')";
        return false;
    }

    return true;
}

bool DatabaseManager::validateKarnetCSVRow(const QStringList& row, QString& errorMsg) {
    if (row.size() < 10) {
        errorMsg = "Za mało kolumn";
        return false;
    }

    // Walidacja ID klienta
    bool ok;
    int idKlienta = row[1].toInt(&ok);
    if (!ok || idKlienta <= 0) {
        errorMsg = "Nieprawidłowe ID klienta";
        return false;
    }

    // Walidacja typu
    QString typ = row[5].trimmed().toLower();
    if (typ != "normalny" && typ != "studencki") {
        errorMsg = "Nieprawidłowy typ karnetu (oczekiwano 'normalny' lub 'studencki')";
        return false;
    }

    // Walidacja daty rozpoczęcia
    QString dataRozp = row[6].trimmed();
    QDate dataRozpObj = QDate::fromString(dataRozp, "yyyy-MM-dd");
    if (!dataRozpObj.isValid()) {
        errorMsg = "Nieprawidłowy format daty rozpoczęcia (oczekiwano yyyy-MM-dd)";
        return false;
    }

    // Walidacja daty zakończenia
    QString dataZak = row[7].trimmed();
    QDate dataZakObj = QDate::fromString(dataZak, "yyyy-MM-dd");
    if (!dataZakObj.isValid()) {
        errorMsg = "Nieprawidłowy format daty zakończenia (oczekiwano yyyy-MM-dd)";
        return false;
    }

    // Sprawdź czy data zakończenia jest późniejsza
    if (dataZakObj <= dataRozpObj) {
        errorMsg = "Data zakończenia musi być późniejsza niż data rozpoczęcia";
        return false;
    }

    // Walidacja ceny
    double cena = row[8].toDouble(&ok);
    if (!ok || cena <= 0) {
        errorMsg = "Nieprawidłowa cena";
        return false;
    }

    // Walidacja statusu aktywności
    QString aktywny = row[9].trimmed();
    if (aktywny != "0" && aktywny != "1") {
        errorMsg = "Nieprawidłowy status aktywności (oczekiwano '0' lub '1')";
        return false;
    }

    return true;
}
