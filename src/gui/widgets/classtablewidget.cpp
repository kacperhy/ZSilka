// src/gui/widgets/classtablewidget.cpp
#include "classtablewidget.h"
#include "gui/dialogs/classdialog.h"
#include "gui/dialogs/reservationdialog.h"

#include <QFileDialog>
#include <QProgressDialog>
#include <QDate>
#include <QDebug>
#include <QGroupBox>

ClassTableWidget::ClassTableWidget(QWidget *parent)
    : QWidget(parent)
    , m_uslugiZajec(nullptr)
    , m_servicesInitialized(false)
    , m_selectedClassId(-1)
{
    setupUI();
    setupClassesTable();
    setupReservationsTable();
    setupToolbar();
    setupConnections();
    createContextMenus();
    
    // Wyłącz przyciski do czasu inicjalizacji serwisów
    m_addButton->setEnabled(false);
    m_editButton->setEnabled(false);
    m_deleteButton->setEnabled(false);
    m_duplicateButton->setEnabled(false);
    m_viewReservationsButton->setEnabled(false);
    m_addReservationButton->setEnabled(false);
    m_cancelReservationButton->setEnabled(false);
    m_refreshButton->setEnabled(false);
    m_exportButton->setEnabled(false);
    
    qDebug() << "ClassTableWidget utworzony";
}

ClassTableWidget::~ClassTableWidget()
{
    // Automatyczne czyszczenie przez Qt parent-child system
}

void ClassTableWidget::initializeServices(UslugiZajec* uslugiZajec)
{
    m_uslugiZajec = uslugiZajec;
    
    if (m_uslugiZajec) {
        m_servicesInitialized = true;
        
        // Włącz przyciski
        m_addButton->setEnabled(true);
        m_refreshButton->setEnabled(true);
        m_exportButton->setEnabled(true);
        
        // Załaduj dane
        refresh();
        
        qDebug() << "ClassTableWidget - serwisy zainicjalizowane";
    }
}

void ClassTableWidget::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(6, 6, 6, 6);
    m_mainLayout->setSpacing(6);
    
    // Toolbar
    m_toolbarLayout = new QHBoxLayout();
    m_mainLayout->addLayout(m_toolbarLayout);
    
    // Splitter dla podział na zajęcia i rezerwacje
    m_splitter = new QSplitter(Qt::Vertical, this);
    
    // Widget zajęć
    m_classesWidget = new QWidget();
    QVBoxLayout* classesLayout = new QVBoxLayout(m_classesWidget);
    classesLayout->setContentsMargins(0, 0, 0, 0);
    
    QGroupBox* classesGroup = new QGroupBox("Zajęcia", m_classesWidget);
    QVBoxLayout* classesGroupLayout = new QVBoxLayout(classesGroup);
    
    m_classesTable = new QTableWidget();
    classesGroupLayout->addWidget(m_classesTable);
    classesLayout->addWidget(classesGroup);
    
    // Widget rezerwacji
    m_reservationsWidget = new QWidget();
    QVBoxLayout* reservationsLayout = new QVBoxLayout(m_reservationsWidget);
    reservationsLayout->setContentsMargins(0, 0, 0, 0);
    
    QGroupBox* reservationsGroup = new QGroupBox("Rezerwacje", m_reservationsWidget);
    QVBoxLayout* reservationsGroupLayout = new QVBoxLayout(reservationsGroup);
    
    m_reservationsTable = new QTableWidget();
    reservationsGroupLayout->addWidget(m_reservationsTable);
    reservationsLayout->addWidget(reservationsGroup);
    
    // Dodaj do splitter
    m_splitter->addWidget(m_classesWidget);
    m_splitter->addWidget(m_reservationsWidget);
    m_splitter->setStretchFactor(0, 2); // Zajęcia 2/3
    m_splitter->setStretchFactor(1, 1); // Rezerwacje 1/3
    
    m_mainLayout->addWidget(m_splitter);
}

