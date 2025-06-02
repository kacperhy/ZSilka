// src/gui/mainwindow.cpp
#include "mainwindow.h"
#include "ui_mainwindow.h"

// Dialogi
#include "dialogs/clientdialog.h"
#include "dialogs/membershipdialog.h"
#include "dialogs/classdialog.h"
#include "dialogs/reservationdialog.h"

// Widgety
#include "widgets/dashboardwidget.h"
#include "widgets/clienttablewidget.h"
#include "widgets/membershiptablewidget.h"
#include "widgets/classtablewidget.h"

// Raporty
#include "reports/reportwindow.h"

// Serwisy z oryginalnej aplikacji
#include "services/uslugi_klienta.h"
#include "services/uslugi_karnetu.h"
#include "services/uslugi_zajec.h"
#include "services/uslugi_raportow.h"

// Qt
#include <QApplication>
#include <QMessageBox>
#include <QFileDialog>
#include <QProgressDialog>
#include <QDesktopServices>
#include <QUrl>
#include <QDebug>
#include <QSplitter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_uslugiKlienta(nullptr)
    , m_uslugiKarnetu(nullptr)
    , m_uslugiZajec(nullptr)
    , m_uslugiRaportow(nullptr)
    , m_tabWidget(nullptr)
    , m_dashboardWidget(nullptr)
    , m_clientTableWidget(nullptr)
    , m_membershipTableWidget(nullptr)
    , m_classTableWidget(nullptr)
    , m_servicesInitialized(false)
    , m_dataModified(false)
    , m_currentTabIndex(0)
{
    ui->setupUi(this);

    // Inicjalizacja ustawień
    m_settings = new QSettings(this);

    // Konfiguracja UI
    setupUI();
    setupMenuBar();
    setupToolBar();
    setupStatusBar();
    setupTabs();
    setupConnections();

    // Załaduj ustawienia
    loadSettings();

    // Konfiguracja timerów
    m_statusUpdateTimer = new QTimer(this);
    m_statusUpdateTimer->setInterval(STATUS_UPDATE_INTERVAL);
    connect(m_statusUpdateTimer, &QTimer::timeout, this, &MainWindow::updateStatusBar);

    m_expirationCheckTimer = new QTimer(this);
    m_expirationCheckTimer->setInterval(EXPIRATION_CHECK_INTERVAL);
    connect(m_expirationCheckTimer, &QTimer::timeout, this, &MainWindow::onMembershipExpiring);

    qDebug() << "MainWindow utworzone";
}

MainWindow::~MainWindow()
{
    saveSettings();
    delete ui;
}

void MainWindow::initializeServices(UslugiKlienta* uslugiKlienta,
                                    UslugiKarnetu* uslugiKarnetu,
                                    UslugiZajec* uslugiZajec,
                                    UslugiRaportow* uslugiRaportow)
{
    m_uslugiKlienta = uslugiKlienta;
    m_uslugiKarnetu = uslugiKarnetu;
    m_uslugiZajec = uslugiZajec;
    m_uslugiRaportow = uslugiRaportow;

    if (m_uslugiKlienta && m_uslugiKarnetu && m_uslugiZajec && m_uslugiRaportow) {
        m_servicesInitialized = true;

        // Przekaż serwisy do widgetów
        if (m_dashboardWidget) {
            m_dashboardWidget->initializeServices(m_uslugiKlienta, m_uslugiKarnetu, m_uslugiZajec, m_uslugiRaportow);
        }

        if (m_clientTableWidget) {
            m_clientTableWidget->initializeServices(m_uslugiKlienta);
        }

        if (m_membershipTableWidget) {
            m_membershipTableWidget->initializeServices(m_uslugiKarnetu, m_uslugiKlienta);
        }

        if (m_classTableWidget) {
            m_classTableWidget->initializeServices(m_uslugiZajec);
        }

        // Rozpocznij aktualizację statusu
        m_statusUpdateTimer->start();
        m_expirationCheckTimer->start();

        // Pierwsza aktualizacja
        updateStatusBar();
        updateWindowTitle();

        qDebug() << "Serwisy zainicjalizowane w MainWindow";
    } else {
        qWarning() << "Nie wszystkie serwisy zostały przekazane do MainWindow";
    }
}

