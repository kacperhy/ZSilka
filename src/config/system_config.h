#ifndef SYSTEM_CONFIG_H
#define SYSTEM_CONFIG_H

#include <string>
#include <map>

struct SystemConfig {
    // Konfiguracja bazy danych
    std::string databasePath = "silownia.db";
    int maxConnections = 10;
    bool enableLogging = true;
    
    // Konfiguracja raportów
    std::string defaultReportPath = "raporty/";
    std::string defaultFormat = "HTML";
    bool autoBackupReports = true;
    
    // Konfiguracja historii
    int maxHistoryDays = 90;        // Dni przechowywania historii
    int maxHistoryOperations = 1000; // Max operacji w pamięci
    bool autoCleanup = true;         // Automatyczne czyszczenie
    
    // Konfiguracja karnetów - ceny domyślne
    struct PriceConfig {
        double normalny_miesieczny = 120.0;
        double student_miesieczny = 80.0;
        double normalny_kwartalny = 300.0;
        double student_kwartalny = 200.0;
        double normalny_roczny = 1000.0;
        double student_roczny = 600.0;
    } ceny;
    
    // Konfiguracja interfejsu
    std::string language = "pl";     // pl, en
    bool colorOutput = true;         // Kolorowe wyjście w konsoli
    int pageSize = 20;              // Ilość rekordów na stronę
    
    // Automatyczne punkty przywracania
    bool autoRestorePoints = true;
    int autoRestoreInterval = 60;    // minuty między automatycznymi punktami
    
    // Konfiguracja alertów
    int karnetExpiryWarningDays = 7; // Ostrzeżenie X dni przed wygaśnięciem
    bool emailNotifications = false;
    std::string smtpServer = "";
    std::string emailFrom = "";
    
    // Metody do wczytywania/zapisywania konfiguracji
    bool wczytajZPliku(const std::string& sciezka = "config.ini");
    bool zapiszDoPliku(const std::string& sciezka = "config.ini");
    void ustawDomyslne();
    
    // Walidacja konfiguracji
    bool sprawdzPoprawnosc() const;
    std::string pobierzBleady() const;
};

#endif // SYSTEM_CONFIG_H