#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QLineEdit>
#include <QTextEdit>
#include <QDateEdit>
#include <QTimeEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include <QTabWidget>
#include "DatabaseManager.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void createTablesIfNotExist();

private slots:
    // === Slots dla zarządzania KLIENTAMI ===
    void odswiezListeKlientow();
    void dodajKlienta();
    void edytujKlienta();
    void usunKlienta();
    void wyczyscFormularzKlienta();
    void wyszukajKlientow();
    void pokazWszystkichKlientow();
    void klientWybrany();  // gdy klikniemy na wiersz w tabeli

    // === Slots dla zarządzania ZAJĘCIAMI ===
    void odswiezListeZajec();
    void dodajZajecia();
    void edytujZajecia();
    void usunZajecia();
    void wyczyscFormularzZajec();
    void wyszukajZajecia();
    void pokazWszystkieZajecia();
    void filtrujZajeciaPoData();
    void zajeciaWybrane();  // gdy klikniemy na wiersz w tabeli zajęć

    // === Slots dla zarządzania REZERWACJAMI ===
    void odswiezListeRezerwacji();
    void dodajRezerwacje();
    void anulujRezerwacje();
    void wyczyscFormularzRezerwacji();
    void filtrujRezerwacje();
    void rezerwacjaWybrana();  // gdy klikniemy na wiersz w tabeli rezerwacji
    void zajeciaRezerwacjiWybrane();  // gdy wybierzemy zajęcia w comboBox
    void pokazStatystyki();
    void pokazAktywnychKlientow();

    // === Slots dla zarządzania KARNETAMI ===
    void odswiezListeKarnetow();
    void dodajKarnet();
    void edytujKarnet();
    void usunKarnet();
    void wyczyscFormularzKarnetu();
    void filtrujKarnety();
    void karnetWybrany();  // gdy klikniemy na wiersz w tabeli karnetów
    void klientKarnetuWybrany();  // gdy wybierzemy klienta w comboBox
    void pokazStatystykiKarnetow();
    void pokazWygasajaceKarnety();

    // === Slots dla menu ===
    void zamknijAplikacje();
    void oProgramie();

private:
    Ui::MainWindow *ui;

    // === Zmienne pomocnicze dla KLIENTÓW ===
    int aktualnieEdytowanyKlientId;  // -1 gdy dodajemy nowego, >0 gdy edytujemy

    // === Zmienne pomocnicze dla ZAJĘĆ ===
    int aktualnieEdytowaneZajeciaId; // -1 gdy dodajemy nowe, >0 gdy edytujemy

    // === Zmienne pomocnicze dla REZERWACJI ===
    int aktualnieWybranaRezerwacjaId; // -1 gdy nic nie wybrane, >0 gdy wybrane

    // === Zmienne pomocnicze dla KARNETÓW ===
    int aktualnieEdytowanyKarnetId; // -1 gdy dodajemy nowy, >0 gdy edytujemy

    // === Metody pomocnicze - OGÓLNE ===
    void setupUI();                    // Konfiguracja UI po uruchomieniu
    void setupConnections();           // Połączenia sygnałów ze slotami

    // === Metody pomocnicze - KLIENCI ===
    void setupTableKlienci();                                    // Konfiguracja tabeli klientów
    void zaladujKlientowDoTabeli(const QList<Klient>& klienci);  // Załaduj klientów do tabeli
    void zaladujKlientaDoFormularza(const Klient& klient);       // Załaduj dane klienta do formularza
    Klient pobierzDaneKlientaZFormularza();                      // Pobierz dane z formularza klienta
    bool walidujFormularzKlienta();                              // Sprawdź czy formularz klienta jest poprawny
    void ustawTrybDodawaniaKlienta();                            // Ustaw UI w tryb dodawania nowego klienta
    void ustawTrybEdycjiKlienta();                               // Ustaw UI w tryb edycji klienta
    void aktualizujLicznikKlientow();                            // Aktualizuj wyświetlaną liczbę klientów

    // === Metody pomocnicze - ZAJĘCIA ===
    void setupTableZajecia();                                    // Konfiguracja tabeli zajęć
    void zaladujZajeciaDoTabeli(const QList<Zajecia>& zajecia);  // Załaduj zajęcia do tabeli
    void zaladujZajeciaDoFormularza(const Zajecia& zajecia);     // Załaduj dane zajęć do formularza
    Zajecia pobierzDaneZajecZFormularza();                       // Pobierz dane z formularza zajęć
    bool walidujFormularzZajec();                                // Sprawdź czy formularz zajęć jest poprawny
    void ustawTrybDodawaniaZajec();                              // Ustaw UI w tryb dodawania nowych zajęć
    void ustawTrybEdycjiZajec();                                 // Ustaw UI w tryb edycji zajęć
    void aktualizujLicznikZajec();                               // Aktualizuj wyświetlaną liczbę zajęć

    // === Metody pomocnicze - REZERWACJE ===
    void setupTableRezerwacje();                                           // Konfiguracja tabeli rezerwacji
    void zaladujRezerwacjeDoTabeli(const QList<Rezerwacja>& rezerwacje);   // Załaduj rezerwacje do tabeli
    void zaladujKlientowDoComboBox();                                      // Załaduj klientów do ComboBox
    void zaladujZajeciaDoComboBox();                                       // Załaduj dostępne zajęcia do ComboBox
    void aktualizujInfoZajec();                                            // Aktualizuj informacje o wybranych zajęciach
    bool walidujFormularzRezerwacji();                                     // Sprawdź czy formularz rezerwacji jest poprawny
    void wyczyscInfoZajec();                                               // Wyczyść informacje o zajęciach
    void aktualizujLicznikRezerwacji();                                    // Aktualizuj wyświetlaną liczbę rezerwacji
    void aktualizujPrzyciskAnuluj();                                       // Aktualizuj przycisk anulowania

    // === Metody pomocnicze - KARNETY ===
    void setupTableKarnety();                                           // Konfiguracja tabeli karnetów
    void zaladujKarnetyDoTabeli(const QList<Karnet>& karnety);          // Załaduj karnety do tabeli
    void zaladujKarnetDoFormularza(const Karnet& karnet);               // Załaduj dane karnetu do formularza
    void zaladujKlientowDoComboBoxKarnetu();                            // Załaduj klientów do ComboBox karnetów
    Karnet pobierzDaneKarnetuZFormularza();                             // Pobierz dane z formularza karnetu
    bool walidujFormularzKarnetu();                                     // Sprawdź czy formularz karnetu jest poprawny
    void ustawTrybDodawaniaKarnetu();                                   // Ustaw UI w tryb dodawania nowego karnetu
    void ustawTrybEdycjiKarnetu();                                      // Ustaw UI w tryb edycji karnetu
    void aktualizujLicznikKarnetow();                                   // Aktualizuj wyświetlaną liczbę karnetów
    void aktualizujInfoKlienta();                                       // Aktualizuj informacje o wybranym kliencie
    void wyczyscInfoKlienta();                                          // Wyczyść informacje o kliencie
    void obliczCeneKarnetu();                                           // Oblicz cenę karnetu na podstawie typu

    // === Metody ogólne ===
    void pokazKomunikat(const QString& tytul, const QString& tresc, QMessageBox::Icon typ = QMessageBox::Information);
};

#endif // MAINWINDOW_H
