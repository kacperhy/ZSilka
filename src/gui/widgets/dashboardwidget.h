// src/gui/widgets/dashboardwidget.h
#ifndef DASHBOARDWIDGET_H
#define DASHBOARDWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QGroupBox>
#include <QProgressBar>
#include <QTableWidget>
#include <QTimer>
#include <QScrollArea>
#include <memory>

// Forward declarations - serwisy z oryginalnej aplikacji
class UslugiKlienta;
class UslugiKarnetu;
class UslugiZajec;
class UslugiRaportow;

// Forward declarations - modele
class Klient;
class Karnet;
class Zajecia;

QT_BEGIN_NAMESPACE
namespace Ui { class DashboardWidget; }
QT_END_NAMESPACE

class DashboardWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DashboardWidget(QWidget *parent = nullptr);
    ~DashboardWidget();

    // Inicjalizacja z serwisami z backendu
    void initializeServices(UslugiKlienta* uslugiKlienta,
                           UslugiKarnetu* uslugiKarnetu,
                           UslugiZajec* uslugiZajec,
                           UslugiRaportow* uslugiRaportow);

public slots:
    void refresh();

private slots:
    void onQuickAddClient();
    void onQuickAddMembership();
    void onQuickAddClass();
    void onViewAllClients();
    void onViewAllMemberships();
    void onViewAllClasses();
    void onGenerateReports();
    void onAutoRefresh();

private:
    void setupUI();
    void setupStatisticsCards();
    void setupQuickActions();
    void setupRecentActivity();
    void setupExpiringMemberships();
    void setupUpcomingClasses();
    void setupConnections();
    
    void updateStatistics();
    void updateRecentActivity();
    void updateExpiringMemberships();
    void updateUpcomingClasses();
    void updateQuickStats();
    
    QFrame* createStatCard(const QString& title, const QString& value, 
                          const QString& subtitle, const QColor& color);
    QFrame* createActionCard(const QString& title, const QString& description,
                            const QString& iconPath, QPushButton* button);

private:
    Ui::DashboardWidget *ui;
    
    // Serwisy - referencje do backendu
    UslugiKlienta* m_uslugiKlienta;
    UslugiKarnetu* m_uslugiKarnetu;
    UslugiZajec* m_uslugiZajec;
    UslugiRaportow* m_uslugiRaportow;
    
    // Główne layout
    QVBoxLayout* m_mainLayout;
    QScrollArea* m_scrollArea;
    QWidget* m_contentWidget;
    
    // Sekcje dashboard
    QGroupBox* m_statisticsGroup;
    QGroupBox* m_quickActionsGroup;
    QGroupBox* m_recentActivityGroup;
    QGroupBox* m_expiringMembershipsGroup;
    QGroupBox* m_upcomingClassesGroup;
    
    // Karty statystyk
    QFrame* m_clientsCard;
    QFrame* m_membershipsCard;
    QFrame* m_classesCard;
    QFrame* m_revenueCard;
    
    // Etykiety statystyk
    QLabel* m_clientsCountLabel;
    QLabel* m_clientsSubLabel;
    QLabel* m_membershipsCountLabel;
    QLabel* m_membershipsSubLabel;
    QLabel* m_classesCountLabel;
    QLabel* m_classesSubLabel;
    QLabel* m_revenueLabel;
    QLabel* m_revenueSubLabel;
    
    // Szybkie akcje
    QPushButton* m_addClientButton;
    QPushButton* m_addMembershipButton;
    QPushButton* m_addClassButton;
    QPushButton* m_generateReportButton;
    QPushButton* m_viewClientsButton;
    QPushButton* m_viewMembershipsButton;
    QPushButton* m_viewClassesButton;
    
    // Tabele aktywności
    QTableWidget* m_recentActivityTable;
    QTableWidget* m_expiringMembershipsTable;
    QTableWidget* m_upcomingClassesTable;
    
    // Pasek postępu dla celów miesięcznych
    QProgressBar* m_monthlyGoalProgress;
    QLabel* m_monthlyGoalLabel;
    
    // Timer do automatycznego odświeżania
    QTimer* m_refreshTimer;
    
    // Stany
    bool m_servicesInitialized;
    
    // Stałe
    static const int REFRESH_INTERVAL = 30000; // 30 sekund
    static const int RECENT_ITEMS_LIMIT = 10;
    static const int EXPIRING_DAYS_WARNING = 7;
};

#endif // DASHBOARDWIDGET_H