#include "eksport_danych.h"
#include <fstream>
#include <sstream>
#include <algorithm>

EksportDanych::EksportDanych(UslugiKlienta& uslugiKlienta, UslugiKarnetu& uslugiKarnetu, UslugiZajec& uslugiZajec)
    : uslugiKlienta(uslugiKlienta), uslugiKarnetu(uslugiKarnetu), uslugiZajec(uslugiZajec) {
}

std::string EksportDanych::escapujCSV(const std::string& str) {
    if (str.find(',') != std::string::npos || str.find('"') != std::string::npos || 
        str.find('\n') != std::string::npos || str.find('\r') != std::string::npos) {
        
        std::string escaped = str;
        size_t pos = 0;
        while ((pos = escaped.find('"', pos)) != std::string::npos) {
            escaped.replace(pos, 1, "\"\"");
            pos += 2;
        }
        
        return "\"" + escaped + "\"";
    }
    
    return str;
}

bool EksportDanych::eksportujKlientowDoCSV(const std::string& sciezkaPliku) {
    std::ofstream plik(sciezkaPliku);
    if (!plik.is_open()) {
        throw WyjatekEksportu("Nie można otworzyć pliku: " + sciezkaPliku);
    }
    
    plik << "id,imie,nazwisko,email,telefon,data_urodzenia,data_rejestracji,uwagi\n";
    
    auto klienci = uslugiKlienta.pobierzWszystkichKlientow();
    for (const auto& klient : klienci) {
        plik << klient.pobierzId() << ","
             << escapujCSV(klient.pobierzImie()) << ","
             << escapujCSV(klient.pobierzNazwisko()) << ","
             << escapujCSV(klient.pobierzEmail()) << ","
             << escapujCSV(klient.pobierzTelefon()) << ","
             << escapujCSV(klient.pobierzDateUrodzenia()) << ","
             << escapujCSV(klient.pobierzDateRejestracji()) << ","
             << escapujCSV(klient.pobierzUwagi()) << "\n";
    }
    
    plik.close();
    return true;
}

bool EksportDanych::eksportujKarnetyDoCSV(const std::string& sciezkaPliku) {
    std::ofstream plik(sciezkaPliku);
    if (!plik.is_open()) {
        throw WyjatekEksportu("Nie można otworzyć pliku: " + sciezkaPliku);
    }
    
    plik << "id,id_klienta,typ,data_rozpoczecia,data_zakonczenia,cena,czy_aktywny\n";
    
    auto karnety = uslugiKarnetu.pobierzWszystkieKarnety();
    for (const auto& karnet : karnety) {
        plik << karnet.pobierzId() << ","
             << karnet.pobierzIdKlienta() << ","
             << escapujCSV(karnet.pobierzTyp()) << ","
             << escapujCSV(karnet.pobierzDateRozpoczecia()) << ","
             << escapujCSV(karnet.pobierzDateZakonczenia()) << ","
             << karnet.pobierzCene() << ","
             << (karnet.pobierzCzyAktywny() ? "1" : "0") << "\n";
    }
    
    plik.close();
    return true;
}

bool EksportDanych::eksportujZajeciaDoCSV(const std::string& sciezkaPliku) {
    std::ofstream plik(sciezkaPliku);
    if (!plik.is_open()) {
        throw WyjatekEksportu("Nie można otworzyć pliku: " + sciezkaPliku);
    }
    
    plik << "id,nazwa,trener,maks_uczestnikow,data,czas,czas_trwania,opis\n";
    
    auto zajecia = uslugiZajec.pobierzWszystkieZajecia();
    for (const auto& zajecie : zajecia) {
        plik << zajecie.pobierzId() << ","
             << escapujCSV(zajecie.pobierzNazwe()) << ","
             << escapujCSV(zajecie.pobierzTrenera()) << ","
             << zajecie.pobierzMaksUczestnikow() << ","
             << escapujCSV(zajecie.pobierzDate()) << ","
             << escapujCSV(zajecie.pobierzCzas()) << ","
             << zajecie.pobierzCzasTrwania() << ","
             << escapujCSV(zajecie.pobierzOpis()) << "\n";
    }
    
    plik.close();
    return true;
}