void ClassTableWidget::setupClassesTable()
{
    // Konfiguracja kolumn
    m_classesTable->setColumnCount(CLASS_COL_COUNT);
    
    QStringList headers;
    headers << "ID" << "Nazwa zajęć" << "Trener" << "Data" << "Godzina" 
            << "Czas trwania" << "Max uczestników" << "Rezerwacje" << "Zapełnienie" << "Opis";
    m_classesTable->setHorizontalHeaderLabels(headers);
    
    // Właściwości tabeli
    m_classesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_classesTable->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_classesTable->setAlternatingRowColors(true);
    m_classesTable->setSortingEnabled(true);
    m_classesTable->setContextMenuPolicy(Qt::DefaultContextMenu);
    
    // Nagłówki
    QHeaderView* header = m_classesTable->horizontalHeader();
    header->setStretchLastSection(true);
    header->setDefaultSectionSize(100);
    
    // Szerokości kolumn
    m_classesTable->setColumnWidth(CLASS_COL_ID, 50);
    m_classesTable->setColumnWidth(CLASS_COL_NAME, 150);
    m_classesTable->setColumnWidth(CLASS_COL_TRAINER, 120);
    m_classesTable->setColumnWidth(CLASS_COL_DATE, 100);
    m_classesTable->setColumnWidth(CLASS_COL_TIME, 80);
    m_classesTable->setColumnWidth(CLASS_COL_DURATION, 100);
    m_classesTable->setColumnWidth(CLASS_COL_MAX_PARTICIPANTS, 120);
    m_classesTable->setColumnWidth(CLASS_COL_RESERVATIONS, 100);
    m_classesTable->setColumnWidth(CLASS_COL_CAPACITY, 100);
    
    // Ukryj kolumnę ID
    m_classesTable->setColumnHidden(CLASS_COL_ID, true);
}

void ClassTableWidget::setupReservationsTable()
{
    // Konfiguracja kolumn
    m_reservationsTable->setColumnCount(RES_COL_COUNT);
    
    QStringList headers;
    headers << "ID" << "Klient" << "ID Klienta" << "Zajęcia" << "Data rezerwacji" << "Status";
    m_reservationsTable->setHorizontalHeaderLabels(headers);
    
    // Właściwości tabeli
    m_reservationsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_reservationsTable->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_reservationsTable->setAlternatingRowColors(true);
    m_reservationsTable->setSortingEnabled(true);
    m_reservationsTable->setContextMenuPolicy(Qt::DefaultContextMenu);
    
    // Nagłówki
    QHeaderView* header = m_reservationsTable->horizontalHeader();
    header->setStretchLastSection(true);
    header->setDefaultSectionSize(120);
    
    // Szerokości kolumn
    m_reservationsTable->setColumnWidth(RES_COL_ID, 50);
    m_reservationsTable->setColumnWidth(RES_COL_CLIENT_NAME, 150);
    m_reservationsTable->setColumnWidth(RES_COL_CLIENT_ID, 80);
    m_reservationsTable->setColumnWidth(RES_COL_CLASS_NAME, 150);
    m_reservationsTable->setColumnWidth(RES_COL_RESERVATION_DATE, 130);
    m_reservationsTable->setColumnWidth(RES_COL_STATUS, 100);
    
    // Ukryj kolumny ID
    m_reservationsTable->setColumnHidden(RES_COL_ID, true);
    m_reservationsTable->setColumnHidden(RES_COL_CLIENT_ID, true);
}

