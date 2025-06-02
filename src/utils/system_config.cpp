#include "../config/system_config.h"
#include <fstream>
#include <sstream>
#include <iostream>

bool SystemConfig::wczytajZPliku(const std::string& sciezka) {
    std::ifstream plik(sciezka);
    if (!plik.is_open()) {
        std::cerr << "Ostrze¿enie: Nie mo¿na otworzyæ pliku konfiguracji: " << sciezka << std::endl;
        std::cerr << "U¿ywam domyœlnych ustawieñ." << std::endl;
        ustawDomyslne();
        return false;
    }

    std::string linia;
    std::string aktualnaSekcja = "";

    while (std::getline(plik, linia)) {
        // Usuñ bia³e znaki na pocz¹tku i koñcu
        linia.erase(0, linia.find_first_not_of(" \t"));
        linia.erase(linia.find_last_not_of(" \t") + 1);

        // Pomiñ puste linie i komentarze
        if (linia.empty() || linia[0] == '#' || linia[0] == ';') {
            continue;
        }

        // SprawdŸ czy to sekcja
        if (linia[0] == '[' && linia.back() == ']') {
            aktualnaSekcja = linia.substr(1, linia.length() - 2);
            continue;
        }

        // Przetwórz parê klucz=wartoœæ
        size_t pozycjaRownania = linia.find('=');
        if (pozycjaRownania != std::string::npos) {
            std::string klucz = linia.substr(0, pozycjaRownania);
            std::string wartosc = linia.substr(pozycjaRownania + 1);

            // Usuñ bia³e znaki
            klucz.erase(0, klucz.find_first_not_of(" \t"));
            klucz.erase(klucz.find_last_not_of(" \t") + 1);
            wartosc.erase(0, wartosc.find_first_not_of(" \t"));
            wartosc.erase(wartosc.find_last_not_of(" \t") + 1);

            // Przetwórz ustawienia na podstawie sekcji i klucza
            if (aktualnaSekcja == "Database") {
                if (klucz == "path") databasePath = wartosc;
                else if (klucz == "maxConnections") maxConnections = std::stoi(wartosc);
                else if (klucz == "enableLogging") enableLogging = (wartosc == "true");
            }
            else if (aktualnaSekcja == "Reports") {
                if (klucz == "defaultPath") defaultReportPath = wartosc;
                else if (klucz == "defaultFormat") defaultFormat = wartosc;
                else if (klucz == "autoBackup") autoBackupReports = (wartosc == "true");
            }
            else if (aktualnaSekcja == "History") {
                if (klucz == "maxDays") maxHistoryDays = std::stoi(wartosc);
                else if (klucz == "maxOperations") maxHistoryOperations = std::stoi(wartosc);
                else if (klucz == "autoCleanup") autoCleanup = (wartosc == "true");
            }
            else if (aktualnaSekcja == "Prices") {
                if (klucz == "normalny_miesieczny") ceny.normalny_miesieczny = std::stod(wartosc);
                else if (klucz == "student_miesieczny") ceny.student_miesieczny = std::stod(wartosc);
                else if (klucz == "normalny_kwartalny") ceny.normalny_kwartalny = std::stod(wartosc);
                else if (klucz == "student_kwartalny") ceny.student_kwartalny = std::stod(wartosc);
                else if (klucz == "normalny_roczny") ceny.normalny_roczny = std::stod(wartosc);
                else if (klucz == "student_roczny") ceny.student_roczny = std::stod(wartosc);
            }
            else if (aktualnaSekcja == "Interface") {
                if (klucz == "language") language = wartosc;
                else if (klucz == "colorOutput") colorOutput = (wartosc == "true");
                else if (klucz == "pageSize") pageSize = std::stoi(wartosc);
            }
            else if (aktualnaSekcja == "Alerts") {
                if (klucz == "karnetExpiryWarningDays") karnetExpiryWarningDays = std::stoi(wartosc);
                else if (klucz == "emailNotifications") emailNotifications = (wartosc == "true");
                else if (klucz == "smtpServer") smtpServer = wartosc;
                else if (klucz == "emailFrom") emailFrom = wartosc;
            }
            else if (aktualnaSekcja == "RestorePoints") {
                if (klucz == "autoCreate") autoRestorePoints = (wartosc == "true");
                else if (klucz == "autoCreateInterval") autoRestoreInterval = std::stoi(wartosc);
            }
        }
    }

    plik.close();
    return true;
}