void MainWindow::setupUI()
{
    setWindowTitle("System Zarządzania Siłownią");
    setWindowIcon(QIcon(":/icons/app.png"));
    resize(1200, 800);
    setMinimumSize(800, 600);

    // Ustaw centralny widget
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // Layout główny
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(4, 4, 4, 4);

    // Utwórz tab widget
    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setTabPosition(QTabWidget::North);
    m_tabWidget->setMovable(true);
    m_tabWidget->setTabsClosable(false);

    mainLayout->addWidget(m_tabWidget);
}

void MainWindow::setupMenuBar()
{
    // Menu Plik
    QMenu* fileMenu = menuBar()->addMenu("&Plik");

    m_newClientAction = fileMenu->addAction(QIcon(":/icons/client.png"), "&Nowy klient...", this, &MainWindow::newClient);
    m_newClientAction->setShortcut(QKeySequence::New);

    fileMenu->addSeparator();

    m_importAction = fileMenu->addAction(QIcon(":/icons/import.png"), "&Importuj dane...", this, &MainWindow::importData);
    m_exportAction = fileMenu->addAction(QIcon(":/icons/export.png"), "&Eksportuj dane...", this, &MainWindow::exportData);

    fileMenu->addSeparator();

    m_printAction = fileMenu->addAction(QIcon(":/icons/print.png"), "&Drukuj raport...", this, &MainWindow::printReport);
    m_printAction->setShortcut(QKeySequence::Print);

    fileMenu->addSeparator();

    m_quitAction = fileMenu->addAction("&Zakończ", this, &MainWindow::quit);
    m_quitAction->setShortcut(QKeySequence::Quit);

    // Menu Edycja
    QMenu* editMenu = menuBar()->addMenu("&Edycja");

    m_undoAction = editMenu->addAction(QIcon(":/icons/undo.png"), "&Cofnij", this, &MainWindow::undo);
    m_undoAction->setShortcut(QKeySequence::Undo);
    m_undoAction->setEnabled(false);

    m_redoAction = editMenu->addAction(QIcon(":/icons/redo.png"), "&Ponów", this, &MainWindow::redo);
    m_redoAction->setShortcut(QKeySequence::Redo);
    m_redoAction->setEnabled(false);

    editMenu->addSeparator();

    m_preferencesAction = editMenu->addAction(QIcon(":/icons/settings.png"), "&Ustawienia...", this, &MainWindow::preferences);

    // Menu Widok
    QMenu* viewMenu = menuBar()->addMenu("&Widok");

    m_fullScreenAction = viewMenu->addAction("&Pełny ekran", this, &MainWindow::toggleFullScreen);
    m_fullScreenAction->setShortcut(QKeySequence::FullScreen);
    m_fullScreenAction->setCheckable(true);

    viewMenu->addSeparator();

    QAction* toggleToolBarAction = viewMenu->addAction("Pasek &narzędzi", this, &MainWindow::toggleToolBar);
    toggleToolBarAction->setCheckable(true);
    toggleToolBarAction->setChecked(true);

    QAction* toggleStatusBarAction = viewMenu->addAction("Pasek &stanu", this, &MainWindow::toggleStatusBar);
    toggleStatusBarAction->setCheckable(true);
    toggleStatusBarAction->setChecked(true);

    viewMenu->addSeparator();

    m_refreshAction = viewMenu->addAction(QIcon(":/icons/refresh.png"), "&Odśwież", this, &MainWindow::refreshView);
    m_refreshAction->setShortcut(QKeySequence::Refresh);

    // Menu Narzędzia
    QMenu* toolsMenu = menuBar()->addMenu("&Narzędzia");

    toolsMenu->addAction(QIcon(":/icons/database.png"), "&Konserwacja bazy danych...", this, &MainWindow::databaseMaintenance);
    toolsMenu->addSeparator();
    toolsMenu->addAction(QIcon(":/icons/backup.png"), "&Kopia zapasowa...", this, &MainWindow::backupDatabase);
    toolsMenu->addAction(QIcon(":/icons/restore.png"), "&Przywróć z kopii...", this, &MainWindow::restoreDatabase);
    toolsMenu->addSeparator();
    toolsMenu->addAction(QIcon(":/icons/info.png"), "&Informacje o systemie...", this, &MainWindow::systemInfo);

    // Menu Pomoc
    QMenu* helpMenu = menuBar()->addMenu("&Pomoc");

    m_helpAction = helpMenu->addAction(QIcon(":/icons/help.png"), "&Pomoc", this, &MainWindow::showHelp);
    m_helpAction->setShortcut(QKeySequence::HelpContents);

    helpMenu->addSeparator();
    helpMenu->addAction("Sprawdź &aktualizacje...", this, &MainWindow::checkForUpdates);
    helpMenu->addSeparator();

    m_aboutAction = helpMenu->addAction(QIcon(":/icons/about.png"), "&O programie...", this, &MainWindow::showAbout);
}

