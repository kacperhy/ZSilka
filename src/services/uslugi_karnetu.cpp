#include "uslugi_karnetu.h"

UslugiKarnetu::UslugiKarnetu(KarnetDAO& karnetDAO) : karnetDAO(karnetDAO) {
}

std::vector<Karnet> UslugiKarnetu::pobierzWszystkieKarnety() {
    return karnetDAO.pobierzWszystkie();
}

std::unique_ptr<Karnet> UslugiKarnetu::pobierzKarnetPoId(int id) {
    return karnetDAO.pobierzPoId(id);
}

std::vector<Karnet> UslugiKarnetu::pobierzKarnetyKlienta(int idKlienta) {
    return karnetDAO.pobierzDlaKlienta(idKlienta);
}

int UslugiKarnetu::dodajKarnet(const Karnet& karnet) {
    return karnetDAO.dodaj(karnet);
}

bool UslugiKarnetu::aktualizujKarnet(const Karnet& karnet) {
    return karnetDAO.aktualizuj(karnet);
}

bool UslugiKarnetu::usunKarnet(int id) {
    return karnetDAO.usun(id);
}

Karnet UslugiKarnetu::utworzKarnetMiesieczny(int idKlienta, bool czyStudent) {
    std::string typKarnetu = pobierzTypKarnetu("miesieczny", czyStudent);
    std::string dataRozpoczecia = Karnet::pobierzAktualnaDate();
    std::string dataZakonczenia = Karnet::dodajDniDoData(dataRozpoczecia, 30);
    double cena = obliczCene("miesieczny", czyStudent);

    Karnet karnet(-1, idKlienta, typKarnetu, dataRozpoczecia, dataZakonczenia, cena, true);
    return karnet;
}

Karnet UslugiKarnetu::utworzKarnetKwartalny(int idKlienta, bool czyStudent) {
    std::string typKarnetu = pobierzTypKarnetu("kwartalny", czyStudent);
    std::string dataRozpoczecia = Karnet::pobierzAktualnaDate();
    std::string dataZakonczenia = Karnet::dodajDniDoData(dataRozpoczecia, 90);
    double cena = obliczCene("kwartalny", czyStudent);

    Karnet karnet(-1, idKlienta, typKarnetu, dataRozpoczecia, dataZakonczenia, cena, true);
    return karnet;
}

Karnet UslugiKarnetu::utworzKarnetRoczny(int idKlienta, bool czyStudent) {
    std::string typKarnetu = pobierzTypKarnetu("roczny", czyStudent);
    std::string dataRozpoczecia = Karnet::pobierzAktualnaDate();
    std::string dataZakonczenia = Karnet::dodajDniDoData(dataRozpoczecia, 365);
    double cena = obliczCene("roczny", czyStudent);

    Karnet karnet(-1, idKlienta, typKarnetu, dataRozpoczecia, dataZakonczenia, cena, true);
    return karnet;
}

double UslugiKarnetu::obliczCene(const std::string& typ, bool czyStudent) {
    double cena = 0.0;

    if (typ == "miesieczny") {
        cena = CENA_MIESIECZNY;
    }
    else if (typ == "kwartalny") {
        cena = CENA_KWARTALNY;
    }
    else if (typ == "roczny") {
        cena = CENA_ROCZNY;
    }

    if (czyStudent) {
        cena = cena * (1.0 - ZNIZKA_STUDENCKA / 100.0);
    }

    return cena;
}

std::string UslugiKarnetu::pobierzTypKarnetu(const std::string& bazaTypu, bool czyStudent) {
    if (czyStudent) {
        return "student_" + bazaTypu;
    }
    else {
        return "normalny_" + bazaTypu;
    }
}

bool UslugiKarnetu::czyKlientMaAktywnyKarnet(int idKlienta) {
    return karnetDAO.czyKlientMaAktywnyKarnet(idKlienta);
}