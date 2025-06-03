#include "DatabaseManager.h"
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <QDebug>
#include <QDateTime>

QSqlDatabase DatabaseManager::db = QSqlDatabase();

// === Podstawowe metody połączenia ===

bool DatabaseManager::connect(const QString& path) {
    if (QSqlDatabase::contains("gym_connection")) {
        db = QSqlDatabase::database("gym_connection");
    } else {
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

// === CRUD dla KLIENTÓW ===

bool DatabaseManager::addKlient(const QString& imie,
                                const QString& nazwisko,
                                const QString& email,
                                const QString& telefon,
                                const QString& dataUrodzenia,
                                const QString& uwagi) {

    // Sprawdź czy email już istnieje (jeśli podano)
    if (!email.isEmpty() && emailExists(email)) {
        qWarning() << "Email już istnieje w bazie:" << email;
        return false;
    }

    QSqlQuery query(db);
    query.prepare(R"(
        INSERT INTO klient (imie, nazwisko, email, telefon, dataUrodzenia, dataRejestracji, uwagi)
        VALUES (:imie, :nazwisko, :email, :telefon, :dataUrodzenia, :dataRejestracji, :uwagi)
    )");

    query.bindValue(":imie", imie);
    query.bindValue(":nazwisko", nazwisko);
    query.bindValue(":email", email.isEmpty() ? QVariant() : email);
    query.bindValue(":telefon", telefon.isEmpty() ? QVariant() : telefon);
    query.bindValue(":dataUrodzenia", dataUrodzenia.isEmpty() ? QVariant() : dataUrodzenia);
    query.bindValue(":dataRejestracji", QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"));
    query.bindValue(":uwagi", uwagi.isEmpty() ? QVariant() : uwagi);

    if (!query.exec()) {
        qWarning() << "Błąd dodawania klienta:" << query.lastError().text();
        return false;
    }

    qDebug() << "Dodano klienta:" << imie << nazwisko;
    return true;
}

QList<Klient> DatabaseManager::getAllKlienci() {
    QList<Klient> klienci;

    QSqlQuery query(db);
    if (!query.exec("SELECT id, imie, nazwisko, email, telefon, dataUrodzenia, dataRejestracji, uwagi FROM klient ORDER BY nazwisko, imie")) {
        qWarning() << "Błąd pobierania klientów:" << query.lastError().text();
        return klienci;
    }

    while (query.next()) {
        klienci.append(queryToKlient(query));
    }

    return klienci;
}

Klient DatabaseManager::getKlientById(int id) {
    Klient klient = {}; // pusty klient

    QSqlQuery query(db);
    query.prepare("SELECT id, imie, nazwisko, email, telefon, dataUrodzenia, dataRejestracji, uwagi FROM klient WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        qWarning() << "Błąd pobierania klienta o ID" << id << ":" << query.lastError().text();
        return klient;
    }

    if (query.next()) {
        klient = queryToKlient(query);
    }

    return klient;
}

bool DatabaseManager::updateKlient(int id,
                                   const QString& imie,
                                   const QString& nazwisko,
                                   const QString& email,
                                   const QString& telefon,
                                   const QString& dataUrodzenia,
                                   const QString& uwagi) {

    // Sprawdź czy email już istnieje u innego klienta
    if (!email.isEmpty() && emailExists(email, id)) {
        qWarning() << "Email już istnieje u innego klienta:" << email;
        return false;
    }

    QSqlQuery query(db);
    query.prepare(R"(
        UPDATE klient
        SET imie = :imie, nazwisko = :nazwisko, email = :email,
            telefon = :telefon, dataUrodzenia = :dataUrodzenia, uwagi = :uwagi
        WHERE id = :id
    )");

    query.bindValue(":id", id);
    query.bindValue(":imie", imie);
    query.bindValue(":nazwisko", nazwisko);
    query.bindValue(":email", email.isEmpty() ? QVariant() : email);
    query.bindValue(":telefon", telefon.isEmpty() ? QVariant() : telefon);
    query.bindValue(":dataUrodzenia", dataUrodzenia.isEmpty() ? QVariant() : dataUrodzenia);
    query.bindValue(":uwagi", uwagi.isEmpty() ? QVariant() : uwagi);

    if (!query.exec()) {
        qWarning() << "Błąd aktualizacji klienta:" << query.lastError().text();
        return false;
    }

    if (query.numRowsAffected() == 0) {
        qWarning() << "Nie znaleziono klienta o ID:" << id;
        return false;
    }

    qDebug() << "Zaktualizowano klienta o ID:" << id;
    return true;
}

bool DatabaseManager::deleteKlient(int id) {
    QSqlQuery query(db);
    query.prepare("DELETE FROM klient WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        qWarning() << "Błąd usuwania klienta:" << query.lastError().text();
        return false;
    }

    if (query.numRowsAffected() == 0) {
        qWarning() << "Nie znaleziono klienta o ID:" << id;
        return false;
    }

    qDebug() << "Usunięto klienta o ID:" << id;
    return true;
}

bool DatabaseManager::emailExists(const QString& email, int excludeId) {
    if (email.isEmpty()) return false;

    QSqlQuery query(db);
    if (excludeId >= 0) {
        query.prepare("SELECT COUNT(*) FROM klient WHERE email = :email AND id != :excludeId");
        query.bindValue(":excludeId", excludeId);
    } else {
        query.prepare("SELECT COUNT(*) FROM klient WHERE email = :email");
    }
    query.bindValue(":email", email);

    if (!query.exec() || !query.next()) {
        qWarning() << "Błąd sprawdzania email:" << query.lastError().text();
        return false;
    }

    return query.value(0).toInt() > 0;
}

QList<Klient> DatabaseManager::searchKlienciByNazwisko(const QString& nazwisko) {
    QList<Klient> klienci;

    QSqlQuery query(db);
    query.prepare("SELECT id, imie, nazwisko, email, telefon, dataUrodzenia, dataRejestracji, uwagi FROM klient WHERE nazwisko LIKE :nazwisko ORDER BY nazwisko, imie");
    query.bindValue(":nazwisko", "%" + nazwisko + "%");

    if (!query.exec()) {
        qWarning() << "Błąd wyszukiwania klientów:" << query.lastError().text();
        return klienci;
    }

    while (query.next()) {
        klienci.append(queryToKlient(query));
    }

    return klienci;
}

int DatabaseManager::getKlienciCount() {
    QSqlQuery query(db);
    if (!query.exec("SELECT COUNT(*) FROM klient")) {
        qWarning() << "Błąd liczenia klientów:" << query.lastError().text();
        return 0;
    }

    if (query.next()) {
        return query.value(0).toInt();
    }

    return 0;
}

// === CRUD dla ZAJĘĆ ===

bool DatabaseManager::addZajecia(const QString& nazwa,
                                 const QString& trener,
                                 int maksUczestnikow,
                                 const QString& data,
                                 const QString& czas,
                                 int czasTrwania,
                                 const QString& opis) {

    // Sprawdź czy zajęcia o tej nazwie, dacie i czasie już istnieją
    if (!data.isEmpty() && !czas.isEmpty() && zajeciaExist(nazwa, data, czas)) {
        qWarning() << "Zajęcia o tej nazwie, dacie i czasie już istnieją:" << nazwa << data << czas;
        return false;
    }

    QSqlQuery query(db);
    query.prepare(R"(
        INSERT INTO zajecia (nazwa, trener, maksUczestnikow, data, czas, czasTrwania, opis)
        VALUES (:nazwa, :trener, :maksUczestnikow, :data, :czas, :czasTrwania, :opis)
    )");

    query.bindValue(":nazwa", nazwa);
    query.bindValue(":trener", trener.isEmpty() ? QVariant() : trener);
    query.bindValue(":maksUczestnikow", maksUczestnikow);
    query.bindValue(":data", data.isEmpty() ? QVariant() : data);
    query.bindValue(":czas", czas.isEmpty() ? QVariant() : czas);
    query.bindValue(":czasTrwania", czasTrwania);
    query.bindValue(":opis", opis.isEmpty() ? QVariant() : opis);

    if (!query.exec()) {
        qWarning() << "Błąd dodawania zajęć:" << query.lastError().text();
        return false;
    }

    qDebug() << "Dodano zajęcia:" << nazwa << "(" << trener << ")";
    return true;
}

QList<Zajecia> DatabaseManager::getAllZajecia() {
    QList<Zajecia> zajecia;

    QSqlQuery query(db);
    if (!query.exec("SELECT id, nazwa, trener, maksUczestnikow, data, czas, czasTrwania, opis FROM zajecia ORDER BY data, czas, nazwa")) {
        qWarning() << "Błąd pobierania zajęć:" << query.lastError().text();
        return zajecia;
    }

    while (query.next()) {
        zajecia.append(queryToZajecia(query));
    }

    return zajecia;
}

Zajecia DatabaseManager::getZajeciaById(int id) {
    Zajecia zajecia = {}; // puste zajęcia

    QSqlQuery query(db);
    query.prepare("SELECT id, nazwa, trener, maksUczestnikow, data, czas, czasTrwania, opis FROM zajecia WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        qWarning() << "Błąd pobierania zajęć o ID" << id << ":" << query.lastError().text();
        return zajecia;
    }

    if (query.next()) {
        zajecia = queryToZajecia(query);
    }

    return zajecia;
}

bool DatabaseManager::updateZajecia(int id,
                                    const QString& nazwa,
                                    const QString& trener,
                                    int maksUczestnikow,
                                    const QString& data,
                                    const QString& czas,
                                    int czasTrwania,
                                    const QString& opis) {

    // Sprawdź czy zajęcia o tej nazwie, dacie i czasie już istnieją (z wykluczeniem aktualnych)
    if (!data.isEmpty() && !czas.isEmpty() && zajeciaExist(nazwa, data, czas, id)) {
        qWarning() << "Zajęcia o tej nazwie, dacie i czasie już istnieją:" << nazwa << data << czas;
        return false;
    }

    QSqlQuery query(db);
    query.prepare(R"(
        UPDATE zajecia
        SET nazwa = :nazwa, trener = :trener, maksUczestnikow = :maksUczestnikow,
            data = :data, czas = :czas, czasTrwania = :czasTrwania, opis = :opis
        WHERE id = :id
    )");

    query.bindValue(":id", id);
    query.bindValue(":nazwa", nazwa);
    query.bindValue(":trener", trener.isEmpty() ? QVariant() : trener);
    query.bindValue(":maksUczestnikow", maksUczestnikow);
    query.bindValue(":data", data.isEmpty() ? QVariant() : data);
    query.bindValue(":czas", czas.isEmpty() ? QVariant() : czas);
    query.bindValue(":czasTrwania", czasTrwania);
    query.bindValue(":opis", opis.isEmpty() ? QVariant() : opis);

    if (!query.exec()) {
        qWarning() << "Błąd aktualizacji zajęć:" << query.lastError().text();
        return false;
    }

    if (query.numRowsAffected() == 0) {
        qWarning() << "Nie znaleziono zajęć o ID:" << id;
        return false;
    }

    qDebug() << "Zaktualizowano zajęcia o ID:" << id;
    return true;
}

bool DatabaseManager::deleteZajecia(int id) {
    QSqlQuery query(db);
    query.prepare("DELETE FROM zajecia WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        qWarning() << "Błąd usuwania zajęć:" << query.lastError().text();
        return false;
    }

    if (query.numRowsAffected() == 0) {
        qWarning() << "Nie znaleziono zajęć o ID:" << id;
        return false;
    }

    qDebug() << "Usunięto zajęcia o ID:" << id;
    return true;
}

// === Pomocnicze metody dla zajęć ===

QList<Zajecia> DatabaseManager::searchZajeciaByNazwa(const QString& nazwa) {
    QList<Zajecia> zajecia;

    QSqlQuery query(db);
    query.prepare("SELECT id, nazwa, trener, maksUczestnikow, data, czas, czasTrwania, opis FROM zajecia WHERE nazwa LIKE :nazwa ORDER BY data, czas, nazwa");
    query.bindValue(":nazwa", "%" + nazwa + "%");

    if (!query.exec()) {
        qWarning() << "Błąd wyszukiwania zajęć po nazwie:" << query.lastError().text();
        return zajecia;
    }

    while (query.next()) {
        zajecia.append(queryToZajecia(query));
    }

    return zajecia;
}

QList<Zajecia> DatabaseManager::searchZajeciaByTrener(const QString& trener) {
    QList<Zajecia> zajecia;

    QSqlQuery query(db);
    query.prepare("SELECT id, nazwa, trener, maksUczestnikow, data, czas, czasTrwania, opis FROM zajecia WHERE trener LIKE :trener ORDER BY data, czas, nazwa");
    query.bindValue(":trener", "%" + trener + "%");

    if (!query.exec()) {
        qWarning() << "Błąd wyszukiwania zajęć po trenerze:" << query.lastError().text();
        return zajecia;
    }

    while (query.next()) {
        zajecia.append(queryToZajecia(query));
    }

    return zajecia;
}

QList<Zajecia> DatabaseManager::getZajeciaByData(const QString& data) {
    QList<Zajecia> zajecia;

    QSqlQuery query(db);
    query.prepare("SELECT id, nazwa, trener, maksUczestnikow, data, czas, czasTrwania, opis FROM zajecia WHERE data = :data ORDER BY czas, nazwa");
    query.bindValue(":data", data);

    if (!query.exec()) {
        qWarning() << "Błąd pobierania zajęć z dnia" << data << ":" << query.lastError().text();
        return zajecia;
    }

    while (query.next()) {
        zajecia.append(queryToZajecia(query));
    }

    return zajecia;
}

int DatabaseManager::getZajeciaCount() {
    QSqlQuery query(db);
    if (!query.exec("SELECT COUNT(*) FROM zajecia")) {
        qWarning() << "Błąd liczenia zajęć:" << query.lastError().text();
        return 0;
    }

    if (query.next()) {
        return query.value(0).toInt();
    }

    return 0;
}

bool DatabaseManager::zajeciaExist(const QString& nazwa, const QString& data, const QString& czas, int excludeId) {
    if (nazwa.isEmpty() || data.isEmpty() || czas.isEmpty()) return false;

    QSqlQuery query(db);
    if (excludeId >= 0) {
        query.prepare("SELECT COUNT(*) FROM zajecia WHERE nazwa = :nazwa AND data = :data AND czas = :czas AND id != :excludeId");
        query.bindValue(":excludeId", excludeId);
    } else {
        query.prepare("SELECT COUNT(*) FROM zajecia WHERE nazwa = :nazwa AND data = :data AND czas = :czas");
    }
    query.bindValue(":nazwa", nazwa);
    query.bindValue(":data", data);
    query.bindValue(":czas", czas);

    if (!query.exec() || !query.next()) {
        qWarning() << "Błąd sprawdzania istnienia zajęć:" << query.lastError().text();
        return false;
    }

    return query.value(0).toInt() > 0;
}

// === Metody pomocnicze ===

Klient DatabaseManager::queryToKlient(QSqlQuery& query) {
    Klient klient;
    klient.id = query.value("id").toInt();
    klient.imie = query.value("imie").toString();
    klient.nazwisko = query.value("nazwisko").toString();
    klient.email = query.value("email").toString();
    klient.telefon = query.value("telefon").toString();
    klient.dataUrodzenia = query.value("dataUrodzenia").toString();
    klient.dataRejestracji = query.value("dataRejestracji").toString();
    klient.uwagi = query.value("uwagi").toString();
    return klient;
}

Zajecia DatabaseManager::queryToZajecia(QSqlQuery& query) {
    Zajecia zajecia;
    zajecia.id = query.value("id").toInt();
    zajecia.nazwa = query.value("nazwa").toString();
    zajecia.trener = query.value("trener").toString();
    zajecia.maksUczestnikow = query.value("maksUczestnikow").toInt();
    zajecia.data = query.value("data").toString();
    zajecia.czas = query.value("czas").toString();
    zajecia.czasTrwania = query.value("czasTrwania").toInt();
    zajecia.opis = query.value("opis").toString();
    return zajecia;
}
