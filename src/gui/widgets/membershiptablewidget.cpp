// src/gui/widgets/membershiptablewidget.cpp
#include "membershiptablewidget.h"
#include "gui/dialogs/membershipdialog.h"

#include <QSortFilterProxyModel>
#include <QFileDialog>
#include <QProgressDialog>
#include <QDate>
#include <QDebug>

MembershipTableWidget::MembershipTableWidget(QWidget *parent)
    : QWidget(parent)
    , m_uslugiKarnetu(nullptr)
    , m_uslugiKlienta(nullptr)
    , m_servicesInitialized(false)
{
    setupUI();
    setupTable();
    setupToolbar();
    setupConnections();
    createContextMenu();
    
    // Wyłącz przyciski do czasu inicjalizacji serwisów
    m_addButton->setEnabled(false);
    m_editButton->setEnabled(false);
    m_deleteButton->setEnabled(false);
    m_renewButton->setEnabled(false);
    m_deactivateButton->setEnabled(false);
    m_refreshButton->setEnabled(false);
    m_exportButton->setEnabled(false);
    
    qDebug() << "MembershipTableWidget utworzony";
}

MembershipTableWidget::~MembershipTableWidget()
{
    // Automatyczne czyszczenie przez Qt parent-child system
}

void MembershipTableWidget::initializeServices(UslugiKarnetu* uslugiKarnetu, UslugiKlienta* uslugiKlienta)
{
    m_uslugiKarnetu = uslugiKarnetu;
    m_uslugiKlienta = uslugiKlienta;
    
    if (m_uslugiKarnetu && m_uslugiKlienta) {
        m_servicesInitialized = true;
        
        // Włącz przyciski
        m_addButton->setEnabled(true);
        m_refreshButton->setEnabled(true);
        m_exportButton->setEnabled(true);
        
        // Załaduj dane
        refresh();
        
        qDebug() << "MembershipTableWidget - serwisy zainicjalizowane";
    }
}

void MembershipTableWidget::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(6, 6, 6, 6);
    m_mainLayout->setSpacing(6);
    
    // Toolbar
    m_toolbarLayout = new QHBoxLayout();
    m_mainLayout->addLayout(m_toolbarLayout);
    
    // Tabela
    m_tableWidget = new QTableWidget(this);
    m_mainLayout->addWidget(m_tableWidget);
}

void MembershipTableWidget::setupTable()
{
    // Konfiguracja kolumn
    m_tableWidget->setColumnCount(COL_COUNT);
    
    QStringList headers;
    headers << "ID" << "Klient" << "ID Klienta" << "Typ karnetu" 
            << "Data rozpoczęcia" << "Data zakończenia" << "Cena" << "Status" << "Dni pozostałe";
    m_tableWidget->setHorizontalHeaderLabels(headers);
    
    // Właściwości tabeli
    m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_tableWidget->setAlternatingRowColors(true);
    m_tableWidget->setSortingEnabled(true);
    m_tableWidget->setContextMenuPolicy(Qt::DefaultContextMenu);
    
    // Nagłówki
    QHeaderView* header = m_tableWidget->horizontalHeader();
    header->setStretchLastSection(true);
    header->setDefaultSectionSize(120);
    
    // Szerokości kolumn
    m_tableWidget->setColumnWidth(COL_ID, 50);
    m_tableWidget->setColumnWidth(COL_CLIENT_NAME, 150);
    m_tableWidget->setColumnWidth(COL_CLIENT_ID, 80);
    m_tableWidget->setColumnWidth(COL_TYPE, 140);
    m_tableWidget->setColumnWidth(COL_START_DATE, 110);
    m_tableWidget->setColumnWidth(COL_END_DATE, 110);
    m_tableWidget->setColumnWidth(COL_PRICE, 80);
    m_tableWidget->setColumnWidth(COL_STATUS, 100);
    m_tableWidget->setColumnWidth(COL_DAYS_LEFT, 100);
    
    // Ukryj kolumny ID
    m_tableWidget->setColumnHidden(COL_ID, true);
    m_tableWidget->setColumnHidden(COL_CLIENT_ID, true);
}

