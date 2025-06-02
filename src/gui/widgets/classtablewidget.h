// src/gui/widgets/classtablewidget.h
#ifndef CLASSTABLEWIDGET_H
#define CLASSTABLEWIDGET_H

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
#include <QSplitter>

// Istniejące klasy z oryginalnej aplikacji
#include "models/zajecia.h"
#include "models/rezerwacja.h"
#include "services/uslugi_zajec.h"

class ClassTableWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ClassTableWidget(QWidget *parent = nullptr);
    ~ClassTableWidget();

    // Inicjalizacja z serwisami (z oryginalnej aplikacji)
    void initializeServices(UslugiZajec* uslugiZajec);

public slots:
    void refresh();
    void addClass();
    void editClass();
    void deleteClass();
    void duplicateClass();
    void viewReservations();
    void addReservation();
    void cancelReservation();
    void searchClasses();
    void clearSearch();
    void exportClasses();

signals:
    void classAdded(const Zajecia& zajecia);
    void classEdited(const Zajecia& zajecia);
    void classDeleted(int classId);
    void reservationAdded(const Rezerwacja& rezerwacja);
    void reservationCancelled(int reservationId);

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

private slots:
    void onTableSelectionChanged();
    void onTableDoubleClicked(int row, int column);
    void onReservationTableSelectionChanged();
    void onReservationTableDoubleClicked(int row, int column);
    void onSearchTextChanged(const QString& text);
    void onFilterChanged();
    void onDateFilterChanged();
    void onClassSelectionChanged();
    void copyToClipboard();
    void selectAll();

private:
    void setupUI();
    void setupClassesTable();
    void setupReservationsTable();
    void setupToolbar();
    void setupConnections();
    void createContextMenus();
    
    void loadClassData();
    void loadReservationData();
    void populateClassesTable(const std::vector<Zajecia>& zajecia);
    void populateReservationsTable(const std::vector<Rezerwacja>& rezerwacje);
    void updateClassCount();
    void updateReservationCount();
    void filterClassesTable();
    
    Zajecia* getSelectedClass();
    QList<Zajecia*> getSelectedClasses();
    Rezerwacja* getSelectedReservation();
    QList<Rezerwacja*> getSelectedReservations();
    void selectClassById(int classId);
    
    QString formatDate(const std::string& dateStr);
    QString formatTime(const std::string& timeStr);
    QString formatDuration(int minutes);
    QString getClientName(int clientId);
    QTableWidgetItem* createTableItem(const QString& text);
    QTableWidgetItem* createCapacityItem(const Zajecia& zajecia);

private:
    // Serwis zajęć (z oryginalnej aplikacji)
    UslugiZajec* m_uslugiZajec;
    
    // Dane
    std::vector<Zajecia> m_allClasses;
    std::vector<Zajecia> m_filteredClasses;
    std::vector<Rezerwacja> m_allReservations;
    std::vector<Rezerwacja> m_filteredReservations;
    
    // UI Components
    QVBoxLayout* m_mainLayout;
    QHBoxLayout* m_toolbarLayout;
    QSplitter* m_splitter;
    
    // Toolbar
    QLineEdit* m_searchEdit;
    QComboBox* m_filterCombo;
    QComboBox* m_trainerFilterCombo;
    QDateEdit* m_fromDateEdit;
    QDateEdit* m_toDateEdit;
    QPushButton* m_addButton;
    QPushButton* m_editButton;
    QPushButton* m_deleteButton;
    QPushButton* m_duplicateButton;
    QPushButton* m_viewReservationsButton;
    QPushButton* m_addReservationButton;
    QPushButton* m_cancelReservationButton;
    QPushButton* m_refreshButton;
    QPushButton* m_exportButton;
    QLabel* m_classCountLabel;
    QLabel* m_reservationCountLabel;
    
    // Tabele
    QWidget* m_classesWidget;
    QWidget* m_reservationsWidget;
    QTableWidget* m_classesTable;
    QTableWidget* m_reservationsTable;
    
    // Context menus
    QMenu* m_classContextMenu;
    QMenu* m_reservationContextMenu;
    QAction* m_editClassAction;
    QAction* m_deleteClassAction;
    QAction* m_duplicateClassAction;
    QAction* m_viewReservationsAction;
    QAction* m_copyClassAction;
    QAction* m_selectAllClassesAction;
    QAction* m_addReservationAction;
    QAction* m_cancelReservationAction;
    QAction* m_copyReservationAction;
    QAction* m_selectAllReservationsAction;
    QAction* m_refreshAction;
    
    // Kolumny tabeli zajęć
    enum ClassColumnIndex {
        CLASS_COL_ID = 0,
        CLASS_COL_NAME,
        CLASS_COL_TRAINER,
        CLASS_COL_DATE,
        CLASS_COL_TIME,
        CLASS_COL_DURATION,
        CLASS_COL_MAX_PARTICIPANTS,
        CLASS_COL_RESERVATIONS,
        CLASS_COL_CAPACITY,
        CLASS_COL_DESCRIPTION,
        CLASS_COL_COUNT
    };
    
    // Kolumny tabeli rezerwacji
    enum ReservationColumnIndex {
        RES_COL_ID = 0,
        RES_COL_CLIENT_NAME,
        RES_COL_CLIENT_ID,
        RES_COL_CLASS_NAME,
        RES_COL_RESERVATION_DATE,
        RES_COL_STATUS,
        RES_COL_COUNT
    };
    
    // Filtry zajęć
    enum FilterType {
        FILTER_ALL = 0,
        FILTER_TODAY,           // Dzisiaj
        FILTER_THIS_WEEK,       // W tym tygodniu
        FILTER_THIS_MONTH,      // W tym miesiącu
        FILTER_UPCOMING,        // Nadchodzące
        FILTER_PAST,            // Przeszłe
        FILTER_FULL,            // Zapełnione
        FILTER_AVAILABLE        // Dostępne miejsca
    };
    
    // Stałe
    bool m_servicesInitialized;
    int m_selectedClassId;
};

#endif // CLASSTABLEWIDGET_H