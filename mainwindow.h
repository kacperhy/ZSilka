#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QLineEdit>
#include <QTextEdit>
#include <QDateEdit>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
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
    // === Slots dla zarządzania klientami ===
    void odswiezListeKlientow();
    void dodajKlienta();
    void edytujKlienta();
    void usunKlienta();
    void wyczyscFormularz();

    // === Slots dla wyszukiwania ===
    void wyszukajKlientow();
    void pokazWszystkichKlientow();

    // === Slots dla tabeli ===
    void klientWybrany();  // gdy klikniemy na wiersz w tabeli

    // === Slots dla menu ===
    void zamknijAplikacje();
    void oProgramie();

private:
    Ui::MainWindow *ui;

    // === Zmienne pomocnicze ===
    int aktualnieEdytowanyKlientId;  // -1 gdy dodajemy nowego, >0 gdy edytujemy

    // === Metody pomocnicze ===
    void setupUI();                    // Konfiguracja UI po uruchomieniu
    void setupConnections();           // Połączenia sygnałów ze slotami
    void setupTable();                 // Konfiguracja tabeli klientów

    void zaladujKlientowDoTabeli(const QList<Klient>& klienci);  // Załaduj klientów do tabeli
    void zaladujKlientaDoFormularza(const Klient& klient);       // Załaduj dane klienta do formularza
    Klient pobierzDaneZFormularza();                            // Pobierz dane z formularza

    bool walidujFormularz();           // Sprawdź czy formularz jest poprawnie wypełniony
    void ustawTrybDodawania();         // Ustaw UI w tryb dodawania nowego klienta
    void ustawTrybEdycji();            // Ustaw UI w tryb edycji klienta

    void pokazKomunikat(const QString& tytul, const QString& tresc, QMessageBox::Icon typ = QMessageBox::Information);
    void aktualizujLicznikKlientow();   // Aktualizuj wyświetlaną liczbę klientów
};

#endif // MAINWINDOW_H