void MainWindow::setupToolBar()
{
    m_mainToolBar = addToolBar("Główny");
    m_mainToolBar->setObjectName("MainToolBar");
    m_mainToolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    // Dodaj główne akcje
    m_mainToolBar->addAction(m_newClientAction);
    m_mainToolBar->addSeparator();
    m_mainToolBar->addAction(m_importAction);
    m_mainToolBar->addAction(m_exportAction);
    m_mainToolBar->addSeparator();
    m_mainToolBar->addAction(m_refreshAction);
    m_mainToolBar->addSeparator();
    m_mainToolBar->addAction(m_helpAction);
}

void MainWindow::setupStatusBar()
{
    m_statusLabel = new QLabel("Gotowy");
    statusBar()->addWidget(m_statusLabel);

    statusBar()->addPermanentWidget(new QLabel("|"));

    m_clientCountLabel = new QLabel("Klienci: 0");
    statusBar()->addPermanentWidget(m_clientCountLabel);

    statusBar()->addPermanentWidget(new QLabel("|"));

    m_membershipCountLabel = new QLabel("Karnety: 0");
    statusBar()->addPermanentWidget(m_membershipCountLabel);

    statusBar()->addPermanentWidget(new QLabel("|"));

    m_classCountLabel = new QLabel("Zajęcia: 0");
    statusBar()->addPermanentWidget(m_classCountLabel);

    // Pasek postępu (ukryty domyślnie)
    m_progressBar = new QProgressBar();
    m_progressBar->setVisible(false);
    statusBar()->addPermanentWidget(m_progressBar);
}

void MainWindow::setupTabs()
{
    // Dashboard - przegląd ogólny
    m_dashboardWidget = new DashboardWidget(this);
    m_tabWidget->addTab(m_dashboardWidget, QIcon(":/icons/dashboard.png"), "Dashboard");

    // Klienci
    m_clientTableWidget = new ClientTableWidget(this);
    m_tabWidget->addTab(m_clientTableWidget, QIcon(":/icons/client.png"), "Klienci");

    // Karnety
    m_membershipTableWidget = new MembershipTableWidget(this);
    m_tabWidget->addTab(m_membershipTableWidget, QIcon(":/icons/membership.png"), "Karnety");

    // Zajęcia
    m_classTableWidget = new ClassTableWidget(this);
    m_tabWidget->addTab(m_classTableWidget, QIcon(":/icons/class.png"), "Zajęcia");

    // Ustaw dashboard jako domyślną zakładkę
    m_tabWidget->setCurrentIndex(0);
}

void MainWindow::setupConnections()
{
    // Sygnały od tabWidget
    connect(m_tabWidget, &QTabWidget::currentChanged, this, &MainWindow::onTabChanged);

    // Można dodać więcej połączeń w miarę potrzeb
}

void MainWindow::loadSettings()
{
    // Geometria okna
    restoreGeometry(m_settings->value("geometry").toByteArray());
    restoreState(m_settings->value("windowState").toByteArray());

    // Ostatnia aktywna zakładka
    int lastTab = m_settings->value("lastTab", 0).toInt();
    if (lastTab >= 0 && lastTab < m_tabWidget->count()) {
        m_tabWidget->setCurrentIndex(lastTab);
    }
}

