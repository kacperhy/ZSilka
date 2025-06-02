#ifndef ZAJECIA_H
#define ZAJECIA_H

#include <string>
#include <vector>

class Zajecia {
public:
    Zajecia();
    Zajecia(int id, const std::string& nazwa, const std::string& trener,
        int maksUczestnikow, const std::string& data, const std::string& czas,
        int czasTrwania, const std::string& opis);

    int pobierzId() const;
    std::string pobierzNazwe() const;
    std::string pobierzTrenera() const;
    int pobierzMaksUczestnikow() const;
    std::string pobierzDate() const;
    std::string pobierzCzas() const;
    int pobierzCzasTrwania() const;
    std::string pobierzOpis() const;

    void ustawId(int id);
    void ustawNazwe(const std::string& nazwa);
    void ustawTrenera(const std::string& trener);
    void ustawMaksUczestnikow(int maksUczestnikow);
    void ustawDate(const std::string& data);
    void ustawCzas(const std::string& czas);
    void ustawCzasTrwania(int czasTrwania);
    void ustawOpis(const std::string& opis);

private:
    int id;
    std::string nazwa;
    std::string trener;
    int maksUczestnikow;
    std::string data;
    std::string czas;
    int czasTrwania;  // czas trwania w minutach
    std::string opis;
};

#endif 