bool EksportDanych::eksportujRezerwacjeDoCSV(const std::string& sciezkaPliku) {
    std::ofstream plik(sciezkaPliku);
    if (!plik.is_open()) {
        throw WyjatekEksportu("Nie można otworzyć pliku: " + sciezkaPliku);
    }
    
    plik << "id,id_klienta,id_zajec,data_rezerwacji,status\n";
    
    auto rezerwacje = uslugiZajec.pobierzWszystkieRezerwacje();
    for (const auto& rezerwacja : rezerwacje) {
        plik << rezerwacja.pobierzId() << ","
             << rezerwacja.pobierzIdKlienta() << ","
             << rezerwacja.pobierzIdZajec() << ","
             << escapujCSV(rezerwacja.pobierzDateRezerwacji()) << ","
             << escapujCSV(rezerwacja.pobierzStatus()) << "\n";
    }
    
    plik.close();
    return true;
}

std::string EksportDanych::escapujJSON(const std::string& str) {
    std::string escaped;
    escaped.reserve(str.size());
    
    for (char c : str) {
        switch (c) {
            case '"': escaped += "\\\""; break;
            case '\\': escaped += "\\\\"; break;
            case '\b': escaped += "\\b"; break;
            case '\f': escaped += "\\f"; break;
            case '\n': escaped += "\\n"; break;
            case '\r': escaped += "\\r"; break;
            case '\t': escaped += "\\t"; break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    char buf[7];
                    snprintf(buf, sizeof(buf), "\\u%04x", c);
                    escaped += buf;
                } else {
                    escaped += c;
                }
        }
    }
    
    return escaped;
}

std::string EksportDanych::klientDoJSON(const Klient& klient) {
    std::stringstream ss;
    
    ss << "{";
    ss << "\"id\":" << klient.pobierzId() << ",";
    ss << "\"imie\":\"" << escapujJSON(klient.pobierzImie()) << "\",";
    ss << "\"nazwisko\":\"" << escapujJSON(klient.pobierzNazwisko()) << "\",";
    ss << "\"email\":\"" << escapujJSON(klient.pobierzEmail()) << "\",";
    ss << "\"telefon\":\"" << escapujJSON(klient.pobierzTelefon()) << "\",";
    ss << "\"dataUrodzenia\":\"" << escapujJSON(klient.pobierzDateUrodzenia()) << "\",";
    ss << "\"dataRejestracji\":\"" << escapujJSON(klient.pobierzDateRejestracji()) << "\",";
    ss << "\"uwagi\":\"" << escapujJSON(klient.pobierzUwagi()) << "\"";
    ss << "}";
    
    return ss.str();
}

std::string EksportDanych::karnetDoJSON(const Karnet& karnet) {
    std::stringstream ss;
    
    ss << "{";
    ss << "\"id\":" << karnet.pobierzId() << ",";
    ss << "\"idKlienta\":" << karnet.pobierzIdKlienta() << ",";
    ss << "\"typ\":\"" << escapujJSON(karnet.pobierzTyp()) << "\",";
    ss << "\"dataRozpoczecia\":\"" << escapujJSON(karnet.pobierzDateRozpoczecia()) << "\",";
    ss << "\"dataZakonczenia\":\"" << escapujJSON(karnet.pobierzDateZakonczenia()) << "\",";
    ss << "\"cena\":" << karnet.pobierzCene() << ",";
    ss << "\"czyAktywny\":" << (karnet.pobierzCzyAktywny() ? "true" : "false");
    ss << "}";
    
    return ss.str();
}

std::string EksportDanych::zajeciaDoJSON(const Zajecia& zajecia) {
    std::stringstream ss;
    
    ss << "{";
    ss << "\"id\":" << zajecia.pobierzId() << ",";
    ss << "\"nazwa\":\"" << escapujJSON(zajecia.pobierzNazwe()) << "\",";
    ss << "\"trener\":\"" << escapujJSON(zajecia.pobierzTrenera()) << "\",";
    ss << "\"maksUczestnikow\":" << zajecia.pobierzMaksUczestnikow() << ",";
    ss << "\"data\":\"" << escapujJSON(zajecia.pobierzDate()) << "\",";
    ss << "\"czas\":\"" << escapujJSON(zajecia.pobierzCzas()) << "\",";
    ss << "\"czasTrwania\":" << zajecia.pobierzCzasTrwania() << ",";
    ss << "\"opis\":\"" << escapujJSON(zajecia.pobierzOpis()) << "\"";
    ss << "}";
    
    return ss.str();
}