void MembershipTableWidget::setupToolbar()
{
    // Wyszukiwanie
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("Wyszukaj karnety...");
    m_searchEdit->setMaximumWidth(200);
    m_toolbarLayout->addWidget(m_searchEdit);
    
    // Filtr główny
    m_filterCombo = new QComboBox(this);
    m_filterCombo->addItem("Wszystkie karnety", FILTER_ALL);
    m_filterCombo->addItem("Aktywne", FILTER_ACTIVE);
    m_filterCombo->addItem("Wygasłe", FILTER_EXPIRED);
    m_filterCombo->addItem("Wygasające (7 dni)", FILTER_EXPIRING);
    m_filterCombo->addItem("W tym miesiącu", FILTER_THIS_MONTH);
    m_filterCombo->addItem("Studenckie", FILTER_STUDENT);
    m_filterCombo->addItem("Standardowe", FILTER_STANDARD);
    m_filterCombo->setMaximumWidth(150);
    m_toolbarLayout->addWidget(m_filterCombo);
    
    // Filtr typu
    m_typeFilterCombo = new QComboBox(this);
    m_typeFilterCombo->addItem("Wszystkie typy", TYPE_ALL);
    m_typeFilterCombo->addItem("Miesięczne", TYPE_MONTHLY);
    m_typeFilterCombo->addItem("Kwartalne", TYPE_QUARTERLY);
    m_typeFilterCombo->addItem("Roczne", TYPE_YEARLY);
    m_typeFilterCombo->addItem("Studenckie", TYPE_STUDENT);
    m_typeFilterCombo->addItem("Standardowe", TYPE_STANDARD);
    m_typeFilterCombo->setMaximumWidth(120);
    m_toolbarLayout->addWidget(m_typeFilterCombo);
    
    // Filtry dat
    QLabel* fromLabel = new QLabel("Od:", this);
    m_fromDateEdit = new QDateEdit(this);
    m_fromDateEdit->setDate(QDate::currentDate().addMonths(-1));
    m_fromDateEdit->setMaximumWidth(120);
    
    QLabel* toLabel = new QLabel("Do:", this);
    m_toDateEdit = new QDateEdit(this);
    m_toDateEdit->setDate(QDate::currentDate().addMonths(1));
    m_toDateEdit->setMaximumWidth(120);
    
    m_toolbarLayout->addWidget(fromLabel);
    m_toolbarLayout->addWidget(m_fromDateEdit);
    m_toolbarLayout->addWidget(toLabel);
    m_toolbarLayout->addWidget(m_toDateEdit);
    
    m_toolbarLayout->addStretch();
    
    // Liczniki
    m_countLabel = new QLabel("Karnety: 0", this);
    m_countLabel->setStyleSheet("QLabel { font-weight: bold; }");
    m_toolbarLayout->addWidget(m_countLabel);
    
    m_revenueLabel = new QLabel("Przychód: 0 zł", this);
    m_revenueLabel->setStyleSheet("QLabel { font-weight: bold; color: #2E86AB; }");
    m_toolbarLayout->addWidget(m_revenueLabel);
    
    m_toolbarLayout->addSpacing(20);
    
    // Przyciski akcji
    m_addButton = new QPushButton("&Dodaj", this);
    m_addButton->setIcon(QIcon(":/icons/add.png"));
    m_addButton->setToolTip("Dodaj nowy karnet");
    m_toolbarLayout->addWidget(m_addButton);
    
    m_editButton = new QPushButton("&Edytuj", this);
    m_editButton->setIcon(QIcon(":/icons/edit.png"));
    m_editButton->setToolTip("Edytuj wybrany karnet");
    m_editButton->setEnabled(false);
    m_toolbarLayout->addWidget(m_editButton);
    
    m_renewButton = new QPushButton("&Przedłuż", this);
    m_renewButton->setIcon(QIcon(":/icons/renew.png"));
    m_renewButton->setToolTip("Przedłuż wybrany karnet");
    m_renewButton->setEnabled(false);
    m_toolbarLayout->addWidget(m_renewButton);
    
    m_deactivateButton = new QPushButton("&Dezaktywuj", this);
    m_deactivateButton->setIcon(QIcon(":/icons/deactivate.png"));
    m_deactivateButton->setToolTip("Dezaktywuj wybrany karnet");
    m_deactivateButton->setEnabled(false);
    m_toolbarLayout->addWidget(m_deactivateButton);
    
    m_deleteButton = new QPushButton("&Usuń", this);
    m_deleteButton->setIcon(QIcon(":/icons/delete.png"));
    m_deleteButton->setToolTip("Usuń wybrane karnety");
    m_deleteButton->setEnabled(false);
    m_toolbarLayout->addWidget(m_deleteButton);
    
    m_toolbarLayout->addSpacing(10);
    
    m_refreshButton = new QPushButton("&Odśwież", this);
    m_refreshButton->setIcon(QIcon(":/icons/refresh.png"));
    m_refreshButton->setToolTip("Odśwież listę karnetów");
    m_toolbarLayout->addWidget(m_refreshButton);
    
    m_exportButton = new QPushButton("&Eksportuj", this);
    m_exportButton->setIcon(QIcon(":/icons/export.png"));
    m_exportButton->setToolTip("Eksportuj dane karnetów");
    m_toolbarLayout->addWidget(m_exportButton);
}