void MainWindow::saveSettings()
{
    if (m_settings) {
        m_settings->setValue("geometry", saveGeometry());
        m_settings->setValue("windowState", saveState());
        m_settings->setValue("lastTab", m_tabWidget->currentIndex());
        m_settings->sync();
    }
}

void MainWindow::updateWindowTitle()
{
    QString title = "System Zarządzania Siłownią";
    if (m_dataModified) {
        title += " *";
    }
    setWindowTitle(title);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (confirmClose()) {
        saveSettings();
        event->accept();
    } else {
        event->ignore();
    }
}

bool MainWindow::confirmClose()
{
    if (m_dataModified) {
        QMessageBox::StandardButton ret = QMessageBox::question(this,
                                                                "Zamknięcie aplikacji",
                                                                "Masz niezapisane zmiany. Czy na pewno chcesz zamknąć aplikację?",
                                                                QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

        return ret == QMessageBox::Yes;
    }
    return true;
}

// === SLOTY MENU ===

void MainWindow::newClient()
{
    if (!m_servicesInitialized) return;

    ClientDialog dialog(m_uslugiKlienta, this);
    if (dialog.exec() == QDialog::Accepted) {
        onClientAdded();
        showNotification("Dodano nowego klienta");
    }
}

void MainWindow::importData()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    "Importuj dane", "", "Pliki CSV (*.csv);;Wszystkie pliki (*)");

    if (!fileName.isEmpty()) {
        // TODO: Implementacja importu z wykorzystaniem istniejącej klasy ImportDanych
        showNotification("Import danych - funkcja w przygotowaniu");
    }
}

void MainWindow::exportData()
{
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    "Eksportuj dane", "", "Pliki CSV (*.csv);;Wszystkie pliki (*)");

    if (!fileName.isEmpty()) {
        // TODO: Implementacja eksportu z wykorzystaniem istniejącej klasy EksportDanych
        showNotification("Eksport danych - funkcja w przygotowaniu");
    }
}

void MainWindow::printReport()
{
    if (!m_servicesInitialized) return;

    ReportWindow reportWindow(m_uslugiRaportow, this);
    reportWindow.exec();
}

void MainWindow::quit()
{
    close();
}

void MainWindow::preferences()
{
    showNotification("Ustawienia - funkcja w przygotowaniu");
}

void MainWindow::undo()
{
    showNotification("Cofnij - funkcja w przygotowaniu");
}

void MainWindow::redo()
{
    showNotification("Ponów - funkcja w przygotowaniu");
}

void MainWindow::toggleFullScreen()
{
    if (isFullScreen()) {
        showNormal();
        m_fullScreenAction->setChecked(false);
    } else {
        showFullScreen();
        m_fullScreenAction->setChecked(true);
    }
}

void MainWindow::toggleToolBar()
{
    m_mainToolBar->setVisible(!m_mainToolBar->isVisible());
}

void MainWindow::toggleStatusBar()
{
    statusBar()->setVisible(!statusBar()->isVisible());
}

void MainWindow::refreshView()
{
    if (!m_servicesInitialized) return;

    // Odśwież aktualną zakładkę
    switch (m_tabWidget->currentIndex()) {
    case 0: // Dashboard
        if (m_dashboardWidget) m_dashboardWidget->refresh();
        break;
    case 1: // Klienci
        if (m_clientTableWidget) m_clientTableWidget->refresh();
        break;
    case 2: // Karnety
        if (m_membershipTableWidget) m_membershipTableWidget->refresh();
        break;
    case 3: // Zajęcia
        if (m_classTableWidget) m_classTableWidget->refresh();
        break;
    }

    updateStatusBar();
    showNotification("Odświeżono dane");
}

void MainWindow::databaseMaintenance()
{
    showNotification("Konserwacja bazy danych - funkcja w przygotowaniu");
}

void MainWindow::backupDatabase()
{
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    "Kopia zapasowa bazy danych", "backup.db", "Pliki bazy danych (*.db)");

    if (!fileName.isEmpty()) {
        showNotification("Kopia zapasowa - funkcja w przygotowaniu");
    }
}

