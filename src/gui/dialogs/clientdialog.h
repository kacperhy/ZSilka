// src/gui/dialogs/clientdialog.h
#ifndef CLIENTDIALOG_H
#define CLIENTDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QTextEdit>
#include <QDateEdit>
#include <QPushButton>
#include <QLabel>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QValidator>

// Istniejące klasy z oryginalnej aplikacji
#include "models/klient.h"
#include "services/uslugi_klienta.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ClientDialog; }
QT_END_NAMESPACE

class ClientDialog : public QDialog
{
    Q_OBJECT

public:
    // Konstruktor dla dodawania nowego klienta
    explicit ClientDialog(UslugiKlienta* uslugiKlienta, QWidget *parent = nullptr);
    
    // Konstruktor dla edycji istniejącego klienta
    ClientDialog(UslugiKlienta* uslugiKlienta, const Klient& klient, QWidget *parent = nullptr);
    
    ~ClientDialog();

    // Gettery dla rezultatu
    Klient getKlient() const { return m_klient; }
    bool isEditMode() const { return m_editMode; }

protected slots:
    void accept() override;
    void reject() override;

private slots:
    void onDataChanged();
    void onClearForm();
    void onValidateForm();

private:
    void setupUI();
    void setupValidators();
    void setupConnections();
    void loadKlientData();
    void clearForm();
    bool validateForm();
    bool saveKlient();
    
    void updateWindowTitle();
    void enableSaveButton();

private:
    Ui::ClientDialog *ui;
    
    // Serwis do obsługi klientów (z oryginalnej aplikacji)
    UslugiKlienta* m_uslugiKlienta;
    
    // Dane klienta
    Klient m_klient;
    bool m_editMode;
    bool m_dataChanged;
    
    // Kontrolki UI
    QLineEdit* m_imieEdit;
    QLineEdit* m_nazwiskoEdit;
    QLineEdit* m_emailEdit;
    QLineEdit* m_telefonEdit;
    QDateEdit* m_dataUrodzeniaDEdit;
    QDateEdit* m_dataRejestracjiEdit;
    QTextEdit* m_uwagiEdit;
    
    // Przyciski
    QPushButton* m_saveButton;
    QPushButton* m_cancelButton;
    QPushButton* m_clearButton;
    
    // Etykiety statusu
    QLabel* m_statusLabel;
    
    // Walidatory
    QValidator* m_emailValidator;
    QValidator* m_phoneValidator;
    
    // Stałe
    static const int MIN_AGE_YEARS = 14;
    static const int MAX_NAME_LENGTH = 50;
    static const int MAX_NOTES_LENGTH = 500;
};

#endif // CLIENTDIALOG_H