void MembershipTableWidget::setupConnections()
{
    // Przyciski
    connect(m_addButton, &QPushButton::clicked, this, &MembershipTableWidget::addMembership);
    connect(m_editButton, &QPushButton::clicked, this, &MembershipTableWidget::editMembership);
    connect(m_deleteButton, &QPushButton::clicked, this, &MembershipTableWidget::deleteMembership);
    connect(m_renewButton, &QPushButton::clicked, this, &MembershipTableWidget::renewMembership);
    connect(m_deactivateButton, &QPushButton::clicked, this, &MembershipTableWidget::deactivateMembership);
    connect(m_refreshButton, &QPushButton::clicked, this, &MembershipTableWidget::refresh);
    connect(m_exportButton, &QPushButton::clicked, this, &MembershipTableWidget::exportMemberships);
    
    // Wyszukiwanie i filtrowanie
    connect(m_searchEdit, &QLineEdit::textChanged, this, &MembershipTableWidget::onSearchTextChanged);
    connect(m_filterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &MembershipTableWidget::onFilterChanged);
    connect(m_typeFilterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &MembershipTableWidget::onFilterChanged);
    connect(m_fromDateEdit, &QDateEdit::dateChanged, this, &MembershipTableWidget::onDateFilterChanged);
    connect(m_toDateEdit, &QDateEdit::dateChanged, this, &MembershipTableWidget::onDateFilterChanged);
    
    // Tabela
    connect(m_tableWidget, &QTableWidget::itemSelectionChanged, 
            this, &MembershipTableWidget::onTableSelectionChanged);
    connect(m_tableWidget, &QTableWidget::cellDoubleClicked, 
            this, &MembershipTableWidget::onTableDoubleClicked);
}

