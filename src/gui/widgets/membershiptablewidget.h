// src/gui/widgets/membershiptablewidget.h
#ifndef MEMBERSHIPTABLEWIDGET_H
#define MEMBERSHIPTABLEWIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QDateEdit>
#include <QHeaderView>
#include <QContextMenuEvent>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QApplication>
#include <QClipboard>
#include <QProgressBar>

// Istniejące klasy z oryginalnej aplikacji
#include "models/karnet.h"
#include "models/klient.h"
#include "services/uslugi_karnetu.h"
#include "services/uslugi_klienta.h"

class MembershipTableWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MembershipTableWidget(QWidget *parent = nullptr);
    ~MembershipTableWidget();

    // Inicjalizacja z serwisami (z oryginalnej aplikacji)
    void initializeServices(UslugiKarnetu* uslugiKarnetu, UslugiKlienta* uslugiKlienta);

public slots:
    void refresh();
    void addMembership();
    void editMembership();
    void deleteMembership();
    void renewMembership();
    void deactivateMembership();
    void searchMemberships();
    void clearSearch();
    void exportMemberships();

signals:
    void membershipAdded(const Karnet& karnet);
    void membershipEdited(const Karnet& karnet);
    void membershipDeleted(int membershipId);

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

private slots:
    void onTableSelectionChanged();
    void onTableDoubleClicked(int row, int column);
    void onSearchTextChanged(const QString& text);
    void onFilterChanged();
    void onDateFilterChanged();
    void copyToClipboard();
    void selectAll();

private:
    void setupUI();
    void setupTable();
    void setupToolbar();
    void setupConnections();
    void createContextMenu();
    
    void loadMembershipData();
    void populateTable(const std::vector<Karnet>& karnety);
    void updateMembershipCount();
    void filterTable();
    
    Karnet* getSelectedMembership();
    QList<Karnet*> getSelectedMemberships();
    void selectMembershipById(int membershipId);
    
    QString formatDate(const std::string& dateStr);
    QString formatPrice(double price);
    QString getClientName(int clientId);
    QTableWidgetItem* createTableItem(const QString& text);
    QTableWidgetItem* createStatusItem(const Karnet& karnet);

private:
    // Serwisy karnetów i klientów (z oryginalnej aplikacji)
    UslugiKarnetu* m_uslugiKarnetu;
    UslugiKlienta* m_uslugiKlienta;
    
    // Dane
    std::vector<Karnet> m_allMemberships;
    std::vector<Karnet> m_filteredMemberships;
    
    // UI Components
    QVBoxLayout* m_mainLayout;
    QHBoxLayout* m_toolbarLayout;
    
    // Toolbar
    QLineEdit* m_searchEdit;
    QComboBox* m_filterCombo;
    QComboBox* m_typeFilterCombo;
    QDateEdit* m_fromDateEdit;
    QDateEdit* m_toDateEdit;
    QPushButton* m_addButton;
    QPushButton* m_editButton;
    QPushButton* m_deleteButton;
    QPushButton* m_renewButton;
    QPushButton* m_deactivateButton;
    QPushButton* m_refreshButton;
    QPushButton* m_exportButton;
    QLabel* m_countLabel;
    QLabel* m_revenueLabel;
    
    // Tabela
    QTableWidget* m_tableWidget;
    
    // Context menu
    QMenu* m_contextMenu;
    QAction* m_editAction;
    QAction* m_deleteAction;
    QAction* m_renewAction;
    QAction* m_deactivateAction;
    QAction* m_copyAction;
    QAction* m_selectAllAction;
    QAction* m_refreshAction;
    
    // Kolumny tabeli
    enum ColumnIndex {
        COL_ID = 0,
        COL_CLIENT_NAME,
        COL_CLIENT_ID,
        COL_TYPE,
        COL_START_DATE,
        COL_END_DATE,
        COL_PRICE,
        COL_STATUS,
        COL_DAYS_LEFT,
        COL_COUNT
    };
    
    // Filtry
    enum FilterType {
        FILTER_ALL = 0,
        FILTER_ACTIVE,          // Aktywne
        FILTER_EXPIRED,         // Wygasłe
        FILTER_EXPIRING,        // Wygasające (7 dni)
        FILTER_THIS_MONTH,      // Zakupione w tym miesiącu
        FILTER_STUDENT,         // Studenckie
        FILTER_STANDARD         // Standardowe
    };
    
    // Typy karnetów
    enum TypeFilter {
        TYPE_ALL = 0,
        TYPE_MONTHLY,           // Miesięczne
        TYPE_QUARTERLY,         // Kwartalne  
        TYPE_YEARLY,            // Roczne
        TYPE_STUDENT,           // Studenckie
        TYPE_STANDARD           // Standardowe
    };
    
    // Stałe
    static const int EXPIRING_DAYS = 7;
    bool m_servicesInitialized;
};

#endif // MEMBERSHIPTABLEWIDGET_H