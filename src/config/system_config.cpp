#include "system_config.h"
#include <fstream>
#include <sstream>
#include <iostream>

bool SystemConfig::wczytajZPliku(const std::string& sciezka) {
    std::ifstream plik(sciezka);
    if (!plik.is_open()) {
        std::cout << "Nie można otworzyć pliku konfiguracyjnego: " << sciezka 
                  << ". Używam domyślnych ustawień." << std::endl;
        ustawDomyslne();
        return false;
    }

    std::string linia;
    while (std::getline(plik, linia)) {
        // Pomijaj puste linie i komentarze
        if (linia.empty() || linia[0] == '#' || linia[0] == ';') {
            continue;
        }

        // Znajdź znak równości
        size_t pozycjaRownania = linia.find('=');
        if (pozycjaRownania == std::string::npos) {
            continue;
        }

        std::string klucz = linia.substr(0, pozycjaRownania);
        std::string wartosc = linia.substr(pozycjaRownania + 1);

        // Usuń białe znaki
        klucz.erase(0, klucz.find_first_not_of(" \t"));
        klucz.erase(klucz.find_last_not_of(" \t") + 1);
        wartosc.erase(0, wartosc.find_first_not_of(" \t"));
        wartosc.erase(wartosc.find_last_not_of(" \t") + 1);

        // Parsuj wartości
        if (klucz == "database_path") {
            databasePath = wartosc;
        } else if (klucz == "max_connections") {
            maxConnections = std::stoi(wartosc);
        } else if (klucz == "enable_logging") {
            enableLogging = (wartosc == "true" || wartosc == "1");
        } else if (klucz == "default_report_path") {
            defaultReportPath = wartosc;
        } else if (klucz == "default_format") {
            defaultFormat = wartosc;
        } else if (klucz == "auto_backup_reports") {
            autoBackupReports = (wartosc == "true" || wartosc == "1");
        } else if (klucz == "max_history_days") {
            maxHistoryDays = std::stoi(wartosc);
        } else if (klucz == "max_history_operations") {
            maxHistoryOperations = std::stoi(wartosc);
        } else if (klucz == "auto_cleanup") {
            autoCleanup = (wartosc == "true" || wartosc == "1");
        } else if (klucz == "language") {
            language = wartosc;
        } else if (klucz == "color_output") {
            colorOutput = (wartosc == "true" || wartosc == "1");
        } else if (klucz == "page_size") {
            pageSize = std::stoi(wartosc);
        } else if (klucz == "auto_restore_points") {
            autoRestorePoints = (wartosc == "true" || wartosc == "1");
        } else if (klucz == "auto_restore_interval") {
            autoRestoreInterval = std::stoi(wartosc);
        } else if (klucz == "karnet_expiry_warning_days") {
            karnetExpiryWarningDays = std::stoi(wartosc);
        } else if (klucz == "email_notifications") {
            emailNotifications = (wartosc == "true" || wartosc == "1");
        } else if (klucz == "smtp_server") {
            smtpServer = wartosc;
        } else if (klucz == "email_from") {
            emailFrom = wartosc;
        }
        // Ceny karnetów
        else if (klucz == "normalny_miesieczny") {
            ceny.normalny_miesieczny = std::stod(wartosc);
        } else if (klucz == "student_miesieczny") {
            ceny.student_miesieczny = std::stod(wartosc);
        } else if (klucz == "normalny_kwartalny") {
            ceny.normalny_kwartalny = std::stod(wartosc);
        } else if (klucz == "student_kwartalny") {
            ceny.student_kwartalny = std::stod(wartosc);
        } else if (klucz == "normalny_roczny") {
            ceny.normalny_roczny = std::stod(wartosc);
        } else if (klucz == "student_roczny") {
            ceny.student_roczny = std::stod(wartosc);
        }
    }

    plik.close();
    return true;
}

bool SystemConfig::zapiszDoPliku(const std::string& sciezka) {
    std::ofstream plik(sciezka);
    if (!plik.is_open()) {
        return false;
    }

    plik << "# Konfiguracja Systemu Zarządzania Siłownią\n";
    plik << "# Wygenerowano automatycznie\n\n";

    plik << "# Konfiguracja bazy danych\n";
    plik << "database_path=" << databasePath << "\n";
    plik << "max_connections=" << maxConnections << "\n";
    plik << "enable_logging=" << (enableLogging ? "true" : "false") << "\n\n";

    plik << "# Konfiguracja raportów\n";
    plik << "default_report_path=" << defaultReportPath << "\n";
    plik << "default_format=" << defaultFormat << "\n";
    plik << "auto_backup_reports=" << (autoBackupReports ? "true" : "false") << "\n\n";

    plik << "# Konfiguracja historii\n";
    plik << "max_history_days=" << maxHistoryDays << "\n";
    plik << "max_history_operations=" << maxHistoryOperations << "\n";
    plik << "auto_cleanup=" << (autoCleanup ? "true" : "false") << "\n\n";

    plik << "# Ceny karnetów (PLN)\n";
    plik << "normalny_miesieczny=" << ceny.normalny_miesieczny << "\n";
    plik << "student_miesieczny=" << ceny.student_miesieczny << "\n";
    plik << "normalny_kwartalny=" << ceny.normalny_kwartalny << "\n";
    plik << "student_kwartalny=" << ceny.student_kwartalny << "\n";
    plik << "normalny_roczny=" << ceny.normalny_roczny << "\n";
    plik << "student_roczny=" << ceny.student_roczny << "\n\n";

    plik << "# Konfiguracja interfejsu\n";
    plik << "language=" << language << "\n";
    plik << "color_output=" << (colorOutput ? "true" : "false") << "\n";
    plik << "page_size=" << pageSize << "\n\n";

    plik << "# Automatyczne punkty przywracania\n";
    plik << "auto_restore_points=" << (autoRestorePoints ? "true" : "false") << "\n";
    plik << "auto_restore_interval=" << autoRestoreInterval << "\n\n";

    plik << "# Konfiguracja alertów\n";
    plik << "karnet_expiry_warning_days=" << karnetExpiryWarningDays << "\n";
    plik << "email_notifications=" << (emailNotifications ? "true" : "false") << "\n";
    plik << "smtp_server=" << smtpServer << "\n";
    plik << "email_from=" << emailFrom << "\n";

    plik.close();
    return true;
}