void MembershipTableWidget::createContextMenu()
{
    m_contextMenu = new QMenu(this);
    
    m_editAction = m_contextMenu->addAction(QIcon(":/icons/edit.png"), "&Edytuj", 
                                           this, &MembershipTableWidget::editMembership);
    m_renewAction = m_contextMenu->addAction(QIcon(":/icons/renew.png"), "&Przedłuż", 
                                            this, &MembershipTableWidget::renewMembership);
    m_deactivateAction = m_contextMenu->addAction(QIcon(":/icons/deactivate.png"), "&Dezaktywuj", 
                                                 this, &MembershipTableWidget::deactivateMembership);
    m_deleteAction = m_contextMenu->addAction(QIcon(":/icons/delete.png"), "&Usuń", 
                                             this, &MembershipTableWidget::deleteMembership);
    
    m_contextMenu->addSeparator();
    
    m_copyAction = m_contextMenu->addAction(QIcon(":/icons/copy.png"), "&Kopiuj", 
                                           this, &MembershipTableWidget::copyToClipboard);
    m_selectAllAction = m_contextMenu->addAction("Zaznacz &wszystko", 
                                                this, &MembershipTableWidget::selectAll);
    
    m_contextMenu->addSeparator();
    
    m_refreshAction = m_contextMenu->addAction(QIcon(":/icons/refresh.png"), "&Odśwież", 
                                              this, &MembershipTableWidget::refresh);
}

void MembershipTableWidget::refresh()
{
    if (!m_servicesInitialized || !m_uslugiKarnetu) {
        return;
    }
    
    try {
        loadMembershipData();
        qDebug() << "MembershipTableWidget - dane odświeżone, karnetów:" << m_allMemberships.size();
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Błąd", 
            QString("Nie udało się załadować danych karnetów:\n%1").arg(e.what()));
        qCritical() << "Błąd ładowania karnetów:" << e.what();
    }
}

void MembershipTableWidget::loadMembershipData()
{
    // Załaduj wszystkie karnety
    m_allMemberships = m_uslugiKarnetu->pobierzWszystkieKarnety();
    
    // Zastosuj filtry
    filterTable();
}

void MembershipTableWidget::populateTable(const std::vector<Karnet>& karnety)
{
    m_tableWidget->setRowCount(static_cast<int>(karnety.size()));
    
    double totalRevenue = 0.0;
    
    for (size_t i = 0; i < karnety.size(); ++i) {
        const Karnet& karnet = karnety[i];
        int row = static_cast<int>(i);
        
        // ID (ukryte)
        m_tableWidget->setItem(row, COL_ID, createTableItem(QString::number(karnet.pobierzId())));
        
        // Nazwa klienta
        QString clientName = getClientName(karnet.pobierzIdKlienta());
        m_tableWidget->setItem(row, COL_CLIENT_NAME, createTableItem(clientName));
        
        // ID Klienta (ukryte)
        m_tableWidget->setItem(row, COL_CLIENT_ID, createTableItem(QString::number(karnet.pobierzIdKlienta())));
        
        // Typ karnetu
        m_tableWidget->setItem(row, COL_TYPE, createTableItem(QString::fromStdString(karnet.pobierzTyp())));
        
        // Data rozpoczęcia
        m_tableWidget->setItem(row, COL_START_DATE, createTableItem(formatDate(karnet.pobierzDateRozpoczecia())));
        
        // Data zakończenia
        m_tableWidget->setItem(row, COL_END_DATE, createTableItem(formatDate(karnet.pobierzDateZakonczenia())));
        
        // Cena
        m_tableWidget->setItem(row, COL_PRICE, createTableItem(formatPrice(karnet.pobierzCene())));
        totalRevenue += karnet.pobierzCene();
        
        // Status z kolorami
        m_tableWidget->setItem(row, COL_STATUS, createStatusItem(karnet));
        
        // Dni pozostałe
        int daysLeft = karnet.ileDniPozostalo();
        QString daysText = daysLeft > 0 ? QString::number(daysLeft) : "Wygasł";
        QTableWidgetItem* daysItem = createTableItem(daysText);
        
        // Kolorowanie na podstawie dni pozostałych
        if (daysLeft <= 0) {
            daysItem->setBackground(QColor("#ffebee")); // Czerwony
        } else if (daysLeft <= 7) {
            daysItem->setBackground(QColor("#fff3e0")); // Pomarańczowy
        } else if (daysLeft <= 30) {
            daysItem->setBackground(QColor("#fff9c4")); // Żółty
        }
        
        m_tableWidget->setItem(row, COL_DAYS_LEFT, daysItem);
    }
    
    updateMembershipCount();
    m_revenueLabel->setText(QString("Przychód: %1 zł").arg(formatPrice(totalRevenue)));
}

