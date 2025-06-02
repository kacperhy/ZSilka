// src/gui/dialogs/clientdialog.cpp
#include "clientdialog.h"
#include "ui_clientdialog.h"

#include <QRegularExpressionValidator>
#include <QRegularExpression>
#include <QDate>
#include <QDebug>

ClientDialog::ClientDialog(UslugiKlienta* uslugiKlienta, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ClientDialog)
    , m_uslugiKlienta(uslugiKlienta)
    , m_editMode(false)
    , m_dataChanged(false)
    , m_emailValidator(nullptr)
    , m_phoneValidator(nullptr)
{
    ui->setupUi(this);
    
    // Nowy klient - ustaw domyślne wartości
    m_klient = Klient();
    
    setupUI();
    setupValidators();
    setupConnections();
    updateWindowTitle();
    clearForm();
    
    qDebug() << "ClientDialog utworzony - tryb dodawania";
}

ClientDialog::ClientDialog(UslugiKlienta* uslugiKlienta, const Klient& klient, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ClientDialog)
    , m_uslugiKlienta(uslugiKlienta)
    , m_klient(klient)
    , m_editMode(true)
    , m_dataChanged(false)
    , m_emailValidator(nullptr)
    , m_phoneValidator(nullptr)
{
    ui->setupUi(this);
    
    setupUI();
    setupValidators();
    setupConnections();
    updateWindowTitle();
    loadKlientData();
    
    qDebug() << "ClientDialog utworzony - tryb edycji klienta ID:" << klient.pobierzId();
}

ClientDialog::~ClientDialog()
{
    delete ui;
}