std::string EksportDanych::rezerwacjaDoJSON(const Rezerwacja& rezerwacja) {
    std::stringstream ss;
    
    ss << "{";
    ss << "\"id\":" << rezerwacja.pobierzId() << ",";
    ss << "\"idKlienta\":" << rezerwacja.pobierzIdKlienta() << ",";
    ss << "\"idZajec\":" << rezerwacja.pobierzIdZajec() << ",";
    ss << "\"dataRezerwacji\":\"" << escapujJSON(rezerwacja.pobierzDateRezerwacji()) << "\",";
    ss << "\"status\":\"" << escapujJSON(rezerwacja.pobierzStatus()) << "\"";
    ss << "}";
    
    return ss.str();
}

bool EksportDanych::eksportujKlientowDoJSON(const std::string& sciezkaPliku) {
    std::ofstream plik(sciezkaPliku);
    if (!plik.is_open()) {
        throw WyjatekEksportu("Nie można otworzyć pliku: " + sciezkaPliku);
    }
    
    auto klienci = uslugiKlienta.pobierzWszystkichKlientow();
    
    plik << "{\n";
    plik << "  \"klienci\": [\n";
    
    for (size_t i = 0; i < klienci.size(); ++i) {
        plik << "    " << klientDoJSON(klienci[i]);
        if (i < klienci.size() - 1) {
            plik << ",";
        }
        plik << "\n";
    }
    
    plik << "  ]\n";
    plik << "}\n";
    
    plik.close();
    return true;
}

bool EksportDanych::eksportujKarnetyDoJSON(const std::string& sciezkaPliku) {
    std::ofstream plik(sciezkaPliku);
    if (!plik.is_open()) {
        throw WyjatekEksportu("Nie można otworzyć pliku: " + sciezkaPliku);
    }
    
    auto karnety = uslugiKarnetu.pobierzWszystkieKarnety();
    
    plik << "{\n";
    plik << "  \"karnety\": [\n";
    
    for (size_t i = 0; i < karnety.size(); ++i) {
        plik << "    " << karnetDoJSON(karnety[i]);
        if (i < karnety.size() - 1) {
            plik << ",";
        }
        plik << "\n";
    }
    
    plik << "  ]\n";
    plik << "}\n";
    
    plik.close();
    return true;
}

bool EksportDanych::eksportujZajeciaDoJSON(const std::string& sciezkaPliku) {
    std::ofstream plik(sciezkaPliku);
    if (!plik.is_open()) {
        throw WyjatekEksportu("Nie można otworzyć pliku: " + sciezkaPliku);
    }
    
    auto zajecia = uslugiZajec.pobierzWszystkieZajecia();
    
    plik << "{\n";
    plik << "  \"zajecia\": [\n";
    
    for (size_t i = 0; i < zajecia.size(); ++i) {
        plik << "    " << zajeciaDoJSON(zajecia[i]);
        if (i < zajecia.size() - 1) {
            plik << ",";
        }
        plik << "\n";
    }
    
    plik << "  ]\n";
    plik << "}\n";
    
    plik.close();
    return true;
}

bool EksportDanych::eksportujRezerwacjeDoJSON(const std::string& sciezkaPliku) {
    std::ofstream plik(sciezkaPliku);
    if (!plik.is_open()) {
        throw WyjatekEksportu("Nie można otworzyć pliku: " + sciezkaPliku);
    }
    
    auto rezerwacje = uslugiZajec.pobierzWszystkieRezerwacje();
    
    plik << "{\n";
    plik << "  \"rezerwacje\": [\n";
    
    for (size_t i = 0; i < rezerwacje.size(); ++i) {
        plik << "    " << rezerwacjaDoJSON(rezerwacje[i]);
        if (i < rezerwacje.size() - 1) {
            plik << ",";
        }
        plik << "\n";
    }
    
    plik << "  ]\n";
    plik << "}\n";
    
    plik.close();
    return true;
}