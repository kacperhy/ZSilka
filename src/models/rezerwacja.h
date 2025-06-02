#ifndef REZERWACJA_H
#define REZERWACJA_H

#include <string>
#include <ctime>
#include <iomanip>
#include <sstream>

class Rezerwacja {
public:
    Rezerwacja();
    Rezerwacja(int id, int idKlienta, int idZajec,
        const std::string& dataRezerwacji, const std::string& status);

    int pobierzId() const;
    int pobierzIdKlienta() const;
    int pobierzIdZajec() const;
    std::string pobierzDateRezerwacji() const;
    std::string pobierzStatus() const;

    void ustawId(int id);
    void ustawIdKlienta(int idKlienta);
    void ustawIdZajec(int idZajec);
    void ustawDateRezerwacji(const std::string& dataRezerwacji);
    void ustawStatus(const std::string& status);

    bool czyPotwierdzona() const;

    static std::string pobierzAktualnyCzas();

private:
    int id;
    int idKlienta;
    int idZajec;
    std::string dataRezerwacji;
    std::string status;  // 'potwierdzona' lub 'anulowana'
};

#endif 