void ClassTableWidget::setupToolbar()
{
    // Wyszukiwanie
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("Wyszukaj zajęcia...");
    m_searchEdit->setMaximumWidth(200);
    m_toolbarLayout->addWidget(m_searchEdit);
    
    // Filtr główny
    m_filterCombo = new QComboBox(this);
    m_filterCombo->addItem("Wszystkie zajęcia", FILTER_ALL);
    m_filterCombo->addItem("Dzisiaj", FILTER_TODAY);
    m_filterCombo->addItem("W tym tygodniu", FILTER_THIS_WEEK);
    m_filterCombo->addItem("W tym miesiącu", FILTER_THIS_MONTH);
    m_filterCombo->addItem("Nadchodzące", FILTER_UPCOMING);
    m_filterCombo->addItem("Przeszłe", FILTER_PAST);
    m_filterCombo->addItem("Zapełnione", FILTER_FULL);
    m_filterCombo->addItem("Dostępne miejsca", FILTER_AVAILABLE);
    m_filterCombo->setMaximumWidth(150);
    m_toolbarLayout->addWidget(m_filterCombo);
    
    // Filtr trenera
    m_trainerFilterCombo = new QComboBox(this);
    m_trainerFilterCombo->addItem("Wszyscy trenerzy", "");
    m_trainerFilterCombo->setMaximumWidth(150);
    m_toolbarLayout->addWidget(m_trainerFilterCombo);
    
    // Filtry dat
    QLabel* fromLabel = new QLabel("Od:", this);
    m_fromDateEdit = new QDateEdit(this);
    m_fromDateEdit->setDate(QDate::currentDate());
    m_fromDateEdit->setMaximumWidth(120);
    
    QLabel* toLabel = new QLabel("Do:", this);
    m_toDateEdit = new QDateEdit(this);
    m_toDateEdit->setDate(QDate::currentDate().addDays(7));
    m_toDateEdit->setMaximumWidth(120);
    
    m_toolbarLayout->addWidget(fromLabel);
    m_toolbarLayout->addWidget(m_fromDateEdit);
    m_toolbarLayout->addWidget(toLabel);
    m_toolbarLayout->addWidget(m_toDateEdit);
    
    m_toolbarLayout->addStretch();
    
    // Liczniki
    m_classCountLabel = new QLabel("Zajęcia: 0", this);
    m_classCountLabel->setStyleSheet("QLabel { font-weight: bold; }");
    m_toolbarLayout->addWidget(m_classCountLabel);
    
    m_reservationCountLabel = new QLabel("Rezerwacje: 0", this);
    m_reservationCountLabel->setStyleSheet("QLabel { font-weight: bold; color: #2E86AB; }");
    m_toolbarLayout->addWidget(m_reservationCountLabel);
    
    m_toolbarLayout->addSpacing(20);
    
    // Przyciski akcji zajęć
    m_addButton = new QPushButton("&Dodaj zajęcia", this);
    m_addButton->setIcon(QIcon(":/icons/add.png"));
    m_addButton->setToolTip("Dodaj nowe zajęcia");
    m_toolbarLayout->addWidget(m_addButton);
    
    m_editButton = new QPushButton("&Edytuj", this);
    m_editButton->setIcon(QIcon(":/icons/edit.png"));
    m_editButton->setToolTip("Edytuj wybrane zajęcia");
    m_editButton->setEnabled(false);
    m_toolbarLayout->addWidget(m_editButton);
    
    m_duplicateButton = new QPushButton("&Duplikuj", this);
    m_duplicateButton->setIcon(QIcon(":/icons/copy.png"));
    m_duplicateButton->setToolTip("Duplikuj wybrane zajęcia");
    m_duplicateButton->setEnabled(false);
    m_toolbarLayout->addWidget(m_duplicateButton);
    
    m_deleteButton = new QPushButton("&Usuń", this);
    m_deleteButton->setIcon(QIcon(":/icons/delete.png"));
    m_deleteButton->setToolTip("Usuń wybrane zajęcia");
    m_deleteButton->setEnabled(false);
    m_toolbarLayout->addWidget(m_deleteButton);
    
    m_toolbarLayout->addSpacing(10);
    
    // Przyciski akcji rezerwacji
    m_viewReservationsButton = new QPushButton("&Pokaż rezerwacje", this);
    m_viewReservationsButton->setIcon(QIcon(":/icons/view.png"));
    m_viewReservationsButton->setToolTip("Pokaż rezerwacje wybranych zajęć");
    m_viewReservationsButton->setEnabled(false);
    m_toolbarLayout->addWidget(m_viewReservationsButton);
    
    m_addReservationButton = new QPushButton("&Rezerwuj", this);
    m_addReservationButton->setIcon(QIcon(":/icons/reservation.png"));
    m_addReservationButton->setToolTip("Dodaj nową rezerwację");
    m_addReservationButton->setEnabled(false);
    m_toolbarLayout->addWidget(m_addReservationButton);
    
    m_cancelReservationButton = new QPushButton("&Anuluj", this);
    m_cancelReservationButton->setIcon(QIcon(":/icons/cancel.png"));
    m_cancelReservationButton->setToolTip("Anuluj wybrane rezerwacje");
    m_cancelReservationButton->setEnabled(false);
    m_toolbarLayout->addWidget(m_cancelReservationButton);
    
    m_toolbarLayout->addSpacing(10);
    
    m_refreshButton = new QPushButton("&Odśwież", this);
    m_refreshButton->setIcon(QIcon(":/icons/refresh.png"));
    m_refreshButton->setToolTip("Odśwież dane");
    m_toolbarLayout->addWidget(m_refreshButton);
    
    m_exportButton = new QPushButton("&Eksportuj", this);
    m_exportButton->setIcon(QIcon(":/icons/export.png"));
    m_exportButton->setToolTip("Eksportuj dane");
    m_toolbarLayout->addWidget(m_exportButton);
}

