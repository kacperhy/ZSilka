// src/gui/widgets/dashboardwidget.cpp
#include "dashboardwidget.h"
#include "ui_dashboardwidget.h"

// Serwisy z oryginalnej aplikacji
#include "services/uslugi_klienta.h"
#include "services/uslugi_karnetu.h"
#include "services/uslugi_zajec.h"
#include "services/uslugi_raportow.h"

// Modele
#include "models/klient.h"
#include "models/karnet.h"
#include "models/zajecia.h"

#include <QHeaderView>
#include <QDate>
#include <QDebug>
#include <QMessageBox>
#include <QApplication>

DashboardWidget::DashboardWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DashboardWidget)
    , m_uslugiKlienta(nullptr)
    , m_uslugiKarnetu(nullptr)
    , m_uslugiZajec(nullptr)
    , m_uslugiRaportow(nullptr)
    , m_servicesInitialized(false)
{
    ui->setupUi(this);
    
    setupUI();
    setupStatisticsCards();
    setupQuickActions();
    setupRecentActivity();
    setupExpiringMemberships();
    setupUpcomingClasses();
    setupConnections();
    
    // Timer do automatycznego odświeżania
    m_refreshTimer = new QTimer(this);
    m_refreshTimer->setInterval(REFRESH_INTERVAL);
    connect(m_refreshTimer, &QTimer::timeout, this, &DashboardWidget::onAutoRefresh);
    
    qDebug() << "DashboardWidget utworzony";
}

DashboardWidget::~DashboardWidget()
{
    delete ui;
}

void DashboardWidget::initializeServices(UslugiKlienta* uslugiKlienta,
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
        
        // Włącz przyciski akcji
        m_addClientButton->setEnabled(true);
        m_addMembershipButton->setEnabled(true);
        m_addClassButton->setEnabled(true);
        m_generateReportButton->setEnabled(true);
        m_viewClientsButton->setEnabled(true);
        m_viewMembershipsButton->setEnabled(true);
        m_viewClassesButton->setEnabled(true);
        
        // Rozpocznij odświeżanie
        refresh();
        m_refreshTimer->start();
        
        qDebug() << "DashboardWidget - serwisy zainicjalizowane";
    }
}

void DashboardWidget::setupUI()
{
    // Główny layout z scroll area
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(12, 12, 12, 12);
    m_mainLayout->setSpacing(12);
    
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    
    m_contentWidget = new QWidget();
    QVBoxLayout* contentLayout = new QVBoxLayout(m_contentWidget);
    contentLayout->setSpacing(16);
    
    // Dodaj główne sekcje
    m_statisticsGroup = new QGroupBox("Podsumowanie", m_contentWidget);
    m_quickActionsGroup = new QGroupBox("Szybkie akcje", m_contentWidget);
    m_recentActivityGroup = new QGroupBox("Ostatnia aktywność", m_contentWidget);
    
    // Layout dla sekcji bocznych
    QHBoxLayout* sideLayout = new QHBoxLayout();
    m_expiringMembershipsGroup = new QGroupBox("Wygasające karnety", m_contentWidget);
    m_upcomingClassesGroup = new QGroupBox("Nadchodzące zajęcia", m_contentWidget);
    
    sideLayout->addWidget(m_expiringMembershipsGroup);
    sideLayout->addWidget(m_upcomingClassesGroup);
    
    // Dodaj do głównego layout
    contentLayout->addWidget(m_statisticsGroup);
    contentLayout->addWidget(m_quickActionsGroup);
    contentLayout->addWidget(m_recentActivityGroup);
    contentLayout->addLayout(sideLayout);
    contentLayout->addStretch();
    
    m_scrollArea->setWidget(m_contentWidget);
    m_mainLayout->addWidget(m_scrollArea);
}

