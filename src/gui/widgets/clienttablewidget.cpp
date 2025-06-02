// src/gui/widgets/clienttablewidget.cpp
#include "clienttablewidget.h"
#include "gui/dialogs/clientdialog.h"

#include <QSortFilterProxyModel>
#include <QFileDialog>
#include <QProgressDialog>
#include <QDate>
#include <QDebug>

ClientTableWidget::ClientTableWidget(QWidget *parent)
    : QWidget(parent)
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
    m_refreshButton->setEnabled(false);
    m_exportButton->setEnabled(false);
    
    qDebug() << "ClientTableWidget utworzony";
}

ClientTableWidget::~ClientTableWidget()
{
    // Automatyczne czyszczenie przez Qt parent-child system
}

void ClientTableWidget::initializeServices(UslugiKlienta* uslugiKlienta)
{
    m_uslugiKlienta = uslugiKlienta;
    
    if (m_uslugiKlienta) {
        m_servicesInitialized = true;
        
        // Włącz przyciski
        m_addButton->setEnabled(true);
        m_refreshButton->setEnabled(true);
        m_exportButton->setEnabled(true);
        
        // Załaduj dane
        refresh();
        
        qDebug() << "ClientTableWidget - serwisy zainicjalizowane";
    }
}

void ClientTableWidget::setupUI()
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

void ClientTableWidget::setupTable()
{
    // Konfiguracja kolumn
    m_tableWidget->setColumnCount(COL_COUNT);
    
    QStringList headers;
    headers << "ID" << "Imię" << "Nazwisko" << "E-mail" 
            << "Telefon" << "Data urodzenia" << "Data rejestracji" << "Uwagi";
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
    m_tableWidget->setColumnWidth(COL_IMIE, 100);
    m_tableWidget->setColumnWidth(COL_NAZWISKO, 120);
    m_tableWidget->setColumnWidth(COL_EMAIL, 200);
    m_tableWidget->setColumnWidth(COL_TELEFON, 120);
    m_tableWidget->setColumnWidth(COL_DATA_URODZENIA, 110);
    m_tableWidget->setColumnWidth(COL_DATA_REJESTRACJI, 110);
    
    // Ukryj kolumnę ID (można pokazać w trybie debug)
    m_tableWidget->setColumnHidden(COL_ID, true);
}

void ClientTableWidget::setupToolbar()
{
    // Wyszukiwanie
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("Wyszukaj klientów...");
    m_searchEdit->setMaximumWidth(200);
    m_toolbarLayout->addWidget(m_searchEdit);
    
    // Filtr
    m_filterCombo = new QComboBox(this);
    m_filterCombo->addItem("Wszyscy klienci", FILTER_ALL);
    m_filterCombo->addItem("Ostatnio zarejestrowani", FILTER_RECENT);
    m_filterCombo->addItem("W tym miesiącu", FILTER_THIS_MONTH);
    m_filterCombo->addItem("Mają e-mail", FILTER_HAS_EMAIL);
    m_filterCombo->addItem("Mają telefon", FILTER_HAS_PHONE);
    m_filterCombo->addItem("Brak danych kontaktowych", FILTER_NO_CONTACT);
    m_filterCombo->setMaximumWidth(200);
    m_toolbarLayout->addWidget(m_filterCombo);
    
    m_toolbarLayout->addStretch();
    
    // Licznik
    m_countLabel = new QLabel("Klienci: 0", this);
    m_countLabel->setStyleSheet("QLabel { font-weight: bold; }");
    m_toolbarLayout->addWidget(m_countLabel);
    
    m_toolbarLayout->addSpacing(20);
    
    // Przyciski akcji
    m_addButton = new QPushButton("&Dodaj", this);
    m_addButton->setIcon(QIcon(":/icons/add.png"));
    m_addButton->setToolTip("Dodaj nowego klienta");
    m_toolbarLayout->addWidget(m_addButton);
    
    m_editButton = new QPushButton("&Edytuj", this);
    m_editButton->setIcon(QIcon(":/icons/edit.png"));
    m_editButton->setToolTip("Edytuj wybranego klienta");
    m_editButton->setEnabled(false);
    m_toolbarLayout->addWidget(m_editButton);
    
    m_deleteButton = new QPushButton("&Usuń", this);
    m_deleteButton->setIcon(QIcon(":/icons/delete.png"));
    m_deleteButton->setToolTip("Usuń wybranych klientów");
    m_deleteButton->setEnabled(false);
    m_toolbarLayout->addWidget(m_deleteButton);
    
    m_toolbarLayout->addSpacing(10);
    
    m_refreshButton = new QPushButton("&Odśwież", this);
    m_refreshButton->setIcon(QIcon(":/icons/refresh.png"));
    m_refreshButton->setToolTip("Odśwież listę klientów");
    m_toolbarLayout->addWidget(m_refreshButton);
    
    m_exportButton = new QPushButton("&Eksportuj", this);
    m_exportButton->setIcon(QIcon(":/icons/export.png"));
    m_exportButton->setToolTip("Eksportuj dane klientów");
    m_toolbarLayout->addWidget(m_exportButton);
}