void ClassTableWidget::setupConnections()
{
    // Przyciski zajęć
    connect(m_addButton, &QPushButton::clicked, this, &ClassTableWidget::addClass);
    connect(m_editButton, &QPushButton::clicked, this, &ClassTableWidget::editClass);
    connect(m_deleteButton, &QPushButton::clicked, this, &ClassTableWidget::deleteClass);
    connect(m_duplicateButton, &QPushButton::clicked, this, &ClassTableWidget::duplicateClass);
    connect(m_refreshButton, &QPushButton::clicked, this, &ClassTableWidget::refresh);
    connect(m_exportButton, &QPushButton::clicked, this, &ClassTableWidget::exportClasses);
    
    // Przyciski rezerwacji
    connect(m_viewReservationsButton, &QPushButton::clicked, this, &ClassTableWidget::viewReservations);
    connect(m_addReservationButton, &QPushButton::clicked, this, &ClassTableWidget::addReservation);
    connect(m_cancelReservationButton, &QPushButton::clicked, this, &ClassTableWidget::cancelReservation);
    
    // Wyszukiwanie i filtrowanie
    connect(m_searchEdit, &QLineEdit::textChanged, this, &ClassTableWidget::onSearchTextChanged);
    connect(m_filterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &ClassTableWidget::onFilterChanged);
    connect(m_trainerFilterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &ClassTableWidget::onFilterChanged);
    connect(m_fromDateEdit, &QDateEdit::dateChanged, this, &ClassTableWidget::onDateFilterChanged);
    connect(m_toDateEdit, &QDateEdit::dateChanged, this, &ClassTableWidget::onDateFilterChanged);
    
    // Tabele
    connect(m_classesTable, &QTableWidget::itemSelectionChanged, 
            this, &ClassTableWidget::onTableSelectionChanged);
    connect(m_classesTable, &QTableWidget::cellDoubleClicked, 
            this, &ClassTableWidget::onTableDoubleClicked);
    connect(m_classesTable, &QTableWidget::itemSelectionChanged, 
            this, &ClassTableWidget::onClassSelectionChanged);
            
    connect(m_reservationsTable, &QTableWidget::itemSelectionChanged, 
            this, &ClassTableWidget::onReservationTableSelectionChanged);
    connect(m_reservationsTable, &QTableWidget::cellDoubleClicked, 
            this, &ClassTableWidget::onReservationTableDoubleClicked);
}

void ClassTableWidget::createContextMenus()
{
    // Context menu dla zajęć
    m_classContextMenu = new QMenu(this);
    
    m_editClassAction = m_classContextMenu->addAction(QIcon(":/icons/edit.png"), "&Edytuj", 
                                                     this, &ClassTableWidget::editClass);
    m_duplicateClassAction = m_classContextMenu->addAction(QIcon(":/icons/copy.png"), "&Duplikuj", 
                                                          this, &ClassTableWidget::duplicateClass);
    m_deleteClassAction = m_classContextMenu->addAction(QIcon(":/icons/delete.png"), "&Usuń", 
                                                        this, &ClassTableWidget::deleteClass);
    
    m_classContextMenu->addSeparator();
    
    m_viewReservationsAction = m_classContextMenu->addAction(QIcon(":/icons/view.png"), "&Pokaż rezerwacje", 
                                                            this, &ClassTableWidget::viewReservations);
    
    m_classContextMenu->addSeparator();
    
    m_copyClassAction = m_classContextMenu->addAction(QIcon(":/icons/copy.png"), "&Kopiuj", 
                                                     this, &ClassTableWidget::copyToClipboard);
    m_selectAllClassesAction = m_classContextMenu->addAction("Zaznacz &wszystko", 
                                                           this, &ClassTableWidget::selectAll);
    
    // Context menu dla rezerwacji
    m_reservationContextMenu = new QMenu(this);
    
    m_addReservationAction = m_reservationContextMenu->addAction(QIcon(":/icons/add.png"), "&Dodaj rezerwację", 
                                                                this, &ClassTableWidget::addReservation);
    m_cancelReservationAction = m_reservationContextMenu->addAction(QIcon(":/icons/cancel.png"), "&Anuluj rezerwację", 
                                                                   this, &ClassTableWidget::cancelReservation);
    
    m_reservationContextMenu->addSeparator();
    
    m_copyReservationAction = m_reservationContextMenu->addAction(QIcon(":/icons/copy.png"), "&Kopiuj", 
                                                                 this, &ClassTableWidget::copyToClipboard);
    m_selectAllReservationsAction = m_reservationContextMenu->addAction("Zaznacz &wszystko", 
                                                                       this, &ClassTableWidget::selectAll);
    
    m_reservationContextMenu->addSeparator();
    
    m_refreshAction = m_reservationContextMenu->addAction(QIcon(":/icons/refresh.png"), "&Odśwież", 
                                                         this, &ClassTableWidget::refresh);
}

void ClassTableWidget::refresh()
{
    if (!m_servicesInitialized || !m_uslugiZajec) {
        return;
    }
    
    try {
        loadClassData();
        loadReservationData();
        qDebug() << "ClassTableWidget - dane odświeżone, zajęć:" << m_allClasses.size() 
                 << ", rezerwacji:" << m_allReservations.size();
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Błąd", 
            QString("Nie udało się załadować danych zajęć:\n%1").arg(e.what()));
        qCritical() << "Błąd ładowania zajęć:" << e.what();
    }
}