void DashboardWidget::setupStatisticsCards()
{
    QHBoxLayout* statsLayout = new QHBoxLayout(m_statisticsGroup);
    
    // Karta klientów
    m_clientsCard = createStatCard("Klienci", "0", "Zarejestrowani", QColor("#2E86AB"));
    m_clientsCountLabel = m_clientsCard->findChild<QLabel*>("countLabel");
    m_clientsSubLabel = m_clientsCard->findChild<QLabel*>("subLabel");
    
    // Karta karnetów
    m_membershipsCard = createStatCard("Karnety", "0", "Aktywne", QColor("#A8DADC"));
    m_membershipsCountLabel = m_membershipsCard->findChild<QLabel*>("countLabel");
    m_membershipsSubLabel = m_membershipsCard->findChild<QLabel*>("subLabel");
    
    // Karta zajęć
    m_classesCard = createStatCard("Zajęcia", "0", "W tym tygodniu", QColor("#457B9D"));
    m_classesCountLabel = m_classesCard->findChild<QLabel*>("countLabel");
    m_classesSubLabel = m_classesCard->findChild<QLabel*>("subLabel");
    
    // Karta przychodów
    m_revenueCard = createStatCard("Przychody", "0 zł", "W tym miesiącu", QColor("#1D3557"));
    m_revenueLabel = m_revenueCard->findChild<QLabel*>("countLabel");
    m_revenueSubLabel = m_revenueCard->findChild<QLabel*>("subLabel");
    
    statsLayout->addWidget(m_clientsCard);
    statsLayout->addWidget(m_membershipsCard);
    statsLayout->addWidget(m_classesCard);
    statsLayout->addWidget(m_revenueCard);
}

QFrame* DashboardWidget::createStatCard(const QString& title, const QString& value, 
                                       const QString& subtitle, const QColor& color)
{
    QFrame* card = new QFrame();
    card->setFrameStyle(QFrame::StyledPanel);
    card->setStyleSheet(QString(
        "QFrame {"
        "  background-color: white;"
        "  border: 2px solid %1;"
        "  border-radius: 8px;"
        "  padding: 16px;"
        "}"
        "QFrame:hover {"
        "  border-color: %2;"
        "  box-shadow: 0 4px 8px rgba(0,0,0,0.1);"
        "}"
    ).arg(color.name()).arg(color.lighter(120).name()));
    
    QVBoxLayout* layout = new QVBoxLayout(card);
    layout->setSpacing(8);
    
    // Tytuł
    QLabel* titleLabel = new QLabel(title);
    titleLabel->setStyleSheet("font-size: 12px; color: #666; font-weight: bold;");
    
    // Wartość główna
    QLabel* countLabel = new QLabel(value);
    countLabel->setObjectName("countLabel");
    countLabel->setStyleSheet(QString("font-size: 28px; font-weight: bold; color: %1;").arg(color.name()));
    
    // Podtytuł
    QLabel* subLabel = new QLabel(subtitle);
    subLabel->setObjectName("subLabel");
    subLabel->setStyleSheet("font-size: 11px; color: #888;");
    
    layout->addWidget(titleLabel);
    layout->addWidget(countLabel);
    layout->addWidget(subLabel);
    layout->addStretch();
    
    return card;
}