bool SystemConfig::zapiszDoPliku(const std::string& sciezka) {
    std::ofstream plik(sciezka);
    if (!plik.is_open()) {
        std::cerr << "B³¹d: Nie mo¿na utworzyæ pliku konfiguracji: " << sciezka << std::endl;
        return false;
    }

    plik << "# =================================================================\n";
    plik << "#           KONFIGURACJA SYSTEMU ZARZ¥DZANIA SI£OWNI¥\n";
    plik << "# =================================================================\n\n";

    plik << "[Database]\n";
    plik << "path=" << databasePath << "\n";
    plik << "maxConnections=" << maxConnections << "\n";
    plik << "enableLogging=" << (enableLogging ? "true" : "false") << "\n\n";

    plik << "[Reports]\n";
    plik << "defaultPath=" << defaultReportPath << "\n";
    plik << "defaultFormat=" << defaultFormat << "\n";
    plik << "autoBackup=" << (autoBackupReports ? "true" : "false") << "\n\n";

    plik << "[History]\n";
    plik << "maxDays=" << maxHistoryDays << "\n";
    plik << "maxOperations=" << maxHistoryOperations << "\n";
    plik << "autoCleanup=" << (autoCleanup ? "true" : "false") << "\n\n";

    plik << "[Prices]\n";
    plik << "normalny_miesieczny=" << ceny.normalny_miesieczny << "\n";
    plik << "student_miesieczny=" << ceny.student_miesieczny << "\n";
    plik << "normalny_kwartalny=" << ceny.normalny_kwartalny << "\n";
    plik << "student_kwartalny=" << ceny.student_kwartalny << "\n";
    plik << "normalny_roczny=" << ceny.normalny_roczny << "\n";
    plik << "student_roczny=" << ceny.student_roczny << "\n\n";

    plik << "[Interface]\n";
    plik << "language=" << language << "\n";
    plik << "colorOutput=" << (colorOutput ? "true" : "false") << "\n";
    plik << "pageSize=" << pageSize << "\n\n";

    plik << "[Alerts]\n";
    plik << "karnetExpiryWarningDays=" << karnetExpiryWarningDays << "\n";
    plik << "emailNotifications=" << (emailNotifications ? "true" : "false") << "\n";
    plik << "smtpServer=" << smtpServer << "\n";
    plik << "emailFrom=" << emailFrom << "\n\n";

    plik << "[RestorePoints]\n";
    plik << "autoCreate=" << (autoRestorePoints ? "true" : "false") << "\n";
    plik << "autoCreateInterval=" << autoRestoreInterval << "\n\n";

    plik.close();
    return true;
}

void SystemConfig::ustawDomyslne() {
    // Ustawienia bazy danych
    databasePath = "silownia.db";
    maxConnections = 10;
    enableLogging = true;

    // Ustawienia raportów
    defaultReportPath = "raporty/";
    defaultFormat = "HTML";
    autoBackupReports = true;

    // Ustawienia historii
    maxHistoryDays = 90;
    maxHistoryOperations = 1000;
    autoCleanup = true;

    // Ceny karnetów
    ceny.normalny_miesieczny = 120.0;
    ceny.student_miesieczny = 80.0;
    ceny.normalny_kwartalny = 300.0;
    ceny.student_kwartalny = 200.0;
    ceny.normalny_roczny = 1000.0;
    ceny.student_roczny = 600.0;

    // Interfejs
    language = "pl";
    colorOutput = true;
    pageSize = 20;

    // Alerty
    karnetExpiryWarningDays = 7;
    emailNotifications = false;
    smtpServer = "";
    emailFrom = "";

    // Punkty przywracania
    autoRestorePoints = true;
    autoRestoreInterval = 60;
}

bool SystemConfig::sprawdzPoprawnosc() const {
    if (databasePath.empty()) return false;
    if (maxConnections <= 0) return false;
    if (maxHistoryDays <= 0) return false;
    if (maxHistoryOperations <= 0) return false;
    if (pageSize <= 0) return false;
    if (karnetExpiryWarningDays < 0) return false;
    if (autoRestoreInterval <= 0) return false;

    // SprawdŸ ceny
    if (ceny.normalny_miesieczny <= 0) return false;
    if (ceny.student_miesieczny <= 0) return false;
    if (ceny.normalny_kwartalny <= 0) return false;
    if (ceny.student_kwartalny <= 0) return false;
    if (ceny.normalny_roczny <= 0) return false;
    if (ceny.student_roczny <= 0) return false;

    return true;
}

std::string SystemConfig::pobierzBleady() const {
    std::stringstream bledy;

    if (databasePath.empty()) {
        bledy << "- Œcie¿ka do bazy danych nie mo¿e byæ pusta\n";
    }

    if (maxConnections <= 0) {
        bledy << "- Maksymalna liczba po³¹czeñ musi byæ wiêksza od 0\n";
    }

    if (maxHistoryDays <= 0) {
        bledy << "- Maksymalny czas przechowywania historii musi byæ wiêkszy od 0\n";
    }

    if (maxHistoryOperations <= 0) {
        bledy << "- Maksymalna liczba operacji w historii musi byæ wiêksza od 0\n";
    }

    if (pageSize <= 0) {
        bledy << "- Rozmiar strony musi byæ wiêkszy od 0\n";
    }

    if (karnetExpiryWarningDays < 0) {
        bledy << "- Liczba dni ostrze¿enia przed wygaœniêciem karnetu nie mo¿e byæ ujemna\n";
    }

    if (autoRestoreInterval <= 0) {
        bledy << "- Interwa³ automatycznych punktów przywracania musi byæ wiêkszy od 0\n";
    }

    // SprawdŸ ceny
    if (ceny.normalny_miesieczny <= 0) {
        bledy << "- Cena karnetu miesiêcznego normalnego musi byæ wiêksza od 0\n";
    }

    if (ceny.student_miesieczny <= 0) {
        bledy << "- Cena karnetu miesiêcznego studenckiego musi byæ wiêksza od 0\n";
    }

    if (ceny.normalny_kwartalny <= 0) {
        bledy << "- Cena karnetu kwartalnego normalnego musi byæ wiêksza od 0\n";
    }

    if (ceny.student_kwartalny <= 0) {
        bledy << "- Cena karnetu kwartalnego studenckiego musi byæ wiêksza od 0\n";
    }

    if (ceny.normalny_roczny <= 0) {
        bledy << "- Cena karnetu rocznego normalnego musi byæ wiêksza od 0\n";
    }

    if (ceny.student_roczny <= 0) {
        bledy << "- Cena karnetu rocznego studenckiego musi byæ wiêksza od 0\n";
    }

    return bledy.str();
}