void ClientTableWidget::setupConnections()
{
    // Przyciski
    connect(m_addButton, &QPushButton::clicked, this, &ClientTableWidget::addClient);
    connect(m_editButton, &QPushButton::clicked, this, &ClientTableWidget::editClient);
    connect(m_deleteButton, &QPushButton::clicked, this, &ClientTableWidget::deleteClient);
    connect(m_refreshButton, &QPushButton::clicked, this, &ClientTableWidget::refresh);
    connect(m_exportButton, &QPushButton::clicked, this, &ClientTableWidget::exportClients);
    
    // Wyszukiwanie i filtrowanie
    connect(m_searchEdit, &QLineEdit::textChanged, this, &ClientTableWidget::onSearchTextChanged);
    connect(m_filterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &ClientTableWidget::onFilterChanged);
    
    // Tabela
    connect(m_tableWidget, &QTableWidget::itemSelectionChanged, 
            this, &ClientTableWidget::onTableSelectionChanged);
    connect(m_tableWidget, &QTableWidget::cellDoubleClicked, 
            this, &ClientTableWidget::onTableDoubleClicked);
}

void ClientTableWidget::createContextMenu()
{
    m_contextMenu = new QMenu(this);
    
    m_editAction = m_contextMenu->addAction(QIcon(":/icons/edit.png"), "&Edytuj", 
                                           this, &ClientTableWidget::editClient);
    m_deleteAction = m_contextMenu->addAction(QIcon(":/icons/delete.png"), "&Usuń", 
                                             this, &ClientTableWidget::deleteClient);
    
    m_contextMenu->addSeparator();
    
    m_copyAction = m_contextMenu->addAction(QIcon(":/icons/copy.png"), "&Kopiuj", 
                                           this, &ClientTableWidget::copyToClipboard);
    m_selectAllAction = m_contextMenu->addAction("Zaznacz &wszystko", 
                                                this, &ClientTableWidget::selectAll);
    
    m_contextMenu->addSeparator();
    
    m_refreshAction = m_contextMenu->addAction(QIcon(":/icons/refresh.png"), "&Odśwież", 
                                              this, &ClientTableWidget::refresh);
}

void ClientTableWidget::refresh()
{
    if (!m_servicesInitialized || !m_uslugiKlienta) {
        return;
    }
    
    try {
        loadClientData();
        qDebug() << "ClientTableWidget - dane odświeżone, klientów:" << m_allClients.size();
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Błąd", 
            QString("Nie udało się załadować danych klientów:\n%1").arg(e.what()));
        qCritical() << "Błąd ładowania klientów:" << e.what();
    }
}

void ClientTableWidget::loadClientData()
{
    // Załaduj wszystkich klientów
    m_allClients = m_uslugiKlienta->pobierzWszystkichKlientow();
    
    // Zastosuj filtry
    filterTable();
}