void DashboardWidget::setupQuickActions()
{
    QGridLayout* actionsLayout = new QGridLayout(m_quickActionsGroup);
    actionsLayout->setSpacing(12);
    
    // Przyciski akcji
    m_addClientButton = new QPushButton("Dodaj klienta");
    m_addClientButton->setIcon(QIcon(":/icons/client.png"));
    m_addClientButton->setEnabled(false);
    
    m_addMembershipButton = new QPushButton("Dodaj karnet");
    m_addMembershipButton->setIcon(QIcon(":/icons/membership.png"));
    m_addMembershipButton->setEnabled(false);
    
    m_addClassButton = new QPushButton("Dodaj zajęcia");
    m_addClassButton->setIcon(QIcon(":/icons/class.png"));
    m_addClassButton->setEnabled(false);
    
    m_generateReportButton = new QPushButton("Generuj raport");
    m_generateReportButton->setIcon(QIcon(":/icons/report.png"));
    m_generateReportButton->setEnabled(false);
    
    // Przyciski przeglądania
    m_viewClientsButton = new QPushButton("Zobacz klientów");
    m_viewClientsButton->setIcon(QIcon(":/icons/view.png"));
    m_viewClientsButton->setEnabled(false);
    
    m_viewMembershipsButton = new QPushButton("Zobacz karnety");
    m_viewMembershipsButton->setIcon(QIcon(":/icons/view.png"));
    m_viewMembershipsButton->setEnabled(false);
    
    m_viewClassesButton = new QPushButton("Zobacz zajęcia");
    m_viewClassesButton->setIcon(QIcon(":/icons/view.png"));
    m_viewClassesButton->setEnabled(false);
    
    // Rozmieść w siatce 2x4
    actionsLayout->addWidget(m_addClientButton, 0, 0);
    actionsLayout->addWidget(m_addMembershipButton, 0, 1);
    actionsLayout->addWidget(m_addClassButton, 0, 2);
    actionsLayout->addWidget(m_generateReportButton, 0, 3);
    actionsLayout->addWidget(m_viewClientsButton, 1, 0);
    actionsLayout->addWidget(m_viewMembershipsButton, 1, 1);
    actionsLayout->addWidget(m_viewClassesButton, 1, 2);
    
    // Cel miesięczny
    QWidget* goalWidget = new QWidget();
    QVBoxLayout* goalLayout = new QVBoxLayout(goalWidget);
    
    m_monthlyGoalLabel = new QLabel("Cel miesięczny: 50 nowych karnetów");
    m_monthlyGoalProgress = new QProgressBar();
    m_monthlyGoalProgress->setRange(0, 50);
    m_monthlyGoalProgress->setValue(0);
    
    goalLayout->addWidget(m_monthlyGoalLabel);
    goalLayout->addWidget(m_monthlyGoalProgress);
    
    actionsLayout->addWidget(goalWidget, 1, 3);
}

void DashboardWidget::setupRecentActivity()
{
    QVBoxLayout* layout = new QVBoxLayout(m_recentActivityGroup);
    
    m_recentActivityTable = new QTableWidget();
    m_recentActivityTable->setColumnCount(4);
    m_recentActivityTable->setHorizontalHeaderLabels({"Czas", "Typ", "Opis", "Użytkownik"});
    m_recentActivityTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_recentActivityTable->setAlternatingRowColors(true);
    m_recentActivityTable->setMaximumHeight(200);
    
    // Konfiguracja nagłówków
    QHeaderView* header = m_recentActivityTable->horizontalHeader();
    header->setStretchLastSection(true);
    header->setDefaultSectionSize(120);
    
    layout->addWidget(m_recentActivityTable);
}

void DashboardWidget::setupExpiringMemberships()
{
    QVBoxLayout* layout = new QVBoxLayout(m_expiringMembershipsGroup);
    
    m_expiringMembershipsTable = new QTableWidget();
    m_expiringMembershipsTable->setColumnCount(3);
    m_expiringMembershipsTable->setHorizontalHeaderLabels({"Klient", "Typ", "Wygasa"});
    m_expiringMembershipsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_expiringMembershipsTable->setAlternatingRowColors(true);
    m_expiringMembershipsTable->setMaximumHeight(250);
    
    // Konfiguracja nagłówków
    QHeaderView* header = m_expiringMembershipsTable->horizontalHeader();
    header->setStretchLastSection(true);
    
    layout->addWidget(m_expiringMembershipsTable);
}

