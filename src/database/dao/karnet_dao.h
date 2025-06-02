#ifndef KARNET_DAO_H
#define KARNET_DAO_H

#include <vector>
#include <memory>
#include "../../models/karnet.h"
#include "../menedzer_bd.h"

class KarnetDAO {
public:
    explicit KarnetDAO(MenedzerBD& menedzerBD);


    std::vector<Karnet> pobierzWszystkie();
    std::unique_ptr<Karnet> pobierzPoId(int id);
    std::vector<Karnet> pobierzDlaKlienta(int idKlienta);
    int dodaj(const Karnet& karnet);
    bool aktualizuj(const Karnet& karnet);
    bool usun(int id);


    bool czyKlientMaAktywnyKarnet(int idKlienta);

private:
    MenedzerBD& menedzerBD;


    Karnet utworzKarnetZWiersza(const WierszBD& wiersz);
};

#endif 