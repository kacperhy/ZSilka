#ifndef KLIENT_H
#define KLIENT_H

#include <string>
#include <ctime>
#include <iomanip>
#include <sstream>

class Klient {
public:
    Klient();
    Klient(int id, const std::string& imie, const std::string& nazwisko,
        const std::string& email, const std::string& telefon,
        const std::string& dataUrodzenia, const std::string& dataRejestracji,
        const std::string& uwagi);

    int pobierzId() const;
    std::string pobierzImie() const;
    std::string pobierzNazwisko() const;
    std::string pobierzEmail() const;
    std::string pobierzTelefon() const;
    std::string pobierzDateUrodzenia() const;
    std::string pobierzDateRejestracji() const;
    std::string pobierzUwagi() const;
    std::string pobierzPelneNazwisko() const;

    void ustawId(int id);
    void ustawImie(const std::string& imie);
    void ustawNazwisko(const std::string& nazwisko);
    void ustawEmail(const std::string& email);
    void ustawTelefon(const std::string& telefon);
    void ustawDateUrodzenia(const std::string& dataUrodzenia);
    void ustawDateRejestracji(const std::string& dataRejestracji);
    void ustawUwagi(const std::string& uwagi);

    static std::string pobierzAktualnaDate();

private:
    int id;
    std::string imie;
    std::string nazwisko;
    std::string email;
    std::string telefon;
    std::string dataUrodzenia;
    std::string dataRejestracji;
    std::string uwagi;
};

#endif 