void DashboardWidget::setupUpcomingClasses()
{
    QVBoxLayout* layout = new QVBoxLayout(m_upcomingClassesGroup);
    
    m_upcomingClassesTable = new QTableWidget();
    m_upcomingClassesTable->setColumnCount(4);
    m_upcomingClassesTable->setHorizontalHeaderLabels({"Zajęcia", "Trener", "Data", "Czas"});
    m_upcomingClassesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_upcomingClassesTable->setAlternatingRowColors(true);
    m_upcomingClassesTable->setMaximumHeight(250);
    
    // Konfiguracja nagłówków
    QHeaderView* header = m_upcomingClassesTable->horizontalHeader();
    header->setStretchLastSection(true);
    
    layout->addWidget(m_upcomingClassesTable);
}

void DashboardWidget::setupConnections()
{
    connect(m_addClientButton, &QPushButton::clicked, this, &DashboardWidget::onQuickAddClient);
    connect(m_addMembershipButton, &QPushButton::clicked, this, &DashboardWidget::onQuickAddMembership);
    connect(m_addClassButton, &QPushButton::clicked, this, &DashboardWidget::onQuickAddClass);
    connect(m_generateReportButton, &QPushButton::clicked, this, &DashboardWidget::onGenerateReports);
    connect(m_viewClientsButton, &QPushButton::clicked, this, &DashboardWidget::onViewAllClients);
    connect(m_viewMembershipsButton, &QPushButton::clicked, this, &DashboardWidget::onViewAllMemberships);
    connect(m_viewClassesButton, &QPushButton::clicked, this, &DashboardWidget::onViewAllClasses);
}

void DashboardWidget::refresh()
{
    if (!m_servicesInitialized) return;
    
    try {
        updateStatistics();
        updateRecentActivity();
        updateExpiringMemberships();
        updateUpcomingClasses();
        updateQuickStats();
        
        qDebug() << "Dashboard odświeżony";
    } catch (const std::exception& e) {
        qWarning() << "Błąd odświeżania dashboard:" << e.what();
        QMessageBox::warning(this, "Błąd", 
            QString("Nie udało się odświeżyć dashboard:\n%1").arg(e.what()));
    }
}

void DashboardWidget::updateStatistics()
{
    // Aktualizuj statystyki klientów
    auto klienci = m_uslugiKlienta->pobierzWszystkichKlientow();
    m_clientsCountLabel->setText(QString::number(klienci.size()));
    
    // Policz nowych klientów w tym miesiącu
    QDate currentDate = QDate::currentDate();
    int nowyKlienciTenMiesiac = 0;
    for (const auto& klient : klienci) {
        QDate dataRejestracji = QDate::fromString(QString::fromStdString(klient.pobierzDateRejestracji()), "yyyy-MM-dd");
        if (dataRejestracji.year() == currentDate.year() && dataRejestracji.month() == currentDate.month()) {
            nowyKlienciTenMiesiac++;
        }
    }
    m_clientsSubLabel->setText(QString("(+%1 w tym miesiącu)").arg(nowyKlienciTenMiesiac));
    
    // Aktualizuj statystyki karnetów
    auto karnety = m_uslugiKarnetu->pobierzWszystkieKarnety();
    int aktywneKarnety = 0;
    double przychodTenMiesiac = 0.0;
    
    for (const auto& karnet : karnety) {
        if (karnet.czyWazny()) {
            aktywneKarnety++;
        }
        
        // Policz przychody z tego miesiąca
        QDate dataRozpoczecia = QDate::fromString(QString::fromStdString(karnet.pobierzDateRozpoczecia()), "yyyy-MM-dd");
        if (dataRozpoczecia.year() == currentDate.year() && dataRozpoczecia.month() == currentDate.month()) {
            przychodTenMiesiac += karnet.pobierzCene();
        }
    }
    
    m_membershipsCountLabel->setText(QString::number(aktywneKarnety));
    m_membershipsSubLabel->setText(QString("z %1 łącznie").arg(karnety.size()));
    
    // Aktualizuj statystyki zajęć (w tym tygodniu)
    auto zajecia = m_uslugiZajec->pobierzWszystkieZajecia();
    int zajeciaTenTydzien = 0;
    QDate startTygodnia = currentDate.addDays(-(currentDate.dayOfWeek() - 1));
    QDate koniecTygodnia = startTygodnia.addDays(6);
    
    for (const auto& zajecie : zajecia) {
        QDate dataZajec = QDate::fromString(QString::fromStdString(zajecie.pobierzDate()), "yyyy-MM-dd");
        if (dataZajec >= startTygodnia && dataZajec <= koniecTygodnia) {
            zajeciaTenTydzien++;
        }
    }
    
    m_classesCountLabel->setText(QString::number(zajeciaTenTydzien));
    m_classesSubLabel->setText(QString("z %1 łącznie").arg(zajecia.size()));
    
    // Aktualizuj przychody
    m_revenueLabel->setText(QString("%1 zł").arg(przychodTenMiesiac, 0, 'f', 0));
    m_revenueSubLabel->setText("W tym miesiącu");
}

