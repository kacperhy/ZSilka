#ifndef KARNET_H
#define KARNET_H

#include <string>
#include <ctime>
#include <chrono>
#include <iomanip>
#include <sstream>

class Karnet {
public:
    Karnet();
    Karnet(int id, int idKlienta, const std::string& typ,
        const std::string& dataRozpoczecia, const std::string& dataZakonczenia,
        double cena, bool czyAktywny);

    int pobierzId() const;
    int pobierzIdKlienta() const;
    std::string pobierzTyp() const;
    std::string pobierzDateRozpoczecia() const;
    std::string pobierzDateZakonczenia() const;
    double pobierzCene() const;
    bool pobierzCzyAktywny() const;

    void ustawId(int id);
    void ustawIdKlienta(int idKlienta);
    void ustawTyp(const std::string& typ);
    void ustawDateRozpoczecia(const std::string& dataRozpoczecia);
    void ustawDateZakonczenia(const std::string& dataZakonczenia);
    void ustawCene(double cena);
    void ustawCzyAktywny(bool czyAktywny);

    bool czyWazny() const;  // Sprawdza, czy karnet jest ważny na dzień dzisiejszy
    int ileDniPozostalo() const;  // Oblicza liczbę dni pozostałych do końca ważności karnetu

    static std::string pobierzAktualnaDate();

    static std::string dodajDniDoData(const std::string& data, int dni);

private:
    int id;
    int idKlienta;
    std::string typ;  // 'normalny' lub 'studencki'
    std::string dataRozpoczecia;
    std::string dataZakonczenia;
    double cena;
    bool czyAktywny;

    static std::tm konwertujStringNaDate(const std::string& tekstDaty);

    static int dniPomiedzy(const std::string& data1, const std::string& data2);
};

#endif 