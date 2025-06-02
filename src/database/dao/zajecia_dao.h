#ifndef ZAJECIA_DAO_H
#define ZAJECIA_DAO_H

#include <vector>
#include <memory>
#include "../../models/zajecia.h"
#include "../../models/rezerwacja.h"
#include "../menedzer_bd.h"

class ZajeciaDAO {
public:
    explicit ZajeciaDAO(MenedzerBD& menedzerBD);


    std::vector<Zajecia> pobierzWszystkieZajecia();
    std::unique_ptr<Zajecia> pobierzZajeciaPoId(int id);
    int dodajZajecia(const Zajecia& zajecia);
    bool aktualizujZajecia(const Zajecia& zajecia);
    bool usunZajecia(int id);

   
    std::vector<Rezerwacja> pobierzWszystkieRezerwacje();
    std::vector<Rezerwacja> pobierzRezerwacjeKlienta(int idKlienta);
    std::vector<Rezerwacja> pobierzRezerwacjeZajec(int idZajec);
    int dodajRezerwacje(const Rezerwacja& rezerwacja);
    bool aktualizujRezerwacje(const Rezerwacja& rezerwacja);
    bool usunRezerwacje(int id);

    // Metody biznesowe
    int policzRezerwacjeZajec(int idZajec);

private:
    MenedzerBD& menedzerBD;

    // Metody pomocnicze
    Zajecia utworzZajeciaZWiersza(const WierszBD& wiersz);
    Rezerwacja utworzRezerwacjeZWiersza(const WierszBD& wiersz);
};

#endif // ZAJECIA_DAO_H