void ClientTableWidget::populateTable(const std::vector<Klient>& klienci)
{
    m_tableWidget->setRowCount(static_cast<int>(klienci.size()));
    
    for (size_t i = 0; i < klienci.size(); ++i) {
        const Klient& klient = klienci[i];
        int row = static_cast<int>(i);
        
        // ID (ukryte)
        m_tableWidget->setItem(row, COL_ID, createTableItem(QString::number(klient.pobierzId())));
        
        // Imię
        m_tableWidget->setItem(row, COL_IMIE, createTableItem(QString::fromStdString(klient.pobierzImie())));
        
        // Nazwisko
        m_tableWidget->setItem(row, COL_NAZWISKO, createTableItem(QString::fromStdString(klient.pobierzNazwisko())));
        
        // E-mail
        m_tableWidget->setItem(row, COL_EMAIL, createTableItem(QString::fromStdString(klient.pobierzEmail())));
        
        // Telefon
        m_tableWidget->setItem(row, COL_TELEFON, createTableItem(QString::fromStdString(klient.pobierzTelefon())));
        
        // Data urodzenia
        m_tableWidget->setItem(row, COL_DATA_URODZENIA, createTableItem(formatDate(klient.pobierzDateUrodzenia())));
        
        // Data rejestracji
        m_tableWidget->setItem(row, COL_DATA_REJESTRACJI, createTableItem(formatDate(klient.pobierzDateRejestracji())));
        
        // Uwagi (skrócone)
        QString uwagi = QString::fromStdString(klient.pobierzUwagi());
        if (uwagi.length() > 50) {
            uwagi = uwagi.left(47) + "...";
        }
        m_tableWidget->setItem(row, COL_UWAGI, createTableItem(uwagi));
    }
    
    updateClientCount();
}

void ClientTableWidget::filterTable()
{
    m_filteredClients.clear();
    QString searchText = m_searchEdit->text().toLower();
    FilterType filterType = static_cast<FilterType>(m_filterCombo->currentData().toInt());
    
    QDate currentDate = QDate::currentDate();
    QDate monthStart = QDate(currentDate.year(), currentDate.month(), 1);
    QDate recentDate = currentDate.addDays(-RECENT_DAYS);
    
    for (const Klient& klient : m_allClients) {
        bool matches = true;
        
        // Filtr tekstowy
        if (!searchText.isEmpty()) {
            QString fullText = QString("%1 %2 %3 %4")
                .arg(QString::fromStdString(klient.pobierzImie()).toLower())
                .arg(QString::fromStdString(klient.pobierzNazwisko()).toLower())
                .arg(QString::fromStdString(klient.pobierzEmail()).toLower())
                .arg(QString::fromStdString(klient.pobierzTelefon()).toLower());
                
            if (!fullText.contains(searchText)) {
                matches = false;
            }
        }
        
        // Filtr kategorii
        if (matches) {
            switch (filterType) {
                case FILTER_ALL:
                    // Wszystkich - bez dodatkowego filtrowania
                    break;
                    
                case FILTER_RECENT: {
                    QDate regDate = QDate::fromString(QString::fromStdString(klient.pobierzDateRejestracji()), "yyyy-MM-dd");
                    if (!regDate.isValid() || regDate < recentDate) {
                        matches = false;
                    }
                    break;
                }
                
                case FILTER_THIS_MONTH: {
                    QDate regDate = QDate::fromString(QString::fromStdString(klient.pobierzDateRejestracji()), "yyyy-MM-dd");
                    if (!regDate.isValid() || regDate < monthStart) {
                        matches = false;
                    }
                    break;
                }
                
                case FILTER_HAS_EMAIL:
                    if (klient.pobierzEmail().empty()) {
                        matches = false;
                    }
                    break;
                    
                case FILTER_HAS_PHONE:
                    if (klient.pobierzTelefon().empty()) {
                        matches = false;
                    }
                    break;
                    
                case FILTER_NO_CONTACT:
                    if (!klient.pobierzEmail().empty() || !klient.pobierzTelefon().empty()) {
                        matches = false;
                    }
                    break;
            }
        }
        
        if (matches) {
            m_filteredClients.push_back(klient);
        }
    }
    
    populateTable(m_filteredClients);
}

void ClientTableWidget::updateClientCount()
{
    m_countLabel->setText(QString("Klienci: %1/%2")
        .arg(m_filteredClients.size())
        .arg(m_allClients.size()));
}