void SystemConfig::ustawDomyslne() {
    // Konfiguracja bazy danych
    databasePath = "silownia.db";
    maxConnections = 10;
    enableLogging = true;
    
    // Konfiguracja raportów
    defaultReportPath = "raporty/";
    defaultFormat = "HTML";
    autoBackupReports = true;
    
    // Konfiguracja historii
    maxHistoryDays = 90;
    maxHistoryOperations = 1000;
    autoCleanup = true;
    
    // Ceny domyślne
    ceny.normalny_miesieczny = 120.0;
    ceny.student_miesieczny = 80.0;
    ceny.normalny_kwartalny = 300.0;
    ceny.student_kwartalny = 200.0;
    ceny.normalny_roczny = 1000.0;
    ceny.student_roczny = 600.0;
    
    // Konfiguracja interfejsu
    language = "pl";
    colorOutput = true;
    pageSize = 20;
    
    // Automatyczne punkty przywracania
    autoRestorePoints = true;
    autoRestoreInterval = 60;
    
    // Konfiguracja alertów
    karnetExpiryWarningDays = 7;
    emailNotifications = false;
    smtpServer = "";
    emailFrom = "";
}

bool SystemConfig::sprawdzPoprawnosc() const {
    // Sprawdź podstawowe wartości
    if (maxConnections <= 0 || maxConnections > 100) {
        return false;
    }
    
    if (maxHistoryDays <= 0 || maxHistoryDays > 365) {
        return false;
    }
    
    if (maxHistoryOperations <= 0 || maxHistoryOperations > 10000) {
        return false;
    }
    
    if (pageSize <= 0 || pageSize > 1000) {
        return false;
    }
    
    if (autoRestoreInterval <= 0 || autoRestoreInterval > 1440) { // max 24h
        return false;
    }
    
    if (karnetExpiryWarningDays < 0 || karnetExpiryWarningDays > 90) {
        return false;
    }
    
    // Sprawdź ceny karnetów
    if (ceny.normalny_miesieczny <= 0 || ceny.student_miesieczny <= 0 ||
        ceny.normalny_kwartalny <= 0 || ceny.student_kwartalny <= 0 ||
        ceny.normalny_roczny <= 0 || ceny.student_roczny <= 0) {
        return false;
    }
    
    // Sprawdź języki
    if (language != "pl" && language != "en") {
        return false;
    }
    
    return true;
}

std::string SystemConfig::pobierzBleedy() const {
    std::stringstream ss;
    
    if (maxConnections <= 0 || maxConnections > 100) {
        ss << "Nieprawidłowa liczba połączeń: " << maxConnections << "\n";
    }
    
    if (maxHistoryDays <= 0 || maxHistoryDays > 365) {
        ss << "Nieprawidłowa liczba dni historii: " << maxHistoryDays << "\n";
    }
    
    if (maxHistoryOperations <= 0 || maxHistoryOperations > 10000) {
        ss << "Nieprawidłowa liczba operacji historii: " << maxHistoryOperations << "\n";
    }
    
    if (pageSize <= 0 || pageSize > 1000) {
        ss << "Nieprawidłowy rozmiar strony: " << pageSize << "\n";
    }
    
    if (autoRestoreInterval <= 0 || autoRestoreInterval > 1440) {
        ss << "Nieprawidłowy interwał punktów przywracania: " << autoRestoreInterval << "\n";
    }
    
    if (karnetExpiryWarningDays < 0 || karnetExpiryWarningDays > 90) {
        ss << "Nieprawidłowa liczba dni ostrzeżenia: " << karnetExpiryWarningDays << "\n";
    }
    
    if (ceny.normalny_miesieczny <= 0) {
        ss << "Nieprawidłowa cena karnetu normalnego miesięcznego\n";
    }
    
    if (language != "pl" && language != "en") {
        ss << "Nieobsługiwany język: " << language << "\n";
    }
    
    return ss.str();
}