void ClientDialog::setupUI()
{
    setModal(true);
    setWindowIcon(QIcon(":/icons/client.png"));
    resize(400, 500);
    setMinimumSize(350, 450);
    
    // Główny layout
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // Grupa: Dane osobowe
    QGroupBox* personalGroup = new QGroupBox("Dane osobowe", this);
    QFormLayout* personalLayout = new QFormLayout(personalGroup);
    
    // Pola formularza
    m_imieEdit = new QLineEdit(this);
    m_imieEdit->setMaxLength(MAX_NAME_LENGTH);
    m_imieEdit->setPlaceholderText("Wprowadź imię klienta");
    personalLayout->addRow("&Imię:", m_imieEdit);
    
    m_nazwiskoEdit = new QLineEdit(this);
    m_nazwiskoEdit->setMaxLength(MAX_NAME_LENGTH);
    m_nazwiskoEdit->setPlaceholderText("Wprowadź nazwisko klienta");
    personalLayout->addRow("&Nazwisko:", m_nazwiskoEdit);
    
    m_dataUrodzeniaDEdit = new QDateEdit(this);
    m_dataUrodzeniaDEdit->setDate(QDate::currentDate().addYears(-25));
    m_dataUrodzeniaDEdit->setMaximumDate(QDate::currentDate().addYears(-MIN_AGE_YEARS));
    m_dataUrodzeniaDEdit->setMinimumDate(QDate::currentDate().addYears(-100));
    m_dataUrodzeniaDEdit->setCalendarPopup(true);
    personalLayout->addRow("Data &urodzenia:", m_dataUrodzeniaDEdit);
    
    mainLayout->addWidget(personalGroup);
    
    // Grupa: Kontakt
    QGroupBox* contactGroup = new QGroupBox("Dane kontaktowe", this);
    QFormLayout* contactLayout = new QFormLayout(contactGroup);
    
    m_emailEdit = new QLineEdit(this);
    m_emailEdit->setPlaceholderText("przyklad@email.com");
    contactLayout->addRow("&E-mail:", m_emailEdit);
    
    m_telefonEdit = new QLineEdit(this);
    m_telefonEdit->setPlaceholderText("123-456-789");
    contactLayout->addRow("&Telefon:", m_telefonEdit);
    
    mainLayout->addWidget(contactGroup);
    
    // Grupa: Informacje systemowe
    QGroupBox* systemGroup = new QGroupBox("Informacje systemowe", this);
    QFormLayout* systemLayout = new QFormLayout(systemGroup);
    
    m_dataRejestracjiEdit = new QDateEdit(this);
    m_dataRejestracjiEdit->setDate(QDate::currentDate());
    m_dataRejestracjiEdit->setMaximumDate(QDate::currentDate());
    m_dataRejestracjiEdit->setCalendarPopup(true);
    if (!m_editMode) {
        m_dataRejestracjiEdit->setEnabled(false); // Data rejestracji automatyczna dla nowych klientów
    }
    systemLayout->addRow("Data &rejestracji:", m_dataRejestracjiEdit);
    
    m_uwagiEdit = new QTextEdit(this);
    m_uwagiEdit->setMaximumHeight(80);
    m_uwagiEdit->setPlaceholderText("Dodatkowe uwagi o kliencie...");
    systemLayout->addRow("&Uwagi:", m_uwagiEdit);
    
    mainLayout->addWidget(systemGroup);
    
    // Status
    m_statusLabel = new QLabel(this);
    m_statusLabel->setStyleSheet("QLabel { color: blue; font-style: italic; }");
    mainLayout->addWidget(m_statusLabel);
    
    // Przyciski
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    
    m_clearButton = new QPushButton("&Wyczyść", this);
    m_clearButton->setIcon(QIcon(":/icons/clear.png"));
    buttonLayout->addWidget(m_clearButton);
    
    buttonLayout->addStretch();
    
    m_cancelButton = new QPushButton("&Anuluj", this);
    m_cancelButton->setIcon(QIcon(":/icons/cancel.png"));
    buttonLayout->addWidget(m_cancelButton);
    
    m_saveButton = new QPushButton(m_editMode ? "&Zapisz" : "&Dodaj", this);
    m_saveButton->setIcon(QIcon(":/icons/save.png"));
    m_saveButton->setDefault(true);
    m_saveButton->setEnabled(false);
    buttonLayout->addWidget(m_saveButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // Ustaw focus na pierwszym polu
    m_imieEdit->setFocus();
}

void ClientDialog::setupValidators()
{
    // Walidator email
    QRegularExpression emailRegex("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
    m_emailValidator = new QRegularExpressionValidator(emailRegex, this);
    m_emailEdit->setValidator(m_emailValidator);
    
    // Walidator telefonu (format polski)
    QRegularExpression phoneRegex("^[0-9\\-\\+\\(\\)\\s]{9,15}$");
    m_phoneValidator = new QRegularExpressionValidator(phoneRegex, this);
    m_telefonEdit->setValidator(m_phoneValidator);
}

void ClientDialog::setupConnections()
{
    // Przyciski
    connect(m_saveButton, &QPushButton::clicked, this, &ClientDialog::accept);
    connect(m_cancelButton, &QPushButton::clicked, this, &ClientDialog::reject);
    connect(m_clearButton, &QPushButton::clicked, this, &ClientDialog::onClearForm);
    
    // Śledzenie zmian w polach
    connect(m_imieEdit, &QLineEdit::textChanged, this, &ClientDialog::onDataChanged);
    connect(m_nazwiskoEdit, &QLineEdit::textChanged, this, &ClientDialog::onDataChanged);
    connect(m_emailEdit, &QLineEdit::textChanged, this, &ClientDialog::onDataChanged);
    connect(m_telefonEdit, &QLineEdit::textChanged, this, &ClientDialog::onDataChanged);
    connect(m_dataUrodzeniaDEdit, QOverload<const QDate &>::of(&QDateEdit::dateChanged), 
            this, &ClientDialog::onDataChanged);
    connect(m_dataRejestracjiEdit, QOverload<const QDate &>::of(&QDateEdit::dateChanged), 
            this, &ClientDialog::onDataChanged);
    connect(m_uwagiEdit, &QTextEdit::textChanged, this, &ClientDialog::onDataChanged);
    
    // Walidacja na bieżąco
    connect(m_imieEdit, &QLineEdit::textChanged, this, &ClientDialog::onValidateForm);
    connect(m_nazwiskoEdit, &QLineEdit::textChanged, this, &ClientDialog::onValidateForm);
    connect(m_emailEdit, &QLineEdit::textChanged, this, &ClientDialog::onValidateForm);
}

void ClientDialog::loadKlientData()
{
    if (!m_editMode) return;
    
    m_imieEdit->setText(QString::fromStdString(m_klient.pobierzImie()));
    m_nazwiskoEdit->setText(QString::fromStdString(m_klient.pobierzNazwisko()));
    m_emailEdit->setText(QString::fromStdString(m_klient.pobierzEmail()));
    m_telefonEdit->setText(QString::fromStdString(m_klient.pobierzTelefon()));
    
    // Konwersja dat z string do QDate
    QString dataUrString = QString::fromStdString(m_klient.pobierzDateUrodzenia());
    if (!dataUrString.isEmpty()) {
        QDate dataUr = QDate::fromString(dataUrString, "yyyy-MM-dd");
        if (dataUr.isValid()) {
            m_dataUrodzeniaDEdit->setDate(dataUr);
        }
    }
    
    QString dataRejString = QString::fromStdString(m_klient.pobierzDateRejestracji());
    if (!dataRejString.isEmpty()) {
        QDate dataRej = QDate::fromString(dataRejString, "yyyy-MM-dd");
        if (dataRej.isValid()) {
            m_dataRejestracjiEdit->setDate(dataRej);
        }
    }
    
    m_uwagiEdit->setPlainText(QString::fromStdString(m_klient.pobierzUwagi()));
    
    m_dataChanged = false;
    enableSaveButton();
}

void ClientDialog::clearForm()
{
    m_imieEdit->clear();
    m_nazwiskoEdit->clear();
    m_emailEdit->clear();
    m_telefonEdit->clear();
    m_dataUrodzeniaDEdit->setDate(QDate::currentDate().addYears(-25));
    m_dataRejestracjiEdit->setDate(QDate::currentDate());
    m_uwagiEdit->clear();
    
    m_dataChanged = false;
    m_statusLabel->clear();
    enableSaveButton();
    
    m_imieEdit->setFocus();
}

bool ClientDialog::validateForm()
{
    QStringList errors;
    
    // Sprawdź wymagane pola
    if (m_imieEdit->text().trimmed().isEmpty()) {
        errors << "Imię jest wymagane";
    }
    
    if (m_nazwiskoEdit->text().trimmed().isEmpty()) {
        errors << "Nazwisko jest wymagane";
    }
    
    // Sprawdź email jeśli został podany
    QString email = m_emailEdit->text().trimmed();
    if (!email.isEmpty()) {
        int pos = 0;
        if (m_emailValidator->validate(email, pos) != QValidator::Acceptable) {
            errors << "Nieprawidłowy format adresu e-mail";
        }
    }
    
    // Sprawdź telefon jeśli został podany
    QString telefon = m_telefonEdit->text().trimmed();
    if (!telefon.isEmpty()) {
        int pos = 0;
        if (m_phoneValidator->validate(telefon, pos) != QValidator::Acceptable) {
            errors << "Nieprawidłowy format numeru telefonu";
        }
    }
    
    // Sprawdź wiek
    QDate dataUrodzenia = m_dataUrodzeniaDEdit->date();
    int wiek = dataUrodzenia.daysTo(QDate::currentDate()) / 365.25;
    if (wiek < MIN_AGE_YEARS) {
        errors << QString("Klient musi mieć co najmniej %1 lat").arg(MIN_AGE_YEARS);
    }
    
    // Sprawdź datę rejestracji
    if (m_dataRejestracjiEdit->date() > QDate::currentDate()) {
        errors << "Data rejestracji nie może być z przyszłości";
    }
    
    // Wyświetl błędy
    if (!errors.isEmpty()) {
        m_statusLabel->setText("Błędy: " + errors.join(", "));
        m_statusLabel->setStyleSheet("QLabel { color: red; font-style: italic; }");
        return false;
    }
    
    m_statusLabel->setText("Formularz poprawny");
    m_statusLabel->setStyleSheet("QLabel { color: green; font-style: italic; }");
    return true;
}

bool ClientDialog::saveKlient()
{
    if (!m_uslugiKlienta) {
        QMessageBox::critical(this, "Błąd", "Brak dostępu do serwisu klientów");
        return false;
    }
    
    try {
        // Utwórz/zaktualizuj obiekt klienta
        if (m_editMode) {
            // Edycja - zachowaj ID
            m_klient.ustawImie(m_imieEdit->text().trimmed().toStdString());
            m_klient.ustawNazwisko(m_nazwiskoEdit->text().trimmed().toStdString());
        } else {
            // Nowy klient
            m_klient = Klient(-1, 
                m_imieEdit->text().trimmed().toStdString(),
                m_nazwiskoEdit->text().trimmed().toStdString(),
                m_emailEdit->text().trimmed().toStdString(),
                m_telefonEdit->text().trimmed().toStdString(),
                m_dataUrodzeniaDEdit->date().toString("yyyy-MM-dd").toStdString(),
                m_dataRejestracjiEdit->date().toString("yyyy-MM-dd").toStdString(),
                m_uwagiEdit->toPlainText().trimmed().toStdString()
            );
        }
        
        // Ustaw pozostałe pola (dla edycji)
        if (m_editMode) {
            m_klient.ustawEmail(m_emailEdit->text().trimmed().toStdString());
            m_klient.ustawTelefon(m_telefonEdit->text().trimmed().toStdString());
            m_klient.ustawDateUrodzenia(m_dataUrodzeniaDEdit->date().toString("yyyy-MM-dd").toStdString());
            m_klient.ustawDateRejestracji(m_dataRejestracjiEdit->date().toString("yyyy-MM-dd").toStdString());
            m_klient.ustawUwagi(m_uwagiEdit->toPlainText().trimmed().toStdString());
        }
        
        // Zapisz w bazie danych
        if (m_editMode) {
            bool success = m_uslugiKlienta->aktualizujKlienta(m_klient);
            if (!success) {
                QMessageBox::critical(this, "Błąd", "Nie udało się zaktualizować danych klienta");
                return false;
            }
        } else {
            int noweId = m_uslugiKlienta->dodajKlienta(m_klient);
            if (noweId <= 0) {
                QMessageBox::critical(this, "Błąd", "Nie udało się dodać nowego klienta");
                return false;
            }
            m_klient.ustawId(noweId);
        }
        
        m_dataChanged = false;
        return true;
        
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Błąd", 
            QString("Wystąpił błąd podczas zapisywania klienta:\n%1").arg(e.what()));
        return false;
    }
}

void ClientDialog::updateWindowTitle()
{
    QString title = m_editMode ? "Edycja klienta" : "Nowy klient";
    if (m_editMode && !m_klient.pobierzPelneNazwisko().empty()) {
        title += " - " + QString::fromStdString(m_klient.pobierzPelneNazwisko());
    }
    if (m_dataChanged) {
        title += " *";
    }
    setWindowTitle(title);
}

void ClientDialog::enableSaveButton()
{
    bool canSave = validateForm();
    if (m_editMode) {
        canSave = canSave && m_dataChanged; // W trybie edycji wymaga zmian
    }
    m_saveButton->setEnabled(canSave);
}

void ClientDialog::accept()
{
    if (!validateForm()) {
        QMessageBox::warning(this, "Błędne dane", 
            "Formularz zawiera błędy. Popraw dane i spróbuj ponownie.");
        return;
    }
    
    if (saveKlient()) {
        QDialog::accept();
    }
}

void ClientDialog::reject()
{
    if (m_dataChanged) {
        QMessageBox::StandardButton ret = QMessageBox::question(this,
            "Niezapisane zmiany",
            "Masz niezapisane zmiany. Czy na pewno chcesz zamknąć okno?",
            QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
            
        if (ret != QMessageBox::Yes) {
            return; // Nie zamykaj okna
        }
    }
    
    QDialog::reject();
}

void ClientDialog::onDataChanged()
{
    m_dataChanged = true;
    updateWindowTitle();
    onValidateForm();
}

void ClientDialog::onClearForm()
{
    if (m_dataChanged) {
        QMessageBox::StandardButton ret = QMessageBox::question(this,
            "Wyczyść formularz",
            "Czy na pewno chcesz wyczyścić wszystkie pola?",
            QMessageBox::Yes | QMessageBox::No);
            
        if (ret != QMessageBox::Yes) {
            return;
        }
    }
    
    clearForm();
}

void ClientDialog::onValidateForm()
{
    validateForm();
    enableSaveButton();
}