void ClassTableWidget::loadClassData()
{
    // Załaduj wszystkie zajęcia
    m_allClasses = m_uslugiZajec->pobierzWszystkieZajecia();
    
    // Aktualizuj filtr trenerów
    QSet<QString> trainers;
    for (const auto& zajecia : m_allClasses) {
        trainers.insert(QString::fromStdString(zajecia.pobierzTrenera()));
    }
    
    QString currentTrainer = m_trainerFilterCombo->currentData().toString();
    m_trainerFilterCombo->clear();
    m_trainerFilterCombo->addItem("Wszyscy trenerzy", "");
    
    for (const QString& trainer : trainers) {
        m_trainerFilterCombo->addItem(trainer, trainer);
    }
    
    // Przywróć poprzedni wybór trenera
    int index = m_trainerFilterCombo->findData(currentTrainer);
    if (index >= 0) {
        m_trainerFilterCombo->setCurrentIndex(index);
    }
    
    // Zastosuj filtry
    filterClassesTable();
}

void ClassTableWidget::loadReservationData()
{
    // Załaduj wszystkie rezerwacje
    m_allReservations = m_uslugiZajec->pobierzWszystkieRezerwacje();
    
    // Filtruj rezerwacje dla wybranego zajęcia
    if (m_selectedClassId > 0) {
        m_filteredReservations.clear();
        for (const auto& rezerwacja : m_allReservations) {
            if (rezerwacja.pobierzIdZajec() == m_selectedClassId) {
                m_filteredReservations.push_back(rezerwacja);
            }
        }
    } else {
        m_filteredReservations = m_allReservations;
    }
    
    populateReservationsTable(m_filteredReservations);
}

void ClassTableWidget::populateClassesTable(const std::vector<Zajecia>& zajecia)
{
    m_classesTable->setRowCount(static_cast<int>(zajecia.size()));
    
    for (size_t i = 0; i < zajecia.size(); ++i) {
        const Zajecia& zajecie = zajecia[i];
        int row = static_cast<int>(i);
        
        // ID (ukryte)
        m_classesTable->setItem(row, CLASS_COL_ID, createTableItem(QString::number(zajecie.pobierzId())));
        
        // Nazwa
        m_classesTable->setItem(row, CLASS_COL_NAME, createTableItem(QString::fromStdString(zajecie.pobierzNazwe())));
        
        // Trener
        m_classesTable->setItem(row, CLASS_COL_TRAINER, createTableItem(QString::fromStdString(zajecie.pobierzTrenera())));
        
        // Data
        m_classesTable->setItem(row, CLASS_COL_DATE, createTableItem(formatDate(zajecie.pobierzDate())));
        
        // Godzina
        m_classesTable->setItem(row, CLASS_COL_TIME, createTableItem(formatTime(zajecie.pobierzCzas())));
        
        // Czas trwania
        m_classesTable->setItem(row, CLASS_COL_DURATION, createTableItem(formatDuration(zajecie.pobierzCzasTrwania())));
        
        // Maksymalna liczba uczestników
        m_classesTable->setItem(row, CLASS_COL_MAX_PARTICIPANTS, createTableItem(QString::number(zajecie.pobierzMaksUczestnikow())));
        
        // Liczba rezerwacji
        int reservationCount = m_uslugiZajec->pobierzRezerwacjeZajec(zajecie.pobierzId()).size();
        m_classesTable->setItem(row, CLASS_COL_RESERVATIONS, createTableItem(QString::number(reservationCount)));
        
        // Zapełnienie z kolorem
        m_classesTable->setItem(row, CLASS_COL_CAPACITY, createCapacityItem(zajecie));
        
        // Opis (skrócony)
        QString opis = QString::fromStdString(zajecie.pobierzOpis());
        if (opis.length() > 50) {
            opis = opis.left(47) + "...";
        }
        m_classesTable->setItem(row, CLASS_COL_DESCRIPTION, createTableItem(opis));
    }
    
    updateClassCount();
}