void ClientTableWidget::addClient()
{
    if (!m_servicesInitialized || !m_uslugiKlienta) return;
    
    ClientDialog dialog(m_uslugiKlienta, this);
    if (dialog.exec() == QDialog::Accepted) {
        Klient nowyKlient = dialog.getKlient();
        emit clientAdded(nowyKlient);
        refresh();
        
        // Zaznacz nowego klienta
        selectClientById(nowyKlient.pobierzId());
    }
}

void ClientTableWidget::editClient()
{
    Klient* klient = getSelectedClient();
    if (!klient || !m_uslugiKlienta) return;
    
    ClientDialog dialog(m_uslugiKlienta, *klient, this);
    if (dialog.exec() == QDialog::Accepted) {
        Klient edytowanyKlient = dialog.getKlient();
        emit clientEdited(edytowanyKlient);
        refresh();
        
        // Zaznacz edytowanego klienta
        selectClientById(edytowanyKlient.pobierzId());
    }
}

void ClientTableWidget::deleteClient()
{
    QList<Klient*> selectedClients = getSelectedClients();
    if (selectedClients.isEmpty() || !m_uslugiKlienta) return;
    
    QString message;
    if (selectedClients.size() == 1) {
        message = QString("Czy na pewno chcesz usunąć klienta:\n%1?")
            .arg(QString::fromStdString(selectedClients[0]->pobierzPelneNazwisko()));
    } else {
        message = QString("Czy na pewno chcesz usunąć %1 wybranych klientów?")
            .arg(selectedClients.size());
    }
    
    QMessageBox::StandardButton ret = QMessageBox::question(this,
        "Potwierdzenie usunięcia", message,
        QMessageBox::Yes | QMessageBox::No);
        
    if (ret != QMessageBox::Yes) return;
    
    // Usuń klientów
    QStringList errors;
    int deletedCount = 0;
    
    for (Klient* klient : selectedClients) {
        try {
            if (m_uslugiKlienta->usunKlienta(klient->pobierzId())) {
                emit clientDeleted(klient->pobierzId());
                deletedCount++;
            } else {
                errors << QString("Nie udało się usunąć: %1")
                    .arg(QString::fromStdString(klient->pobierzPelneNazwisko()));
            }
        } catch (const std::exception& e) {
            errors << QString("Błąd usuwania %1: %2")
                .arg(QString::fromStdString(klient->pobierzPelneNazwisko()))
                .arg(e.what());
        }
    }
    
    // Pokaż wyniki
    if (deletedCount > 0) {
        QMessageBox::information(this, "Usuwanie zakończone",
            QString("Usunięto %1 klientów.").arg(deletedCount));
        refresh();
    }
    
    if (!errors.isEmpty()) {
        QMessageBox::warning(this, "Błędy usuwania",
            "Wystąpiły błędy:\n" + errors.join("\n"));
    }
}

void ClientTableWidget::exportClients()
{
    if (m_filteredClients.empty()) {
        QMessageBox::information(this, "Export", "Brak klientów do eksportu.");
        return;
    }
    
    QString fileName = QFileDialog::getSaveFileName(this,
        "Eksportuj klientów", "klienci.csv", "Pliki CSV (*.csv)");
        
    if (fileName.isEmpty()) return;
    
    // TODO: Implementacja eksportu z wykorzystaniem istniejącej klasy EksportDanych
    QMessageBox::information(this, "Export", 
        QString("Export %1 klientów do pliku %2\n\nFunkcja w przygotowaniu.")
        .arg(m_filteredClients.size()).arg(fileName));
}

Klient* ClientTableWidget::getSelectedClient()
{
    QList<QTableWidgetItem*> selectedItems = m_tableWidget->selectedItems();
    if (selectedItems.isEmpty()) return nullptr;
    
    int row = selectedItems[0]->row();
    if (row < 0 || row >= static_cast<int>(m_filteredClients.size())) return nullptr;
    
    return &m_filteredClients[row];
}

