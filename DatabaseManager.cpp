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
    Klient klient = {};

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
    Zajecia zajecia = {};

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

// === CRUD dla REZERWACJI ===

bool DatabaseManager::addRezerwacja(int idKlienta, int idZajec, const QString& status) {
    // Sprawdź czy klient już ma rezerwację na te zajęcia
    if (klientMaRezerwacje(idKlienta, idZajec)) {
        qWarning() << "Klient już ma rezerwację na te zajęcia. Klient ID:" << idKlienta << "Zajęcia ID:" << idZajec;
        return false;
    }

    // Sprawdź czy można zarezerwować (czy nie przekroczono limitu)
    if (!moznaZarezerwowac(idZajec)) {
        qWarning() << "Przekroczono limit uczestników dla zajęć ID:" << idZajec;
        return false;
    }

    QSqlQuery query(db);
    query.prepare(R"(
        INSERT INTO rezerwacja (idKlienta, idZajec, dataRezerwacji, status)
        VALUES (:idKlienta, :idZajec, :dataRezerwacji, :status)
    )");

    query.bindValue(":idKlienta", idKlienta);
    query.bindValue(":idZajec", idZajec);
    query.bindValue(":dataRezerwacji", QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"));
    query.bindValue(":status", status);

    if (!query.exec()) {
        qWarning() << "Błąd dodawania rezerwacji:" << query.lastError().text();
        return false;
    }

    qDebug() << "Dodano rezerwację: klient" << idKlienta << "na zajęcia" << idZajec;
    return true;
}

QList<Rezerwacja> DatabaseManager::getAllRezerwacje() {
    QList<Rezerwacja> rezerwacje;

    QSqlQuery query(db);
    if (!query.exec(R"(
        SELECT r.id, r.idKlienta, r.idZajec, r.dataRezerwacji, r.status,
               k.imie, k.nazwisko,
               z.nazwa, z.trener, z.data, z.czas
        FROM rezerwacja r
        JOIN klient k ON r.idKlienta = k.id
        JOIN zajecia z ON r.idZajec = z.id
        ORDER BY r.dataRezerwacji DESC
    )")) {
        qWarning() << "Błąd pobierania rezerwacji:" << query.lastError().text();
        return rezerwacje;
    }

    while (query.next()) {
        rezerwacje.append(queryToRezerwacja(query));
    }

    return rezerwacje;
}

Rezerwacja DatabaseManager::getRezerwacjaById(int id) {
    Rezerwacja rezerwacja = {};

    QSqlQuery query(db);
    query.prepare(R"(
        SELECT r.id, r.idKlienta, r.idZajec, r.dataRezerwacji, r.status,
               k.imie, k.nazwisko,
               z.nazwa, z.trener, z.data, z.czas
        FROM rezerwacja r
        JOIN klient k ON r.idKlienta = k.id
        JOIN zajecia z ON r.idZajec = z.id
        WHERE r.id = :id
    )");
    query.bindValue(":id", id);

    if (!query.exec()) {
        qWarning() << "Błąd pobierania rezerwacji o ID" << id << ":" << query.lastError().text();
        return rezerwacja;
    }

    if (query.next()) {
        rezerwacja = queryToRezerwacja(query);
    }

    return rezerwacja;
}

bool DatabaseManager::updateRezerwacjaStatus(int id, const QString& status) {
    QSqlQuery query(db);
    query.prepare("UPDATE rezerwacja SET status = :status WHERE id = :id");
    query.bindValue(":id", id);
    query.bindValue(":status", status);

    if (!query.exec()) {
        qWarning() << "Błąd aktualizacji statusu rezerwacji:" << query.lastError().text();
        return false;
    }

    if (query.numRowsAffected() == 0) {
        qWarning() << "Nie znaleziono rezerwacji o ID:" << id;
        return false;
    }

    qDebug() << "Zaktualizowano status rezerwacji o ID:" << id << "na:" << status;
    return true;
}

bool DatabaseManager::deleteRezerwacja(int id) {
    QSqlQuery query(db);
    query.prepare("DELETE FROM rezerwacja WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        qWarning() << "Błąd usuwania rezerwacji:" << query.lastError().text();
        return false;
    }

    if (query.numRowsAffected() == 0) {
        qWarning() << "Nie znaleziono rezerwacji o ID:" << id;
        return false;
    }

    qDebug() << "Usunięto rezerwację o ID:" << id;
    return true;
}

// === Pomocnicze metody dla rezerwacji ===

bool DatabaseManager::klientMaRezerwacje(int idKlienta, int idZajec) {
    QSqlQuery query(db);
    query.prepare("SELECT COUNT(*) FROM rezerwacja WHERE idKlienta = :idKlienta AND idZajec = :idZajec AND status = 'aktywna'");
    query.bindValue(":idKlienta", idKlienta);
    query.bindValue(":idZajec", idZajec);

    if (!query.exec() || !query.next()) {
        qWarning() << "Błąd sprawdzania rezerwacji klienta:" << query.lastError().text();
        return false;
    }

    return query.value(0).toInt() > 0;
}

int DatabaseManager::getIloscAktywnychRezerwacji(int idZajec) {
    QSqlQuery query(db);
    query.prepare("SELECT COUNT(*) FROM rezerwacja WHERE idZajec = :idZajec AND status = 'aktywna'");
    query.bindValue(":idZajec", idZajec);

    if (!query.exec() || !query.next()) {
        qWarning() << "Błąd liczenia aktywnych rezerwacji:" << query.lastError().text();
        return 0;
    }

    return query.value(0).toInt();
}

bool DatabaseManager::moznaZarezerwowac(int idZajec) {
    // Pobierz limit uczestników dla zajęć
    QSqlQuery query(db);
    query.prepare("SELECT maksUczestnikow FROM zajecia WHERE id = :id");
    query.bindValue(":id", idZajec);

    if (!query.exec() || !query.next()) {
        qWarning() << "Błąd pobierania limitu zajęć:" << query.lastError().text();
        return false;
    }

    int limit = query.value(0).toInt();
    int aktualne = getIloscAktywnychRezerwacji(idZajec);

    return aktualne < limit;
}

QList<Rezerwacja> DatabaseManager::getRezerwacjeKlienta(int idKlienta) {
    QList<Rezerwacja> rezerwacje;

    QSqlQuery query(db);
    query.prepare(R"(
        SELECT r.id, r.idKlienta, r.idZajec, r.dataRezerwacji, r.status,
               k.imie, k.nazwisko,
               z.nazwa, z.trener, z.data, z.czas
        FROM rezerwacja r
        JOIN klient k ON r.idKlienta = k.id
        JOIN zajecia z ON r.idZajec = z.id
        WHERE r.idKlienta = :idKlienta
        ORDER BY z.data, z.czas
    )");
    query.bindValue(":idKlienta", idKlienta);

    if (!query.exec()) {
        qWarning() << "Błąd pobierania rezerwacji klienta:" << query.lastError().text();
        return rezerwacje;
    }

    while (query.next()) {
        rezerwacje.append(queryToRezerwacja(query));
    }

    return rezerwacje;
}

QList<Rezerwacja> DatabaseManager::getRezerwacjeZajec(int idZajec) {
    QList<Rezerwacja> rezerwacje;

    QSqlQuery query(db);
    query.prepare(R"(
        SELECT r.id, r.idKlienta, r.idZajec, r.dataRezerwacji, r.status,
               k.imie, k.nazwisko,
               z.nazwa, z.trener, z.data, z.czas
        FROM rezerwacja r
        JOIN klient k ON r.idKlienta = k.id
        JOIN zajecia z ON r.idZajec = z.id
        WHERE r.idZajec = :idZajec AND r.status = 'aktywna'
        ORDER BY k.nazwisko, k.imie
    )");
    query.bindValue(":idZajec", idZajec);

    if (!query.exec()) {
        qWarning() << "Błąd pobierania rezerwacji zajęć:" << query.lastError().text();
        return rezerwacje;
    }

    while (query.next()) {
        rezerwacje.append(queryToRezerwacja(query));
    }

    return rezerwacje;
}

QList<Zajecia> DatabaseManager::getZajeciaDostepneDoRezerwacji() {
    QList<Zajecia> zajecia;

    QSqlQuery query(db);
    if (!query.exec(R"(
        SELECT z.id, z.nazwa, z.trener, z.maksUczestnikow, z.data, z.czas, z.czasTrwania, z.opis,
               COUNT(r.id) as aktualne_rezerwacje
        FROM zajecia z
        LEFT JOIN rezerwacja r ON z.id = r.idZajec AND r.status = 'aktywna'
        GROUP BY z.id, z.nazwa, z.trener, z.maksUczestnikow, z.data, z.czas, z.czasTrwania, z.opis
        HAVING COUNT(r.id) < z.maksUczestnikow
        ORDER BY z.data, z.czas
    )")) {
        qWarning() << "Błąd pobierania dostępnych zajęć:" << query.lastError().text();
        return zajecia;
    }

    while (query.next()) {
        zajecia.append(queryToZajecia(query));
    }

    return zajecia;
}

int DatabaseManager::getRezerwacjeCount() {
    QSqlQuery query(db);
    if (!query.exec("SELECT COUNT(*) FROM rezerwacja")) {
        qWarning() << "Błąd liczenia rezerwacji:" << query.lastError().text();
        return 0;
    }

    if (query.next()) {
        return query.value(0).toInt();
    }

    return 0;
}

// === Metody raportowe ===

QList<QPair<QString, int>> DatabaseManager::getNajpopularniejszeZajecia(int limit) {
    QList<QPair<QString, int>> wyniki;

    QSqlQuery query(db);
    query.prepare(R"(
        SELECT z.nazwa, COUNT(r.id) as liczba_rezerwacji
        FROM zajecia z
        LEFT JOIN rezerwacja r ON z.id = r.idZajec AND r.status = 'aktywna'
        GROUP BY z.id, z.nazwa
        ORDER BY liczba_rezerwacji DESC, z.nazwa
        LIMIT :limit
    )");
    query.bindValue(":limit", limit);

    if (!query.exec()) {
        qWarning() << "Błąd pobierania najpopularniejszych zajęć:" << query.lastError().text();
        return wyniki;
    }

    while (query.next()) {
        QString nazwa = query.value("nazwa").toString();
        int liczba = query.value("liczba_rezerwacji").toInt();
        wyniki.append(qMakePair(nazwa, liczba));
    }

    return wyniki;
}

QList<QPair<QString, int>> DatabaseManager::getNajaktywniejszychKlientow(int limit) {
    QList<QPair<QString, int>> wyniki;

    QSqlQuery query(db);
    query.prepare(R"(
        SELECT k.imie || ' ' || k.nazwisko as pelne_imie, COUNT(r.id) as liczba_rezerwacji
        FROM klient k
        LEFT JOIN rezerwacja r ON k.id = r.idKlienta AND r.status = 'aktywna'
        GROUP BY k.id, k.imie, k.nazwisko
        ORDER BY liczba_rezerwacji DESC, k.nazwisko, k.imie
        LIMIT :limit
    )");
    query.bindValue(":limit", limit);

    if (!query.exec()) {
        qWarning() << "Błąd pobierania najaktywniejszych klientów:" << query.lastError().text();
        return wyniki;
    }

    while (query.next()) {
        QString nazwa = query.value("pelne_imie").toString();
        int liczba = query.value("liczba_rezerwacji").toInt();
        wyniki.append(qMakePair(nazwa, liczba));
    }

    return wyniki;
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

Rezerwacja DatabaseManager::queryToRezerwacja(QSqlQuery& query) {
    Rezerwacja rezerwacja;
    rezerwacja.id = query.value("id").toInt();
    rezerwacja.idKlienta = query.value("idKlienta").toInt();
    rezerwacja.idZajec = query.value("idZajec").toInt();
    rezerwacja.dataRezerwacji = query.value("dataRezerwacji").toString();
    rezerwacja.status = query.value("status").toString();

    // Informacje z joinów
    rezerwacja.imieKlienta = query.value("imie").toString();
    rezerwacja.nazwiskoKlienta = query.value("nazwisko").toString();
    rezerwacja.nazwaZajec = query.value("nazwa").toString();
    rezerwacja.trenerZajec = query.value("trener").toString();
    rezerwacja.dataZajec = query.value("data").toString();
    rezerwacja.czasZajec = query.value("czas").toString();

    return rezerwacja;
}