void MembershipTableWidget::filterTable()
{
    m_filteredMemberships.clear();
    QString searchText = m_searchEdit->text().toLower();
    FilterType filterType = static_cast<FilterType>(m_filterCombo->currentData().toInt());
    TypeFilter typeFilter = static_cast<TypeFilter>(m_typeFilterCombo->currentData().toInt());
    
    QDate fromDate = m_fromDateEdit->date();
    QDate toDate = m_toDateEdit->date();
    QDate currentDate = QDate::currentDate();
    
    for (const Karnet& karnet : m_allMemberships) {
        bool matches = true;
        
        // Filtr tekstowy
        if (!searchText.isEmpty()) {
            QString clientName = getClientName(karnet.pobierzIdKlienta()).toLower();
            QString type = QString::fromStdString(karnet.pobierzTyp()).toLower();
            
            if (!clientName.contains(searchText) && !type.contains(searchText)) {
                matches = false;
            }
        }
        
        // Filtr główny
        if (matches) {
            switch (filterType) {
                case FILTER_ALL:
                    break;
                    
                case FILTER_ACTIVE:
                    if (!karnet.czyWazny()) {
                        matches = false;
                    }
                    break;
                    
                case FILTER_EXPIRED:
                    if (karnet.czyWazny()) {
                        matches = false;
                    }
                    break;
                    
                case FILTER_EXPIRING: {
                    int daysLeft = karnet.ileDniPozostalo();
                    if (daysLeft < 0 || daysLeft > EXPIRING_DAYS) {
                        matches = false;
                    }
                    break;
                }
                
                case FILTER_THIS_MONTH: {
                    QDate startDate = QDate::fromString(QString::fromStdString(karnet.pobierzDateRozpoczecia()), "yyyy-MM-dd");
                    if (!startDate.isValid() || 
                        startDate.year() != currentDate.year() || 
                        startDate.month() != currentDate.month()) {
                        matches = false;
                    }
                    break;
                }
                
                case FILTER_STUDENT:
                    if (!QString::fromStdString(karnet.pobierzTyp()).contains("student", Qt::CaseInsensitive)) {
                        matches = false;
                    }
                    break;
                    
                case FILTER_STANDARD:
                    if (!QString::fromStdString(karnet.pobierzTyp()).contains("normalny", Qt::CaseInsensitive)) {
                        matches = false;
                    }
                    break;
            }
        }
        
        // Filtr typu
        if (matches && typeFilter != TYPE_ALL) {
            QString type = QString::fromStdString(karnet.pobierzTyp()).toLower();
            
            switch (typeFilter) {
                case TYPE_MONTHLY:
                    if (!type.contains("miesieczny")) matches = false;
                    break;
                case TYPE_QUARTERLY:
                    if (!type.contains("kwartalny")) matches = false;
                    break;
                case TYPE_YEARLY:
                    if (!type.contains("roczny")) matches = false;
                    break;
                case TYPE_STUDENT:
                    if (!type.contains("student")) matches = false;
                    break;
                case TYPE_STANDARD:
                    if (!type.contains("normalny")) matches = false;
                    break;
            }
        }
        
        // Filtr dat
        if (matches) {
            QDate startDate = QDate::fromString(QString::fromStdString(karnet.pobierzDateRozpoczecia()), "yyyy-MM-dd");
            if (startDate.isValid() && (startDate < fromDate || startDate > toDate)) {
                matches = false;
            }
        }
        
        if (matches) {
            m_filteredMemberships.push_back(karnet);
        }
    }
    
    populateTable(m_filteredMemberships);
}