QList<Klient*> ClientTableWidget::getSelectedClients()
{
    QList<Klient*> result;
    QSet<int> selectedRows;
    
    for (QTableWidgetItem* item : m_tableWidget->selectedItems()) {
        selectedRows.insert(item->row());
    }
    
    for (int row : selectedRows) {
        if (row >= 0 && row < static_cast<int>(m_filteredClients.size())) {
            result.append(&m_filteredClients[row]);
        }
    }
    
    return result;
}

void ClientTableWidget::selectClientById(int clientId)
{
    for (int row = 0; row < m_tableWidget->rowCount(); ++row) {
        QTableWidgetItem* idItem = m_tableWidget->item(row, COL_ID);
        if (idItem && idItem->text().toInt() == clientId) {
            m_tableWidget->selectRow(row);
            m_tableWidget->scrollToItem(idItem);
            break;
        }
    }
}

QString ClientTableWidget::formatDate(const std::string& dateStr)
{
    if (dateStr.empty()) return "";
    
    QDate date = QDate::fromString(QString::fromStdString(dateStr), "yyyy-MM-dd");
    if (date.isValid()) {
        return date.toString("dd.MM.yyyy");
    }
    
    return QString::fromStdString(dateStr);
}

QTableWidgetItem* ClientTableWidget::createTableItem(const QString& text)
{
    QTableWidgetItem* item = new QTableWidgetItem(text);
    item->setFlags(item->flags() & ~Qt::ItemIsEditable); // Tylko do odczytu
    return item;
}

void ClientTableWidget::contextMenuEvent(QContextMenuEvent *event)
{
    if (!m_servicesInitialized) return;
    
    QTableWidgetItem* item = m_tableWidget->itemAt(m_tableWidget->mapFromGlobal(event->globalPos()));
    
    // Włącz/wyłącz akcje w zależności od zaznaczenia
    bool hasSelection = !m_tableWidget->selectedItems().isEmpty();
    m_editAction->setEnabled(hasSelection && m_tableWidget->selectedItems().size() <= COL_COUNT);
    m_deleteAction->setEnabled(hasSelection);
    m_copyAction->setEnabled(hasSelection);
    
    m_contextMenu->exec(event->globalPos());
}

void ClientTableWidget::onTableSelectionChanged()
{
    bool hasSelection = !m_tableWidget->selectedItems().isEmpty();
    bool singleSelection = m_tableWidget->selectedItems().size() <= COL_COUNT;
    
    m_editButton->setEnabled(hasSelection && singleSelection);
    m_deleteButton->setEnabled(hasSelection);
}

void ClientTableWidget::onTableDoubleClicked(int row, int column)
{
    Q_UNUSED(column)
    
    if (row >= 0 && row < static_cast<int>(m_filteredClients.size())) {
        editClient();
    }
}

void ClientTableWidget::onSearchTextChanged(const QString& text)
{
    Q_UNUSED(text)
    filterTable();
}

void ClientTableWidget::onFilterChanged()
{
    filterTable();
}

void ClientTableWidget::copyToClipboard()
{
    QList<Klient*> selectedClients = getSelectedClients();
    if (selectedClients.isEmpty()) return;
    
    QStringList lines;
    lines << "Imię\tNazwisko\tE-mail\tTelefon\tData urodzenia\tData rejestracji";
    
    for (Klient* klient : selectedClients) {
        lines << QString("%1\t%2\t%3\t%4\t%5\t%6")
            .arg(QString::fromStdString(klient->pobierzImie()))
            .arg(QString::fromStdString(klient->pobierzNazwisko()))
            .arg(QString::fromStdString(klient->pobierzEmail()))
            .arg(QString::fromStdString(klient->pobierzTelefon()))
            .arg(formatDate(klient->pobierzDateUrodzenia()))
            .arg(formatDate(klient->pobierzDateRejestracji()));
    }
    
    QApplication::clipboard()->setText(lines.join("\n"));
}

void ClientTableWidget::selectAll()
{
    m_tableWidget->selectAll();
}

void ClientTableWidget::searchClients()
{
    m_searchEdit->setFocus();
    m_searchEdit->selectAll();
}

void ClientTableWidget::clearSearch()
{
    m_searchEdit->clear();
    m_filterCombo->setCurrentIndex(0);
}