void DashboardWidget::updateRecentActivity()
{
    // Przykładowa aktywność - w prawdziwej aplikacji można pobrać z historii zmian
    m_recentActivityTable->setRowCount(5);
    
    QStringList aktywnosci = {
        "Dodano klienta Jan Kowalski",
        "Zakupiono karnet miesięczny",
        "Zarezerwowano zajęcia Yoga",
        "Anulowano rezerwację",
        "Wygenerowano raport"
    };
    
    QStringList czasy = {
        "10:30", "10:15", "09:45", "09:30", "09:00"
    };
    
    QStringList typy = {
        "Klient", "Karnet", "Rezerwacja", "Rezerwacja", "Raport"
    };
    
    for (int i = 0; i < 5; ++i) {
        m_recentActivityTable->setItem(i, 0, new QTableWidgetItem(czasy[i]));
        m_recentActivityTable->setItem(i, 1, new QTableWidgetItem(typy[i]));
        m_recentActivityTable->setItem(i, 2, new QTableWidgetItem(aktywnosci[i]));
        m_recentActivityTable->setItem(i, 3, new QTableWidgetItem("System"));
    }
}

void DashboardWidget::updateExpiringMemberships()
{
    auto karnety = m_uslugiKarnetu->pobierzWszystkieKarnety();
    QDate currentDate = QDate::currentDate();
    QDate warningDate = currentDate.addDays(EXPIRING_DAYS_WARNING);
    
    // Filtruj wygasające karnety
    std::vector<Karnet> wygasajace;
    for (const auto& karnet : karnety) {
        if (!karnet.czyWazny()) continue;
        
        QDate dataZakonczenia = QDate::fromString(QString::fromStdString(karnet.pobierzDateZakonczenia()), "yyyy-MM-dd");
        if (dataZakonczenia <= warningDate) {
            wygasajace.push_back(karnet);
        }
    }
    
    m_expiringMembershipsTable->setRowCount(wygasajace.size());
    
    for (size_t i = 0; i < wygasajace.size(); ++i) {
        const auto& karnet = wygasajace[i];
        
        // Pobierz dane klienta
        auto klient = m_uslugiKlienta->pobierzKlientaPoId(karnet.pobierzIdKlienta());
        QString nazwaKlienta = "Nieznany";
        if (klient) {
            nazwaKlienta = QString::fromStdString(klient->pobierzPelneNazwisko());
        }
        
        m_expiringMembershipsTable->setItem(i, 0, new QTableWidgetItem(nazwaKlienta));
        m_expiringMembershipsTable->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(karnet.pobierzTyp())));
        
        QDate dataZakonczenia = QDate::fromString(QString::fromStdString(karnet.pobierzDateZakonczenia()), "yyyy-MM-dd");
        m_expiringMembershipsTable->setItem(i, 2, new QTableWidgetItem(dataZakonczenia.toString("dd.MM.yyyy")));
    }
}

