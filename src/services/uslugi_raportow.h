#ifndef USLUGI_RAPORTOW_H
#define USLUGI_RAPORTOW_H

#include <string>
#include <vector>
#include <map>
#include "../database/menedzer_bd.h"


struct RaportAktywnosciKlienta {
    std::string nazwaKlienta;
    int lacznaLiczbaZajec;
    int liczbaAnulowanychZajec;
    std::string najczestszaZajecia;
    std::string ostatniaWizyta;
};


struct RaportPopularnosciZajec {
    std::string nazwaZajec;
    std::string trener;
    int lacznaLiczbaRezerwacji;
    double stopienWypelnienia;  // w procentach
};


struct RaportFinansowy {
    double lacznyPrzychod;
    int lacznaLiczbaKarnetow;
    double przychodStudencki;
    double przychodStandardowy;
    std::map<std::string, double> przychodWgTypuKarnetu;
    std::map<std::string, int> liczbaKarnetowWgTypu;
};

class UslugiRaportow {
public:
    explicit UslugiRaportow(MenedzerBD& menedzerBD);


    std::vector<RaportAktywnosciKlienta> generujRaportAktywnosciKlienta(
        const std::string& dataOd, const std::string& dataDo);

    std::vector<RaportPopularnosciZajec> generujRaportPopularnosciZajec(
        const std::string& dataOd, const std::string& dataDo);

    RaportFinansowy generujRaportFinansowy(
        const std::string& dataOd, const std::string& dataDo);

private:
    MenedzerBD& menedzerBD;
};

#endif 