QString MembershipTableWidget::getClientName(int clientId)
{
    if (!m_uslugiKlienta) return "Nieznany";
    
    try {
        auto klient = m_uslugiKlienta->pobierzKlientaPoId(clientId);
        if (klient) {
            return QString::fromStdString(klient->pobierzPelneNazwisko());
        }
    } catch (const std::exception&) {
        // Ignoruj błędy
    }
    
    return QString("Klient ID: %1").arg(clientId);
}

QTableWidgetItem* MembershipTableWidget::createStatusItem(const Karnet& karnet)
{
    QString statusText;
    QColor backgroundColor;
    
    if (!karnet.pobierzCzyAktywny()) {
        statusText = "Nieaktywny";
        backgroundColor = QColor("#f5f5f5");
    } else if (karnet.czyWazny()) {
        int daysLeft = karnet.ileDniPozostalo();
        if (daysLeft <= 7) {
            statusText = "Wygasa";
            backgroundColor = QColor("#fff3e0"); // Pomarańczowy
        } else {
            statusText = "Aktywny";
            backgroundColor = QColor("#e8f5e8"); // Zielony
        }
    } else {
        statusText = "Wygasł";
        backgroundColor = QColor("#ffebee"); // Czerwony
    }
    
    QTableWidgetItem* item = createTableItem(statusText);
    item->setBackground(backgroundColor);
    return item;
}

void MembershipTableWidget::updateMembershipCount()
{
    int active = 0;
    for (const auto& karnet : m_filteredMemberships) {
        if (karnet.czyWazny()) {
            active++;
        }
    }
    
    m_countLabel->setText(QString("Karnety: %1/%2 (aktywne: %3)")
        .arg(m_filteredMemberships.size())
        .arg(m_allMemberships.size())
        .arg(active));
}

QString MembershipTableWidget::formatDate(const std::string& dateStr)
{
    if (dateStr.empty()) return "";
    
    QDate date = QDate::fromString(QString::fromStdString(dateStr), "yyyy-MM-dd");
    if (date.isValid()) {
        return date.toString("dd.MM.yyyy");
    }
    
    return QString::fromStdString(dateStr);
}

QString MembershipTableWidget::formatPrice(double price)
{
    return QString::number(price, 'f', 0) + " zł";
}

QTableWidgetItem* MembershipTableWidget::createTableItem(const QString& text)
{
    QTableWidgetItem* item = new QTableWidgetItem(text);
    item->setFlags(item->flags() & ~Qt::ItemIsEditable); // Tylko do odczytu
    return item;
}

Karnet* MembershipTableWidget::getSelectedMembership()
{
    QList<QTableWidgetItem*> selectedItems = m_tableWidget->selectedItems();
    if (selectedItems.isEmpty()) return nullptr;
    
    int row = selectedItems[0]->row();
    if (row < 0 || row >= static_cast<int>(m_filteredMemberships.size())) return nullptr;
    
    return &m_filteredMemberships[row];
}

QList<Karnet*> MembershipTableWidget::getSelectedMemberships()
{
    QList<Karnet*> result;
    QSet<int> selectedRows;
    
    for (QTableWidgetItem* item : m_tableWidget->selectedItems()) {
        selectedRows.insert(item->row());
    }
    
    for (int row : selectedRows) {
        if (row >= 0 && row < static_cast<int>(m_filteredMemberships.size())) {
            result.append(&m_filteredMemberships[row]);
        }
    }
    
    return result;
}

// === SLOTY AKCJI ===

void MembershipTableWidget::addMembership()
{
    // TODO: Implementacja po utworzeniu MembershipDialog
    QMessageBox::information(this, "Dodaj karnet", "Funkcja dodawania karnetu - w przygotowaniu");
}

