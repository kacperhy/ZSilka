#ifndef HISTORIA_ZMIAN_H
#define HISTORIA_ZMIAN_H

#include <string>
#include <vector>
#include <memory>
#include <ctime>
#include <iomanip>
#include <sstream>
#include "../database/menedzer_bd.h"

// Struktura reprezentująca log operacji
struct LogOperacji {
    int id;
    std::string typOperacji;    // INSERT, UPDATE, DELETE
    std::string tabela;         // klienci, karnety, zajecia, rezerwacje
    int idRekordu;
    std::string danePrzed;      // JSON z danymi przed zmianą
    std::string danePo;         // JSON z danymi po zmianie
    std::string uzytkownik;     // nazwa użytkownika (domyślnie "system")
    std::string czasOperacji;   // timestamp operacji
    std::string opis;           // opcjonalny opis operacji
};

// Struktura reprezentująca punkt przywracania
struct PunktPrzywracania {
    int id;
    std::string nazwa;
    std::string opis;
    std::string czasUtworzenia;
};

class HistoriaZmian {
public:
    explicit HistoriaZmian(MenedzerBD& menedzerBD);

    // Inicjalizacja tabel historii (jeśli nie istnieją)
    bool inicjalizujTabele();

    // Logowanie operacji
    int logujOperacje(const std::string& typOperacji, const std::string& tabela, 
                     int idRekordu, const std::string& danePrzed, const std::string& danePo,
                     const std::string& opis = "");

    // Cofanie operacji
    bool cofnijOstatnia();
    bool cofnijOperacje(int idOperacji);
    
    // Punkty przywracania
    int utworzPunktPrzywracania(const std::string& nazwa, const std::string& opis = "");
    bool przywrocDoPunktu(int idPunktu);
    std::vector<PunktPrzywracania> pobierzPunktyPrzywracania();
    bool usunPunktPrzywracania(int idPunktu);

    // Przeglądanie historii
    std::vector<LogOperacji> pobierzHistorie(int limit = 50);
    std::vector<LogOperacji> pobierzHistorieTabeli(const std::string& tabela, int limit = 50);
    std::vector<LogOperacji> pobierzHistorieRekordu(const std::string& tabela, int idRekordu);
    
    // Statystyki
    int policzOperacje(const std::string& typOperacji = "");
    std::string generujRaportHistorii();

    // Czyszczenie historii
    bool wyczyscStaraHistorie(int dniDoZachowania = 30);
    bool wyczyscWszystko();

private:
    MenedzerBD& menedzerBD;

    // Metody pomocnicze
    std::string pobierzAktualnyCzas();
    std::string escapujJSON(const std::string& str);
    LogOperacji utworzLogZWiersza(const WierszBD& wiersz);
    PunktPrzywracania utworzPunktZWiersza(const WierszBD& wiersz);
    
    // Metody cofania konkretnych operacji
    bool cofnijInsert(const LogOperacji& log);
    bool cofnijUpdate(const LogOperacji& log);
    bool cofnijDelete(const LogOperacji& log);
};

// Klasa pomocnicza do automatycznego logowania w serwisach
class AutoLogger {
public:
    AutoLogger(HistoriaZmian& historia, const std::string& tabela, int idRekordu, const std::string& danePrzed);
    ~AutoLogger();
    
    void ustawDanePo(const std::string& danePo);
    void ustawTypOperacji(const std::string& typ);
    void ustawOpis(const std::string& opis);
    void anuluj(); // Nie loguj tej operacji

private:
    HistoriaZmian& historia;
    std::string tabela;
    int idRekordu;
    std::string danePrzed;
    std::string danePo;
    std::string typOperacji;
    std::string opis;
    bool anulowano;
};

#endif // HISTORIA_ZMIAN_H