void ClassTableWidget::populateReservationsTable(const std::vector<Rezerwacja>& rezerwacje)
{
    m_reservationsTable->setRowCount(static_cast<int>(rezerwacje.size()));
    
    for (size_t i = 0; i < rezerwacje.size(); ++i) {
        const Rezerwacja& rezerwacja = rezerwacje[i];
        int row = static_cast<int>(i);
        
        // ID (ukryte)
        m_reservationsTable->setItem(row, RES_COL_ID, createTableItem(QString::number(rezerwacja.pobierzId())));
        
        // Nazwa klienta
        QString clientName = getClientName(rezerwacja.pobierzIdKlienta());
        m_reservationsTable->setItem(row, RES_COL_CLIENT_NAME, createTableItem(clientName));
        
        // ID Klienta (ukryte)
        m_reservationsTable->setItem(row, RES_COL_CLIENT_ID, createTableItem(QString::number(rezerwacja.pobierzIdKlienta())));
        
        // Nazwa zajęć
        QString className = "Nieznane";
        auto zajecia = m_uslugiZajec->pobierzZajeciaPoId(rezerwacja.pobierzIdZajec());
        if (zajecia) {
            className = QString::fromStdString(zajecia->pobierzNazwe());
        }
        m_reservationsTable->setItem(row, RES_COL_CLASS_NAME, createTableItem(className));
        
        // Data rezerwacji
        m_reservationsTable->setItem(row, RES_COL_RESERVATION_DATE, createTableItem(formatDate(rezerwacja.pobierzDateRezerwacji())));
        
        // Status z kolorem
        QString status = QString::fromStdString(rezerwacja.pobierzStatus());
        QTableWidgetItem* statusItem = createTableItem(status);
        
        if (status == "potwierdzona") {
            statusItem->setBackground(QColor("#e8f5e8")); // Zielony
        } else if (status == "anulowana") {
            statusItem->setBackground(QColor("#ffebee")); // Czerwony
        }
        
        m_reservationsTable->setItem(row, RES_COL_STATUS, statusItem);
    }
    
    updateReservationCount();
}

QTableWidgetItem* ClassTableWidget::createCapacityItem(const Zajecia& zajecia)
{
    int maxParticipants = zajecia.pobierzMaksUczestnikow();
    int currentReservations = m_uslugiZajec->pobierzRezerwacjeZajec(zajecia.pobierzId()).size();
    double percentage = maxParticipants > 0 ? (double(currentReservations) / maxParticipants) * 100.0 : 0.0;
    
    QString text = QString("%1/%2 (%3%)")
        .arg(currentReservations)
        .arg(maxParticipants)
        .arg(percentage, 0, 'f', 0);
    
    QTableWidgetItem* item = createTableItem(text);
    
    // Kolorowanie na podstawie zapełnienia
    if (percentage >= 100) {
        item->setBackground(QColor("#ffebee")); // Czerwony - zapełnione
    } else if (percentage >= 80) {
        item->setBackground(QColor("#fff3e0")); // Pomarańczowy - prawie zapełnione
    } else if (percentage >= 50) {
        item->setBackground(QColor("#fff9c4")); // Żółty - pół zapełnienia
    } else {
        item->setBackground(QColor("#e8f5e8")); // Zielony - dużo miejsca
    }
    
    return item;
}

void ClassTableWidget::filterClassesTable()
{
    m_filteredClasses.clear();
    QString searchText = m_searchEdit->text().toLower();
    FilterType filterType = static_cast<FilterType>(m_filterCombo->currentData().toInt());
    QString selectedTrainer = m_trainerFilterCombo->currentData().toString();
    
    QDate fromDate = m_fromDateEdit->date();
    QDate toDate = m_toDateEdit->date();
    QDate currentDate = QDate::currentDate();
    
    for (const Zajecia& zajecia : m_allClasses) {
        bool matches = true;
        
        // Filtr tekstowy
        if (!searchText.isEmpty()) {
            QString fullText = QString("%1 %2 %3")
                .arg(QString::fromStdString(zajecia.pobierzNazwe()).toLower())
                .arg(QString::fromStdString(zajecia.pobierzTrenera()).toLower())
                .arg(QString::fromStdString(zajecia.pobierzOpis()).toLower());
                
            if (!fullText.contains(searchText)) {
                matches = false;
            }
        }
        
        // Filtr trenera
        if (matches && !selectedTrainer.isEmpty()) {
            if (QString::fromStdString(zajecia.pobierzTrenera()) != selectedTrainer) {
                matches = false;
            }
        }
        
        // Filtr dat
        if (matches) {
            QDate classDate = QDate::fromString(QString::fromStdString(zajecia.pobierzDate()), "yyyy-MM-dd");
            if (classDate.isValid() && (classDate < fromDate || classDate > toDate)) {
                matches = false;
            }
        }
        
        // Filtr główny
        if (matches) {
            QDate classDate = QDate::fromString(QString::fromStdString(zajecia.pobierzDate()), "yyyy-MM-dd");
            
            switch (filterType) {
                case FILTER_ALL:
                    break;
                    
                case FILTER_TODAY:
                    if (classDate != currentDate) {
                        matches = false;
                    }
                    break;
                    
                case FILTER_THIS_WEEK: {
                    QDate startOfWeek = currentDate.addDays(-(currentDate.dayOfWeek() - 1));
                    QDate endOfWeek = startOfWeek.addDays(6);
                    if (classDate < startOfWeek || classDate > endOfWeek) {
                        matches = false;
                    }
                    break;
                }
                
                case FILTER_THIS_MONTH:
                    if (classDate.year() != currentDate.year() || classDate.month() != currentDate.month()) {
                        matches = false;
                    }
                    break;
                    
                case FILTER_UPCOMING:
                    if (classDate < currentDate) {
                        matches = false;
                    }
                    break;
                    
                case FILTER_PAST:
                    if (classDate >= currentDate) {
                        matches = false;
                    }
                    break;
                    
                case FILTER_FULL: {
                    int maxParticipants = zajecia.pobierzMaksUczestnikow();
                    int currentReservations = m_uslugiZajec->pobierzRezerwacjeZajec(zajecia.pobierzId()).size();
                    if (currentReservations < maxParticipants) {
                        matches = false;
                    }
                    break;
                }
                
                case FILTER_AVAILABLE: {
                    int maxParticipants = zajecia.pobierzMaksUczestnikow();
                    int currentReservations = m_uslugiZajec->pobierzRezerwacjeZajec(zajecia.pobierzId()).size();
                    if (currentReservations >= maxParticipants) {
                        matches = false;
                    }
                    break;
                }
            }
        }
        
        if (matches) {
            m_filteredClasses.push_back(zajecia);
        }
    }
    
    populateClassesTable(m_filteredClasses);
}