void MembershipTableWidget::editMembership()
{
    // TODO: Implementacja po utworzeniu MembershipDialog
    QMessageBox::information(this, "Edytuj karnet", "Funkcja edycji karnetu - w przygotowaniu");
}

void MembershipTableWidget::deleteMembership()
{
    QList<Karnet*> selectedMemberships = getSelectedMemberships();
    if (selectedMemberships.isEmpty() || !m_uslugiKarnetu) return;
    
    QString message;
    if (selectedMemberships.size() == 1) {
        QString clientName = getClientName(selectedMemberships[0]->pobierzIdKlienta());
        message = QString("Czy na pewno chcesz usunąć karnet:\n%1 (%2)?")
            .arg(clientName)
            .arg(QString::fromStdString(selectedMemberships[0]->pobierzTyp()));
    } else {
        message = QString("Czy na pewno chcesz usunąć %1 wybranych karnetów?")
            .arg(selectedMemberships.size());
    }
    
    QMessageBox::StandardButton ret = QMessageBox::question(this,
        "Potwierdzenie usunięcia", message,
        QMessageBox::Yes | QMessageBox::No);
        
    if (ret != QMessageBox::Yes) return;
    
    // Usuń karnety
    QStringList errors;
    int deletedCount = 0;
    
    for (Karnet* karnet : selectedMemberships) {
        try {
            if (m_uslugiKarnetu->usunKarnet(karnet->pobierzId())) {
                emit membershipDeleted(karnet->pobierzId());
                deletedCount++;
            } else {
                errors << QString("Nie udało się usunąć karnetu ID: %1")
                    .arg(karnet->pobierzId());
            }
        } catch (const std::exception& e) {
            errors << QString("Błąd usuwania karnetu ID %1: %2")
                .arg(karnet->pobierzId())
                .arg(e.what());
        }
    }
    
    // Pokaż wyniki
    if (deletedCount > 0) {
        QMessageBox::information(this, "Usuwanie zakończone",
            QString("Usunięto %1 karnetów.").arg(deletedCount));
        refresh();
    }
    
    if (!errors.isEmpty()) {
        QMessageBox::warning(this, "Błędy usuwania",
            "Wystąpiły błędy:\n" + errors.join("\n"));
    }
}

void MembershipTableWidget::renewMembership()
{
    Karnet* karnet = getSelectedMembership();
    if (!karnet) return;
    
    QMessageBox::information(this, "Przedłuż karnet", "Funkcja przedłużania karnetu - w przygotowaniu");
}

void MembershipTableWidget::deactivateMembership()
{
    Karnet* karnet = getSelectedMembership();
    if (!karnet || !m_uslugiKarnetu) return;
    
    QString clientName = getClientName(karnet->pobierzIdKlienta());
    QMessageBox::StandardButton ret = QMessageBox::question(this,
        "Potwierdzenie dezaktywacji", 
        QString("Czy na pewno chcesz dezaktywować karnet:\n%1 (%2)?")
            .arg(clientName)
            .arg(QString::fromStdString(karnet->pobierzTyp())),
        QMessageBox::Yes | QMessageBox::No);
        
    if (ret != QMessageBox::Yes) return;
    
    try {
        Karnet aktualizowanyKarnet = *karnet;
        aktualizowanyKarnet.ustawCzyAktywny(false);
        
        if (m_uslugiKarnetu->aktualizujKarnet(aktualizowanyKarnet)) {
            QMessageBox::information(this, "Dezaktywacja", "Karnet został dezaktywowany.");
            refresh();
        } else {
            QMessageBox::warning(this, "Błąd", "Nie udało się dezaktywować karnetu.");
        }
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Błąd", 
            QString("Wystąpił błąd podczas dezaktywacji:\n%1").arg(e.what()));
    }
}