void MainWindow::restoreDatabase()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    "Przywróć z kopii", "", "Pliki bazy danych (*.db)");

    if (!fileName.isEmpty()) {
        showNotification("Przywracanie z kopii - funkcja w przygotowaniu");
    }
}

void MainWindow::systemInfo()
{
    QString info = QString("System Zarządzania Siłownią v2.0\n\n"
                           "Qt: %1\n"
                           "Kompilator: %2\n"
                           "System: %3")
                       .arg(QT_VERSION_STR)
                       .arg(__VERSION__)
                       .arg(QSysInfo::prettyProductName());

    QMessageBox::information(this, "Informacje o systemie", info);
}

void MainWindow::showHelp()
{
    QDesktopServices::openUrl(QUrl("https://github.com/example/gymmanagement/wiki"));
}

void MainWindow::showAbout()
{
    QMessageBox::about(this, "O programie",
                       "<h3>System Zarządzania Siłownią</h3>"
                       "<p>Wersja 2.0</p>"
                       "<p>Aplikacja do zarządzania klientami, karnetami i zajęciami w siłowni.</p>"
                       "<p>Stworzone z wykorzystaniem Qt i C++</p>");
}

void MainWindow::checkForUpdates()
{
    showNotification("Sprawdzanie aktualizacji - funkcja w przygotowaniu");
}

void MainWindow::onTabChanged(int index)
{
    m_currentTabIndex = index;

    // Można dodać specyficzną logikę dla każdej zakładki
    switch (index) {
    case 0: // Dashboard
        if (m_dashboardWidget && m_servicesInitialized) {
            m_dashboardWidget->refresh();
        }
        break;
    case 1: // Klienci
        // Aktywacja kontekstowego menu/narzędzi dla klientów
        break;
    case 2: // Karnety
        // Aktywacja kontekstowego menu/narzędzi dla karnetów
        break;
    case 3: // Zajęcia
        // Aktywacja kontekstowego menu/narzędzi dla zajęć
        break;
    }
}

void MainWindow::updateStatusBar()
{
    if (!m_servicesInitialized) return;

    try {
        // Aktualizuj liczniki
        auto klienci = m_uslugiKlienta->pobierzWszystkichKlientow();
        auto karnety = m_uslugiKarnetu->pobierzWszystkieKarnety();
        auto zajecia = m_uslugiZajec->pobierzWszystkieZajecia();

        m_clientCountLabel->setText(QString("Klienci: %1").arg(klienci.size()));
        m_membershipCountLabel->setText(QString("Karnety: %1").arg(karnety.size()));
        m_classCountLabel->setText(QString("Zajęcia: %1").arg(zajecia.size()));

        m_statusLabel->setText("Gotowy");

    } catch (const std::exception& e) {
        m_statusLabel->setText(QString("Błąd: %1").arg(e.what()));
        qWarning() << "Błąd aktualizacji paska stanu:" << e.what();
    }
}

void MainWindow::showNotification(const QString& message, int timeout)
{
    m_statusLabel->setText(message);

    // Przywróć normalny status po określonym czasie
    QTimer::singleShot(timeout, this, [this]() {
        if (m_statusLabel) {
            m_statusLabel->setText("Gotowy");
        }
    });
}

void MainWindow::onDataChanged()
{
    m_dataModified = true;
    updateWindowTitle();
    updateStatusBar();
}

void MainWindow::onClientAdded()
{
    onDataChanged();
    // Odśwież widoki klientów i dashboard
    if (m_clientTableWidget) {
        m_clientTableWidget->refresh();
    }
    if (m_dashboardWidget) {
        m_dashboardWidget->refresh();
    }
}

void MainWindow::onMembershipExpiring()
{
    // TODO: Sprawdź wygasające karnety i wyświetl powiadomienie
    if (!m_servicesInitialized) return;

    // Logika sprawdzania wygasających karnetów
    // można wykorzystać istniejące metody z serwisów
}

void MainWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::WindowStateChange) {
        if (isMinimized()) {
            // Okno zminimalizowane - można wstrzymać niektóre timery
        } else {
            // Okno przywrócone - wznów timery
        }
    }
    QMainWindow::changeEvent(event);
}