void ClassTableWidget::updateClassCount()
{
    m_classCountLabel->setText(QString("Zajęcia: %1/%2")
        .arg(m_filteredClasses.size())
        .arg(m_allClasses.size()));
}

void ClassTableWidget::updateReservationCount()
{
    int confirmed = 0;
    for (const auto& rezerwacja : m_filteredReservations) {
        if (rezerwacja.pobierzStatus() == "potwierdzona") {
            confirmed++;
        }
    }
    
    m_reservationCountLabel->setText(QString("Rezerwacje: %1 (potwierdzone: %2)")
        .arg(m_filteredReservations.size())
        .arg(confirmed));
}

QString ClassTableWidget::getClientName(int clientId)
{
    // TODO: Pobrać z serwisu klientów gdy będzie dostępny
    return QString("Klient ID: %1").arg(clientId);
}

QString ClassTableWidget::formatDate(const std::string& dateStr)
{
    if (dateStr.empty()) return "";
    
    QDate date = QDate::fromString(QString::fromStdString(dateStr), "yyyy-MM-dd");
    if (date.isValid()) {
        return date.toString("dd.MM.yyyy");
    }
    
    return QString::fromStdString(dateStr);
}

QString ClassTableWidget::formatTime(const std::string& timeStr)
{
    return QString::fromStdString(timeStr);
}

QString ClassTableWidget::formatDuration(int minutes)
{
    if (minutes < 60) {
        return QString("%1 min").arg(minutes);
    } else {
        int hours = minutes / 60;
        int remainingMinutes = minutes % 60;
        if (remainingMinutes == 0) {
            return QString("%1 h").arg(hours);
        } else {
            return QString("%1h %2min").arg(hours).arg(remainingMinutes);
        }
    }
}

QTableWidgetItem* ClassTableWidget::createTableItem(const QString& text)
{
    QTableWidgetItem* item = new QTableWidgetItem(text);
    item->setFlags(item->flags() & ~Qt::ItemIsEditable); // Tylko do odczytu
    return item;
}

Zajecia* ClassTableWidget::getSelectedClass()
{
    QList<QTableWidgetItem*> selectedItems = m_classesTable->selectedItems();
    if (selectedItems.isEmpty()) return nullptr;
    
    int row = selectedItems[0]->row();
    if (row < 0 || row >= static_cast<int>(m_filteredClasses.size())) return nullptr;
    
    return &m_filteredClasses[row];
}

// === SLOTY AKCJI ===

void ClassTableWidget::addClass()
{
    // TODO: Implementacja po utworzeniu ClassDialog
    QMessageBox::information(this, "Dodaj zajęcia", "Funkcja dodawania zajęć - w przygotowaniu");
}

void ClassTableWidget::editClass()
{
    // TODO: Implementacja po utworzeniu ClassDialog
    QMessageBox::information(this, "Edytuj zajęcia", "Funkcja edycji zajęć - w przygotowaniu");
}

void ClassTableWidget::deleteClass()
{
    // TODO: Implementacja usuwania zajęć
    QMessageBox::information(this, "Usuń zajęcia", "Funkcja usuwania zajęć - w przygotowaniu");
}

