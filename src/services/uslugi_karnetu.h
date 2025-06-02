#ifndef USLUGI_KARNETU_H
#define USLUGI_KARNETU_H

#include <vector>
#include <memory>
#include <string>
#include "../models/karnet.h"
#include "../database/dao/karnet_dao.h"

class UslugiKarnetu {
public:
    explicit UslugiKarnetu(KarnetDAO& karnetDAO);

    std::vector<Karnet> pobierzWszystkieKarnety();
    std::unique_ptr<Karnet> pobierzKarnetPoId(int id);
    std::vector<Karnet> pobierzKarnetyKlienta(int idKlienta);
    int dodajKarnet(const Karnet& karnet);
    bool aktualizujKarnet(const Karnet& karnet);
    bool usunKarnet(int id);

    Karnet utworzKarnetMiesieczny(int idKlienta, bool czyStudent);
    Karnet utworzKarnetKwartalny(int idKlienta, bool czyStudent);
    Karnet utworzKarnetRoczny(int idKlienta, bool czyStudent);
    bool czyKlientMaAktywnyKarnet(int idKlienta);

    double obliczCene(const std::string& typ, bool czyStudent);
    std::string pobierzTypKarnetu(const std::string& bazaTypu, bool czyStudent);

private:
    KarnetDAO& karnetDAO;

    const double CENA_MIESIECZNY = 150.0;
    const double CENA_KWARTALNY = 400.0;
    const double CENA_ROCZNY = 1200.0;

    const double ZNIZKA_STUDENCKA = 20.0;
};

#endif 