void MembershipTableWidget::exportMemberships()
{
    if (m_filteredMemberships.empty()) {
        QMessageBox::information(this, "Export", "Brak karnetów do eksportu.");
        return;
    }
    
    QString fileName = QFileDialog::getSaveFileName(this,
        "Eksportuj karnety", "karnety.csv", "Pliki CSV (*.csv)");
        
    if (fileName.isEmpty()) return;
    
    // TODO: Implementacja eksportu z wykorzystaniem istniejącej klasy EksportDanych
    QMessageBox::information(this, "Export", 
        QString("Export %1 karnetów do pliku %2\n\nFunkcja w przygotowaniu.")
        .arg(m_filteredMemberships.size()).arg(fileName));
}

// === POZOSTAŁE SLOTY ===

void MembershipTableWidget::contextMenuEvent(QContextMenuEvent *event)
{
    if (!m_servicesInitialized) return;
    
    QTableWidgetItem* item = m_tableWidget->itemAt(m_tableWidget->mapFromGlobal(event->globalPos()));
    
    // Włącz/wyłącz akcje w zależności od zaznaczenia
    bool hasSelection = !m_tableWidget->selectedItems().isEmpty();
    bool singleSelection = m_tableWidget->selectedItems().size() <= COL_COUNT;
    
    m_editAction->setEnabled(hasSelection && singleSelection);
    m_renewAction->setEnabled(hasSelection && singleSelection);
    m_deactivateAction->setEnabled(hasSelection && singleSelection);
    m_deleteAction->setEnabled(hasSelection);
    m_copyAction->setEnabled(hasSelection);
    
    m_contextMenu->exec(event->globalPos());
}

void MembershipTableWidget::onTableSelectionChanged()
{
    bool hasSelection = !m_tableWidget->selectedItems().isEmpty();
    bool singleSelection = m_tableWidget->selectedItems().size() <= COL_COUNT;
    
    m_editButton->setEnabled(hasSelection && singleSelection);
    m_renewButton->setEnabled(hasSelection && singleSelection);
    m_deactivateButton->setEnabled(hasSelection && singleSelection);
    m_deleteButton->setEnabled(hasSelection);
}

void MembershipTableWidget::onTableDoubleClicked(int row, int column)
{
    Q_UNUSED(column)
    
    if (row >= 0 && row < static_cast<int>(m_filteredMemberships.size())) {
        editMembership();
    }
}

void MembershipTableWidget::onSearchTextChanged(const QString& text)
{
    Q_UNUSED(text)
    filterTable();
}

void MembershipTableWidget::onFilterChanged()
{
    filterTable();
}

void MembershipTableWidget::onDateFilterChanged()
{
    filterTable();
}

void MembershipTableWidget::copyToClipboard()
{
    QList<Karnet*> selectedMemberships = getSelectedMemberships();
    if (selectedMemberships.isEmpty()) return;
    
    QStringList lines;
    lines << "Klient\tTyp\tRozpoczęcie\tZakończenie\tCena\tStatus";
    
    for (Karnet* karnet : selectedMemberships) {
        QString clientName = getClientName(karnet->pobierzIdKlienta());
        QString status = karnet->czyWazny() ? "Aktywny" : "Nieaktywny";
        
        lines << QString("%1\t%2\t%3\t%4\t%5\t%6")
            .arg(clientName)
            .arg(QString::fromStdString(karnet->pobierzTyp()))
            .arg(formatDate(karnet->pobierzDateRozpoczecia()))
            .arg(formatDate(karnet->pobierzDateZakonczenia()))
            .arg(formatPrice(karnet->pobierzCene()))
            .arg(status);
    }
    
    QApplication::clipboard()->setText(lines.join("\n"));
}

void MembershipTableWidget::selectAll()
{
    m_tableWidget->selectAll();
}

void MembershipTableWidget::searchMemberships()
{
    m_searchEdit->setFocus();
    m_searchEdit->selectAll();
}

void MembershipTableWidget::clearSearch()
{
    m_searchEdit->clear();
    m_filterCombo->setCurrentIndex(0);
    m_typeFilterCombo->setCurrentIndex(0);
}