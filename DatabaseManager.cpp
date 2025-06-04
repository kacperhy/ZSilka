#include "DatabaseManager.h"
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <QDebug>
#include <QDateTime>
#include <QTextStream>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>

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

// === CRUD dla ZAJĘĆ === (pozostają bez zmian)

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

// === CRUD dla REZERWACJI === (pozostają bez zmian - skrócone dla oszczędności miejsca)

bool DatabaseManager::addRezerwacja(int idKlienta, int idZajec, const QString& status) {
    if (klientMaRezerwacje(idKlienta, idZajec)) {
        qWarning() << "Klient już ma rezerwację na te zajęcia. Klient ID:" << idKlienta << "Zajęcia ID:" << idZajec;
        return false;
    }

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

// === CRUD dla KARNETÓW ===

bool DatabaseManager::addKarnet(int idKlienta,
                                const QString& typ,
                                const QString& dataRozpoczecia,
                                const QString& dataZakonczenia,
                                double cena,
                                bool czyAktywny) {

    // Sprawdź czy można utworzyć karnet (czy klient nie ma już aktywnego karnetu tego typu)
    if (!moznaUtworzycKarnet(idKlienta, typ)) {
        qWarning() << "Klient już ma aktywny karnet typu:" << typ << "dla klienta ID:" << idKlienta;
        return false;
    }

    QSqlQuery query(db);
    query.prepare(R"(
        INSERT INTO karnet (idKlienta, typ, dataRozpoczecia, dataZakonczenia, cena, czyAktywny)
        VALUES (:idKlienta, :typ, :dataRozpoczecia, :dataZakonczenia, :cena, :czyAktywny)
    )");

    query.bindValue(":idKlienta", idKlienta);
    query.bindValue(":typ", typ);
    query.bindValue(":dataRozpoczecia", dataRozpoczecia);
    query.bindValue(":dataZakonczenia", dataZakonczenia);
    query.bindValue(":cena", cena);
    query.bindValue(":czyAktywny", czyAktywny ? 1 : 0);

    if (!query.exec()) {
        qWarning() << "Błąd dodawania karnetu:" << query.lastError().text();
        return false;
    }

    qDebug() << "Dodano karnet typu" << typ << "dla klienta ID:" << idKlienta;
    return true;
}

QList<Karnet> DatabaseManager::getAllKarnety() {
    QList<Karnet> karnety;

    QSqlQuery query(db);
    if (!query.exec(R"(
        SELECT k.id, k.idKlienta, k.typ, k.dataRozpoczecia, k.dataZakonczenia, k.cena, k.czyAktywny,
               kl.imie, kl.nazwisko, kl.email
        FROM karnet k
        JOIN klient kl ON k.idKlienta = kl.id
        ORDER BY k.dataRozpoczecia DESC, kl.nazwisko, kl.imie
    )")) {
        qWarning() << "Błąd pobierania karnetów:" << query.lastError().text();
        return karnety;
    }

    while (query.next()) {
        karnety.append(queryToKarnet(query));
    }

    return karnety;
}

Karnet DatabaseManager::getKarnetById(int id) {
    Karnet karnet = {};

    QSqlQuery query(db);
    query.prepare(R"(
        SELECT k.id, k.idKlienta, k.typ, k.dataRozpoczecia, k.dataZakonczenia, k.cena, k.czyAktywny,
               kl.imie, kl.nazwisko, kl.email
        FROM karnet k
        JOIN klient kl ON k.idKlienta = kl.id
        WHERE k.id = :id
    )");
    query.bindValue(":id", id);

    if (!query.exec()) {
        qWarning() << "Błąd pobierania karnetu o ID" << id << ":" << query.lastError().text();
        return karnet;
    }

    if (query.next()) {
        karnet = queryToKarnet(query);
    }

    return karnet;
}

bool DatabaseManager::updateKarnet(int id,
                                   int idKlienta,
                                   const QString& typ,
                                   const QString& dataRozpoczecia,
                                   const QString& dataZakonczenia,
                                   double cena,
                                   bool czyAktywny) {

    QSqlQuery query(db);
    query.prepare(R"(
        UPDATE karnet
        SET idKlienta = :idKlienta, typ = :typ, dataRozpoczecia = :dataRozpoczecia,
            dataZakonczenia = :dataZakonczenia, cena = :cena, czyAktywny = :czyAktywny
        WHERE id = :id
    )");

    query.bindValue(":id", id);
    query.bindValue(":idKlienta", idKlienta);
    query.bindValue(":typ", typ);
    query.bindValue(":dataRozpoczecia", dataRozpoczecia);
    query.bindValue(":dataZakonczenia", dataZakonczenia);
    query.bindValue(":cena", cena);
    query.bindValue(":czyAktywny", czyAktywny ? 1 : 0);

    if (!query.exec()) {
        qWarning() << "Błąd aktualizacji karnetu:" << query.lastError().text();
        return false;
    }

    if (query.numRowsAffected() == 0) {
        qWarning() << "Nie znaleziono karnetu o ID:" << id;
        return false;
    }

    qDebug() << "Zaktualizowano karnet o ID:" << id;
    return true;
}

bool DatabaseManager::deleteKarnet(int id) {
    QSqlQuery query(db);
    query.prepare("DELETE FROM karnet WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        qWarning() << "Błąd usuwania karnetu:" << query.lastError().text();
        return false;
    }

    if (query.numRowsAffected() == 0) {
        qWarning() << "Nie znaleziono karnetu o ID:" << id;
        return false;
    }

    qDebug() << "Usunięto karnet o ID:" << id;
    return true;
}

// === Pomocnicze metody dla karnetów ===

QList<Karnet> DatabaseManager::getKarnetyKlienta(int idKlienta) {
    QList<Karnet> karnety;

    QSqlQuery query(db);
    query.prepare(R"(
        SELECT k.id, k.idKlienta, k.typ, k.dataRozpoczecia, k.dataZakonczenia, k.cena, k.czyAktywny,
               kl.imie, kl.nazwisko, kl.email
        FROM karnet k
        JOIN klient kl ON k.idKlienta = kl.id
        WHERE k.idKlienta = :idKlienta
        ORDER BY k.dataRozpoczecia DESC
    )");
    query.bindValue(":idKlienta", idKlienta);

    if (!query.exec()) {
        qWarning() << "Błąd pobierania karnetów klienta:" << query.lastError().text();
        return karnety;
    }

    while (query.next()) {
        karnety.append(queryToKarnet(query));
    }

    return karnety;
}

QList<Karnet> DatabaseManager::getAktywneKarnetyKlienta(int idKlienta) {
    QList<Karnet> karnety;

    QSqlQuery query(db);
    query.prepare(R"(
        SELECT k.id, k.idKlienta, k.typ, k.dataRozpoczecia, k.dataZakonczenia, k.cena, k.czyAktywny,
               kl.imie, kl.nazwisko, kl.email
        FROM karnet k
        JOIN klient kl ON k.idKlienta = kl.id
        WHERE k.idKlienta = :idKlienta AND k.czyAktywny = 1
        ORDER BY k.dataRozpoczecia DESC
    )");
    query.bindValue(":idKlienta", idKlienta);

    if (!query.exec()) {
        qWarning() << "Błąd pobierania aktywnych karnetów klienta:" << query.lastError().text();
        return karnety;
    }

    while (query.next()) {
        karnety.append(queryToKarnet(query));
    }

    return karnety;
}

bool DatabaseManager::klientMaAktywnyKarnet(int idKlienta) {
    QSqlQuery query(db);
    query.prepare("SELECT COUNT(*) FROM karnet WHERE idKlienta = :idKlienta AND czyAktywny = 1");
    query.bindValue(":idKlienta", idKlienta);

    if (!query.exec() || !query.next()) {
        qWarning() << "Błąd sprawdzania aktywnych karnetów klienta:" << query.lastError().text();
        return false;
    }

    return query.value(0).toInt() > 0;
}

QList<Karnet> DatabaseManager::getKarnetyByTyp(const QString& typ) {
    QList<Karnet> karnety;

    QSqlQuery query(db);
    query.prepare(R"(
        SELECT k.id, k.idKlienta, k.typ, k.dataRozpoczecia, k.dataZakonczenia, k.cena, k.czyAktywny,
               kl.imie, kl.nazwisko, kl.email
        FROM karnet k
        JOIN klient kl ON k.idKlienta = kl.id
        WHERE k.typ = :typ
        ORDER BY k.dataRozpoczecia DESC, kl.nazwisko, kl.imie
    )");
    query.bindValue(":typ", typ);

    if (!query.exec()) {
        qWarning() << "Błąd pobierania karnetów po typie:" << query.lastError().text();
        return karnety;
    }

    while (query.next()) {
        karnety.append(queryToKarnet(query));
    }

    return karnety;
}

QList<Karnet> DatabaseManager::getKarnetyByStatus(bool czyAktywny) {
    QList<Karnet> karnety;

    QSqlQuery query(db);
    query.prepare(R"(
        SELECT k.id, k.idKlienta, k.typ, k.dataRozpoczecia, k.dataZakonczenia, k.cena, k.czyAktywny,
               kl.imie, kl.nazwisko, kl.email
        FROM karnet k
        JOIN klient kl ON k.idKlienta = kl.id
        WHERE k.czyAktywny = :czyAktywny
        ORDER BY k.dataRozpoczecia DESC, kl.nazwisko, kl.imie
    )");
    query.bindValue(":czyAktywny", czyAktywny ? 1 : 0);

    if (!query.exec()) {
        qWarning() << "Błąd pobierania karnetów po statusie:" << query.lastError().text();
        return karnety;
    }

    while (query.next()) {
        karnety.append(queryToKarnet(query));
    }

    return karnety;
}

QList<Karnet> DatabaseManager::getKarnetyWygasajace(const QString& dataOd, const QString& dataDo) {
    QList<Karnet> karnety;

    QSqlQuery query(db);
    query.prepare(R"(
        SELECT k.id, k.idKlienta, k.typ, k.dataRozpoczecia, k.dataZakonczenia, k.cena, k.czyAktywny,
               kl.imie, kl.nazwisko, kl.email
        FROM karnet k
        JOIN klient kl ON k.idKlienta = kl.id
        WHERE k.dataZakonczenia BETWEEN :dataOd AND :dataDo AND k.czyAktywny = 1
        ORDER BY k.dataZakonczenia ASC, kl.nazwisko, kl.imie
    )");
    query.bindValue(":dataOd", dataOd);
    query.bindValue(":dataDo", dataDo);

    if (!query.exec()) {
        qWarning() << "Błąd pobierania wygasających karnetów:" << query.lastError().text();
        return karnety;
    }

    while (query.next()) {
        karnety.append(queryToKarnet(query));
    }

    return karnety;
}

int DatabaseManager::getKarnetyCount() {
    QSqlQuery query(db);
    if (!query.exec("SELECT COUNT(*) FROM karnet")) {
        qWarning() << "Błąd liczenia karnetów:" << query.lastError().text();
        return 0;
    }

    if (query.next()) {
        return query.value(0).toInt();
    }

    return 0;
}

bool DatabaseManager::moznaUtworzycKarnet(int idKlienta, const QString& typ) {
    QSqlQuery query(db);
    query.prepare("SELECT COUNT(*) FROM karnet WHERE idKlienta = :idKlienta AND typ = :typ AND czyAktywny = 1");
    query.bindValue(":idKlienta", idKlienta);
    query.bindValue(":typ", typ);

    if (!query.exec() || !query.next()) {
        qWarning() << "Błąd sprawdzania możliwości utworzenia karnetu:" << query.lastError().text();
        return false;
    }

    return query.value(0).toInt() == 0; // Można utworzyć jeśli nie ma aktywnych karnetów tego typu
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

QList<QPair<QString, int>> DatabaseManager::getStatystykiKarnetow() {
    QList<QPair<QString, int>> wyniki;

    QSqlQuery query(db);
    if (!query.exec(R"(
        SELECT typ, COUNT(*) as liczba
        FROM karnet
        WHERE czyAktywny = 1
        GROUP BY typ
        ORDER BY liczba DESC
    )")) {
        qWarning() << "Błąd pobierania statystyk karnetów:" << query.lastError().text();
        return wyniki;
    }

    while (query.next()) {
        QString typ = query.value("typ").toString();
        int liczba = query.value("liczba").toInt();
        wyniki.append(qMakePair(typ, liczba));
    }

    return wyniki;
}

double DatabaseManager::getCalkowitePrzychodyZKarnetow() {
    QSqlQuery query(db);
    if (!query.exec("SELECT SUM(cena) FROM karnet WHERE czyAktywny = 1")) {
        qWarning() << "Błąd liczenia przychodów z karnetów:" << query.lastError().text();
        return 0.0;
    }

    if (query.next()) {
        return query.value(0).toDouble();
    }

    return 0.0;
}

int DatabaseManager::getLiczbaAktywnychKarnetow() {
    QSqlQuery query(db);
    if (!query.exec("SELECT COUNT(*) FROM karnet WHERE czyAktywny = 1")) {
        qWarning() << "Błąd liczenia aktywnych karnetów:" << query.lastError().text();
        return 0;
    }

    if (query.next()) {
        return query.value(0).toInt();
    }

    return 0;
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

Karnet DatabaseManager::queryToKarnet(QSqlQuery& query) {
    Karnet karnet;
    karnet.id = query.value("id").toInt();
    karnet.idKlienta = query.value("idKlienta").toInt();
    karnet.typ = query.value("typ").toString();
    karnet.dataRozpoczecia = query.value("dataRozpoczecia").toString();
    karnet.dataZakonczenia = query.value("dataZakonczenia").toString();
    karnet.cena = query.value("cena").toDouble();
    karnet.czyAktywny = query.value("czyAktywny").toInt() == 1;

    // Informacje z joinów
    karnet.imieKlienta = query.value("imie").toString();
    karnet.nazwiskoKlienta = query.value("nazwisko").toString();
    karnet.emailKlienta = query.value("email").toString();

    return karnet;
}

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
