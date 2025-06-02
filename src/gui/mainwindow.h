// src/gui/mainwindow.h
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QLabel>
#include <QProgressBar>
#include <QTimer>
#include <QCloseEvent>
#include <QSettings>
#include <memory>

// Forward declarations - serwisy z oryginalnej aplikacji
class UslugiKlienta;
class UslugiKarnetu;
class UslugiZajec;
class UslugiRaportow;

// Forward declarations - widgety GUI
class DashboardWidget;
class ClientTableWidget;
class MembershipTableWidget;
class ClassTableWidget;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // Inicjalizacja z serwisami z backendu
    void initializeServices(UslugiKlienta* uslugiKlienta,
                            UslugiKarnetu* uslugiKarnetu,
                            UslugiZajec* uslugiZajec,
                            UslugiRaportow* uslugiRaportow);

protected:
    void closeEvent(QCloseEvent *event) override;
    void changeEvent(QEvent *event) override;

private slots:
    // Menu Plik
    void newClient();
    void importData();
    void exportData();
    void printReport();
    void quit();

    // Menu Edycja
    void preferences();
    void undo();
    void redo();

    // Menu Widok
    void toggleFullScreen();
    void toggleToolBar();
    void toggleStatusBar();
    void refreshView();

    // Menu Narzędzia
    void databaseMaintenance();
    void backupDatabase();
    void restoreDatabase();
    void systemInfo();

    // Menu Pomoc
    void showHelp();
    void showAbout();
    void checkForUpdates();

    // Zakładki
    void onTabChanged(int index);

    // Status i powiadomienia
    void updateStatusBar();
    void showNotification(const QString& message, int timeout = 3000);

    // Obsługa danych
    void onDataChanged();
    void onClientAdded();
    void onMembershipExpiring();

private:
    void setupUI();
    void setupMenuBar();
    void setupToolBar();
    void setupStatusBar();
    void setupTabs();
    void setupConnections();

    void loadSettings();
    void saveSettings();

    void updateWindowTitle();
    void updateStatistics();

    bool confirmClose();

private:
    Ui::MainWindow *ui;

    // Serwisy - referencje do backendu
    UslugiKlienta* m_uslugiKlienta;
    UslugiKarnetu* m_uslugiKarnetu;
    UslugiZajec* m_uslugiZajec;
    UslugiRaportow* m_uslugiRaportow;

    // Główne komponenty UI
    QTabWidget* m_tabWidget;

    // Zakładki/widgety
    DashboardWidget* m_dashboardWidget;
    ClientTableWidget* m_clientTableWidget;
    MembershipTableWidget* m_membershipTableWidget;
    ClassTableWidget* m_classTableWidget;

    // Menu i paski narzędzi
    QMenuBar* m_menuBar;
    QToolBar* m_mainToolBar;
    QStatusBar* m_statusBar;

    // Elementy paska stanu
    QLabel* m_statusLabel;
    QLabel* m_clientCountLabel;
    QLabel* m_membershipCountLabel;
    QLabel* m_classCountLabel;
    QProgressBar* m_progressBar;

    // Timery
    QTimer* m_statusUpdateTimer;
    QTimer* m_expirationCheckTimer;

    // Ustawienia
    QSettings* m_settings;

    // Stany
    bool m_servicesInitialized;
    bool m_dataModified;
    int m_currentTabIndex;

    // Akcje menu - dla łatwego dostępu
    QAction* m_newClientAction;
    QAction* m_importAction;
    QAction* m_exportAction;
    QAction* m_printAction;
    QAction* m_quitAction;
    QAction* m_preferencesAction;
    QAction* m_undoAction;
    QAction* m_redoAction;
    QAction* m_fullScreenAction;
    QAction* m_refreshAction;
    QAction* m_helpAction;
    QAction* m_aboutAction;

    // Stałe
    static const int STATUS_UPDATE_INTERVAL = 30000; // 30 sekund
    static const int EXPIRATION_CHECK_INTERVAL = 3600000; // 1 godzina
};

#endif // MAINWINDOW_H
