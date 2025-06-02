#include "uslugi_zajec.h"

UslugiZajec::UslugiZajec(ZajeciaDAO& zajeciaDAO, UslugiKarnetu& uslugiKarnetu)
    : zajeciaDAO(zajeciaDAO), uslugiKarnetu(uslugiKarnetu) {
}

std::vector<Zajecia> UslugiZajec::pobierzWszystkieZajecia() {
    return zajeciaDAO.pobierzWszystkieZajecia();
}

std::unique_ptr<Zajecia> UslugiZajec::pobierzZajeciaPoId(int id) {
    return zajeciaDAO.pobierzZajeciaPoId(id);
}

int UslugiZajec::dodajZajecia(const Zajecia& zajecia) {
    return zajeciaDAO.dodajZajecia(zajecia);
}

bool UslugiZajec::aktualizujZajecia(const Zajecia& zajecia) {
    return zajeciaDAO.aktualizujZajecia(zajecia);
}

bool UslugiZajec::usunZajecia(int id) {
    return zajeciaDAO.usunZajecia(id);
}

std::vector<Rezerwacja> UslugiZajec::pobierzWszystkieRezerwacje() {
    return zajeciaDAO.pobierzWszystkieRezerwacje();
}

std::vector<Rezerwacja> UslugiZajec::pobierzRezerwacjeKlienta(int idKlienta) {
    return zajeciaDAO.pobierzRezerwacjeKlienta(idKlienta);
}

std::vector<Rezerwacja> UslugiZajec::pobierzRezerwacjeZajec(int idZajec) {
    return zajeciaDAO.pobierzRezerwacjeZajec(idZajec);
}

int UslugiZajec::dodajRezerwacje(const Rezerwacja& rezerwacja) {
    return zajeciaDAO.dodajRezerwacje(rezerwacja);
}

bool UslugiZajec::anulujRezerwacje(int idRezerwacji) {
    auto rezerwacja = std::find_if(
        pobierzWszystkieRezerwacje().begin(),
        pobierzWszystkieRezerwacje().end(),
        [idRezerwacji](const Rezerwacja& r) { return r.pobierzId() == idRezerwacji; }
    );

    if (rezerwacja != pobierzWszystkieRezerwacje().end()) {
        Rezerwacja aktualizowanaRezerwacja = *rezerwacja;
        aktualizowanaRezerwacja.ustawStatus("anulowana");
        return zajeciaDAO.aktualizujRezerwacje(aktualizowanaRezerwacja);
    }

    return false;
}

int UslugiZajec::pobierzDostepneMiejscaZajec(int idZajec) {
    auto zajecia = pobierzZajeciaPoId(idZajec);
    if (!zajecia) return 0;

    int maksUczestnikow = zajecia->pobierzMaksUczestnikow();
    int zajeteMiejsca = zajeciaDAO.policzRezerwacjeZajec(idZajec);

    return maksUczestnikow - zajeteMiejsca;
}

bool UslugiZajec::czyKlientUprawniony(int idKlienta, int idZajec) {
    return uslugiKarnetu.czyKlientMaAktywnyKarnet(idKlienta);
}