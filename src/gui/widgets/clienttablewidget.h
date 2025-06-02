// src/gui/widgets/clienttablewidget.h
#ifndef CLIENTTABLEWIDGET_H
#define CLIENTTABLEWIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QHeaderView>
#include <QContextMenuEvent>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QApplication>
#include <QClipboard>

// Istniejące klasy z oryginalnej aplikacji
#include "models/klient.h"
#include "services/uslugi_klienta.h"

class ClientTableWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ClientTableWidget(QWidget *parent = nullptr);
    ~ClientTableWidget();

    // Inicjalizacja z serwisem (z oryginalnej aplikacji)
    void initializeServices(UslugiKlienta* uslugiKlienta);

public slots:
    void refresh();
    void addClient();
    void editClient();
    void deleteClient();
    void searchClients();
    void clearSearch();
    void exportClients();

signals:
    void clientAdded(const Klient& klient);
    void clientEdited(const Klient& klient);
    void clientDeleted(int clientId);

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

private slots:
    void onTableSelectionChanged();
    void onTableDoubleClicked(int row, int column);
    void onSearchTextChanged(const QString& text);
    void onFilterChanged();
    void copyToClipboard();
    void selectAll();

private:
    void setupUI();
    void setupTable();
    void setupToolbar();
    void setupConnections();
    void createContextMenu();
    
    void loadClientData();
    void populateTable(const std::vector<Klient>& klienci);
    void updateClientCount();
    void filterTable();
    
    Klient* getSelectedClient();
    QList<Klient*> getSelectedClients();
    void selectClientById(int clientId);
    
    QString formatDate(const std::string& dateStr);
    QTableWidgetItem* createTableItem(const QString& text);

private:
    // Serwis klientów (z oryginalnej aplikacji)
    UslugiKlienta* m_uslugiKlienta;
    
    // Dane
    std::vector<Klient> m_allClients;
    std::vector<Klient> m_filteredClients;
    
    // UI Components
    QVBoxLayout* m_mainLayout;
    QHBoxLayout* m_toolbarLayout;
    
    // Toolbar
    QLineEdit* m_searchEdit;
    QComboBox* m_filterCombo;
    QPushButton* m_addButton;
    QPushButton* m_editButton;
    QPushButton* m_deleteButton;
    QPushButton* m_refreshButton;
    QPushButton* m_exportButton;
    QLabel* m_countLabel;
    
    // Tabela
    QTableWidget* m_tableWidget;
    
    // Context menu
    QMenu* m_contextMenu;
    QAction* m_editAction;
    QAction* m_deleteAction;
    QAction* m_copyAction;
    QAction* m_selectAllAction;
    QAction* m_refreshAction;
    
    // Kolumny tabeli
    enum ColumnIndex {
        COL_ID = 0,
        COL_IMIE,
        COL_NAZWISKO,
        COL_EMAIL,
        COL_TELEFON,
        COL_DATA_URODZENIA,
        COL_DATA_REJESTRACJI,
        COL_UWAGI,
        COL_COUNT
    };
    
    // Filtry
    enum FilterType {
        FILTER_ALL = 0,
        FILTER_RECENT,          // Ostatnio zarejestrowani
        FILTER_THIS_MONTH,      // W tym miesiącu
        FILTER_HAS_EMAIL,       // Mają email
        FILTER_HAS_PHONE,       // Mają telefon
        FILTER_NO_CONTACT       // Brak danych kontaktowych
    };
    
    // Stałe
    static const int RECENT_DAYS = 30;
    bool m_servicesInitialized;
};

#endif // CLIENTTABLEWIDGET_H