void ClassTableWidget::duplicateClass()
{
    // TODO: Implementacja duplikowania zajęć
    QMessageBox::information(this, "Duplikuj zajęcia", "Funkcja duplikowania zajęć - w przygotowaniu");
}

void ClassTableWidget::viewReservations()
{
    Zajecia* zajecia = getSelectedClass();
    if (!zajecia) return;
    
    m_selectedClassId = zajecia->pobierzId();
    loadReservationData();
}

void ClassTableWidget::addReservation()
{
    // TODO: Implementacja po utworzeniu ReservationDialog
    QMessageBox::information(this, "Dodaj rezerwację", "Funkcja dodawania rezerwacji - w przygotowaniu");
}

void ClassTableWidget::cancelReservation()
{
    // TODO: Implementacja anulowania rezerwacji
    QMessageBox::information(this, "Anuluj rezerwację", "Funkcja anulowania rezerwacji - w przygotowaniu");
}

void ClassTableWidget::exportClasses()
{
    // TODO: Implementacja eksportu
    QMessageBox::information(this, "Export", "Funkcja eksportu - w przygotowaniu");
}

// === POZOSTAŁE SLOTY ===

void ClassTableWidget::contextMenuEvent(QContextMenuEvent *event)
{
    if (!m_servicesInitialized) return;
    
    QWidget* sourceWidget = childAt(event->pos());
    
    if (sourceWidget && sourceWidget->parent() == m_classesTable) {
        // Context menu dla tabeli zajęć
        bool hasSelection = !m_classesTable->selectedItems().isEmpty();
        bool singleSelection = m_classesTable->selectedItems().size() <= CLASS_COL_COUNT;
        
        m_editClassAction->setEnabled(hasSelection && singleSelection);
        m_duplicateClassAction->setEnabled(hasSelection && singleSelection);
        m_deleteClassAction->setEnabled(hasSelection);
        m_viewReservationsAction->setEnabled(hasSelection && singleSelection);
        m_copyClassAction->setEnabled(hasSelection);
        
        m_classContextMenu->exec(event->globalPos());
    } else if (sourceWidget && sourceWidget->parent() == m_reservationsTable) {
        // Context menu dla tabeli rezerwacji
        bool hasSelection = !m_reservationsTable->selectedItems().isEmpty();
        
        m_cancelReservationAction->setEnabled(hasSelection);
        m_copyReservationAction->setEnabled(hasSelection);
        
        m_reservationContextMenu->exec(event->globalPos());
    }
}

void ClassTableWidget::onTableSelectionChanged()
{
    bool hasSelection = !m_classesTable->selectedItems().isEmpty();
    bool singleSelection = m_classesTable->selectedItems().size() <= CLASS_COL_COUNT;
    
    m_editButton->setEnabled(hasSelection && singleSelection);
    m_duplicateButton->setEnabled(hasSelection && singleSelection);
    m_deleteButton->setEnabled(hasSelection);
    m_viewReservationsButton->setEnabled(hasSelection && singleSelection);
}

void ClassTableWidget::onReservationTableSelectionChanged()
{
    bool hasSelection = !m_reservationsTable->selectedItems().isEmpty();
    
    m_cancelReservationButton->setEnabled(hasSelection);
}

void ClassTableWidget::onTableDoubleClicked(int row, int column)
{
    Q_UNUSED(column)
    
    if (row >= 0 && row < static_cast<int>(m_filteredClasses.size())) {
        editClass();
    }
}

void ClassTableWidget::onReservationTableDoubleClicked(int row, int column)
{
    Q_UNUSED(column)
    Q_UNUSED(row)
    
    // TODO: Otwórz szczegóły rezerwacji
}

void ClassTableWidget::onSearchTextChanged(const QString& text)
{
    Q_UNUSED(text)
    filterClassesTable();
}

void ClassTableWidget::onFilterChanged()
{
    filterClassesTable();
}

void ClassTableWidget::onDateFilterChanged()
{
    filterClassesTable();
}

void ClassTableWidget::onClassSelectionChanged()
{
    viewReservations();
}

void ClassTableWidget::copyToClipboard()
{
    // TODO: Implementacja kopiowania do schowka
    QMessageBox::information(this, "Kopiuj", "Funkcja kopiowania - w przygotowaniu");
}

void ClassTableWidget::selectAll()
{
    m_classesTable->selectAll();
}

void ClassTableWidget::searchClasses()
{
    m_searchEdit->setFocus();
    m_searchEdit->selectAll();
}

void ClassTableWidget::clearSearch()
{
    m_searchEdit->clear();
    m_filterCombo->setCurrentIndex(0);
    m_trainerFilterCombo->setCurrentIndex(0);
}