void DashboardWidget::updateUpcomingClasses()
{
    auto zajecia = m_uslugiZajec->pobierzWszystkieZajecia();
    QDate currentDate = QDate::currentDate();
    QDate endDate = currentDate.addDays(7);
    
    // Filtruj nadchodzące zajęcia
    std::vector<Zajecia> nadchodzace;
    for (const auto& zajecie : zajecia) {
        QDate dataZajec = QDate::fromString(QString::fromStdString(zajecie.pobierzDate()), "yyyy-MM-dd");
        if (dataZajec >= currentDate && dataZajec <= endDate) {
            nadchodzace.push_back(zajecie);
        }
    }
    
    m_upcomingClassesTable->setRowCount(nadchodzace.size());
    
    for (size_t i = 0; i < nadchodzace.size(); ++i) {
        const auto& zajecie = nadchodzace[i];
        
        m_upcomingClassesTable->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(zajecie.pobierzNazwe())));
        m_upcomingClassesTable->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(zajecie.pobierzTrenera())));
        
        QDate dataZajec = QDate::fromString(QString::fromStdString(zajecie.pobierzDate()), "yyyy-MM-dd");
        m_upcomingClassesTable->setItem(i, 2, new QTableWidgetItem(dataZajec.toString("dd.MM.yyyy")));
        m_upcomingClassesTable->setItem(i, 3, new QTableWidgetItem(QString::fromStdString(zajecie.pobierzCzas())));
    }
}

void DashboardWidget::updateQuickStats()
{
    // Aktualizuj pasek postępu celów miesięcznych
    auto karnety = m_uslugiKarnetu->pobierzWszystkieKarnety();
    QDate currentDate = QDate::currentDate();
    
    int karnetyTenMiesiac = 0;
    for (const auto& karnet : karnety) {
        QDate dataRozpoczecia = QDate::fromString(QString::fromStdString(karnet.pobierzDateRozpoczecia()), "yyyy-MM-dd");
        if (dataRozpoczecia.year() == currentDate.year() && dataRozpoczecia.month() == currentDate.month()) {
            karnetyTenMiesiac++;
        }
    }
    
    m_monthlyGoalProgress->setValue(karnetyTenMiesiac);
    m_monthlyGoalLabel->setText(QString("Cel miesięczny: %1/50 nowych karnetów").arg(karnetyTenMiesiac));
}

// === SLOTY ===

void DashboardWidget::onQuickAddClient()
{
    // TODO: Otwórz dialog dodawania klienta
    QMessageBox::information(this, "Dodaj klienta", "Funkcja dodawania klienta - w przygotowaniu");
}

void DashboardWidget::onQuickAddMembership()
{
    // TODO: Otwórz dialog dodawania karnetu  
    QMessageBox::information(this, "Dodaj karnet", "Funkcja dodawania karnetu - w przygotowaniu");
}

void DashboardWidget::onQuickAddClass()
{
    // TODO: Otwórz dialog dodawania zajęć
    QMessageBox::information(this, "Dodaj zajęcia", "Funkcja dodawania zajęć - w przygotowaniu");
}

void DashboardWidget::onViewAllClients()
{
    // TODO: Przełącz na zakładkę klientów
    QMessageBox::information(this, "Klienci", "Przełączanie na zakładkę klientów - w przygotowaniu");
}

void DashboardWidget::onViewAllMemberships()
{
    // TODO: Przełącz na zakładkę karnetów
    QMessageBox::information(this, "Karnety", "Przełączanie na zakładkę karnetów - w przygotowaniu");
}

void DashboardWidget::onViewAllClasses()
{
    // TODO: Przełącz na zakładkę zajęć
    QMessageBox::information(this, "Zajęcia", "Przełączanie na zakładkę zajęć - w przy