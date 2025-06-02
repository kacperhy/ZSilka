// src/main.cpp
#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include <QDir>
#include <QMessageBox>
#include <QStyleFactory>
#include <QStandardPaths>
#include <QSplashScreen>
#include <QPixmap>
#include <QTimer>
#include <QDebug>
#include <memory>

// GUI
#include "gui/mainwindow.h"

// Backend - zachowany z oryginalnej aplikacji
#include "database/menedzer_bd.h"
#include "database/dao/klient_dao.h"
#include "database/dao/karnet_dao.h"
#include "database/dao/zajecia_dao.h"
#include "services/uslugi_klienta.h"
#include "services/uslugi_karnetu.h"
#include "services/uslugi_zajec.h"
#include "services/uslugi_raportow.h"
#include "utils/system_config.h"

class Application : public QApplication
{
    Q_OBJECT

public:
    Application(int &argc, char **argv) : QApplication(argc, argv)
    {
        setupApplicationProperties();
        setupStyle();
        setupTranslations();
    }

    bool initializeDatabase()
    {
        try {
            // Ścieżka do bazy danych w folderze aplikacji
            QString dbPath = QDir::currentPath() + "/data/database/silownia.db";

            // Utwórz folder jeśli nie istnieje
            QDir().mkpath(QDir::currentPath() + "/data/database");

            qDebug() << "Inicjalizacja bazy danych:" << dbPath;

            // Inicjalizacja menedżera bazy danych
            menedzerBD = std::make_unique<MenedzerBD>(dbPath.toStdString());
            menedzerBD->otworz();
            menedzerBD->inicjalizujBazeDanych();

            // Inicjalizacja DAO
            klientDAO = std::make_unique<KlientDAO>(*menedzerBD);
            karnetDAO = std::make_unique<KarnetDAO>(*menedzerBD);
            zajeciaDAO = std::make_unique<ZajeciaDAO>(*menedzerBD);

            // Inicjalizacja serwisów
            uslugiKlienta = std::make_unique<UslugiKlienta>(*klientDAO);
            uslugiKarnetu = std::make_unique<UslugiKarnetu>(*karnetDAO);
            uslugiZajec = std::make_unique<UslugiZajec>(*zajeciaDAO, *uslugiKarnetu);
            uslugiRaportow = std::make_unique<UslugiRaportow>(*menedzerBD);

            qDebug() << "Baza danych zainicjalizowana pomyślnie";
            return true;

        } catch (const std::exception& e) {
            qCritical() << "Błąd inicjalizacji bazy danych:" << e.what();

            QMessageBox::critical(nullptr,
                                  "Błąd inicjalizacji",
                                  QString("Nie można zainicjalizować bazy danych:\n%1\n\n"
                                          "Aplikacja zostanie zamknięta.").arg(e.what()));
            return false;
        }
    }

    bool initializeConfiguration()
    {
        try {
            QString configPath = QDir::currentPath() + "/config.ini";

            // Tutaj można dodać inicjalizację konfiguracji z istniejącej klasy SystemConfig
            qDebug() << "Ładowanie konfiguracji z:" << configPath;

            return true;

        } catch (const std::exception& e) {
            qWarning() << "Błąd ładowania konfiguracji:" << e.what();
            // Konfiguracja nie jest krytyczna, aplikacja może działać z domyślnymi ustawieniami
            return true;
        }
    }

    // Gettery dla serwisów - będą używane przez GUI
    UslugiKlienta* getUslugiKlienta() const { return uslugiKlienta.get(); }
    UslugiKarnetu* getUslugiKarnetu() const { return uslugiKarnetu.get(); }
    UslugiZajec* getUslugiZajec() const { return uslugiZajec.get(); }
    UslugiRaportow* getUslugiRaportow() const { return uslugiRaportow.get(); }

private:
    void setupApplicationProperties()
    {
        setApplicationName("System Zarządzania Siłownią");
        setApplicationVersion("2.0.0");
        setApplicationDisplayName("System Zarządzania Siłownią");
        setOrganizationName("GymSoft");
        setOrganizationDomain("gymsoft.pl");

        // Ikona aplikacji
        setWindowIcon(QIcon(":/icons/app.png"));
    }

    void setupStyle()
    {
        // Ustaw nowoczesny styl
        if (QStyleFactory::keys().contains("Fusion")) {
            setStyle("Fusion");
        }

        // Załaduj niestandardowy styl jeśli istnieje
        QFile styleFile(":/styles/application.qss");
        if (styleFile.open(QFile::ReadOnly)) {
            QString style = QLatin1String(styleFile.readAll());
            setStyleSheet(style);
        }

        // Ustaw paletę kolorów dla ciemnego motywu (opcjonalnie)
        setupDarkTheme();
    }

    void setupDarkTheme()
    {
        QPalette darkPalette;
        darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
        darkPalette.setColor(QPalette::WindowText, Qt::white);
        darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
        darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
        darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
        darkPalette.setColor(QPalette::ToolTipText, Qt::white);
        darkPalette.setColor(QPalette::Text, Qt::white);
        darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
        darkPalette.setColor(QPalette::ButtonText, Qt::white);
        darkPalette.setColor(QPalette::BrightText, Qt::red);
        darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
        darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
        darkPalette.setColor(QPalette::HighlightedText, Qt::black);

        // Odkomentuj poniższą linię aby włączyć ciemny motyw
        // setPalette(darkPalette);
    }

    void setupTranslations()
    {
        QString locale = QLocale::system().name();

        translator = std::make_unique<QTranslator>();
        if (translator->load(":/translations/gymmanagement_" + locale)) {
            installTranslator(translator.get());
            qDebug() << "Załadowano tłumaczenie:" << locale;
        } else {
            qDebug() << "Nie znaleziono tłumaczenia dla locale:" << locale;
        }
    }

private:
    // Backend components - zachowane z oryginalnej aplikacji
    std::unique_ptr<MenedzerBD> menedzerBD;
    std::unique_ptr<KlientDAO> klientDAO;
    std::unique_ptr<KarnetDAO> karnetDAO;
    std::unique_ptr<ZajeciaDAO> zajeciaDAO;
    std::unique_ptr<UslugiKlienta> uslugiKlienta;
    std::unique_ptr<UslugiKarnetu> uslugiKarnetu;
    std::unique_ptr<UslugiZajec> uslugiZajec;
    std::unique_ptr<UslugiRaportow> uslugiRaportow;

    // Translation
    std::unique_ptr<QTranslator> translator;
};

// Globalna instancja aplikacji dla łatwego dostępu do serwisów
Application* g_app = nullptr;

// Funkcje pomocnicze dla GUI do dostępu do serwisów
UslugiKlienta* getUslugiKlienta() {
    return g_app ? g_app->getUslugiKlienta() : nullptr;
}

UslugiKarnetu* getUslugiKarnetu() {
    return g_app ? g_app->getUslugiKarnetu() : nullptr;
}

UslugiZajec* getUslugiZajec() {
    return g_app ? g_app->getUslugiZajec() : nullptr;
}

UslugiRaportow* getUslugiRaportow() {
    return g_app ? g_app->getUslugiRaportow() : nullptr;
}

int main(int argc, char *argv[])
{
    // Włącz skalowanie DPI dla ekranów wysokiej rozdzielczości
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    Application app(argc, argv);
    g_app = &app;

    // Ekran powitalny (splash screen)
    QPixmap splashPixmap(":/icons/splash.png");
    QSplashScreen splash(splashPixmap);
    splash.show();

    splash.showMessage("Inicjalizacja aplikacji...", Qt::AlignBottom | Qt::AlignCenter, Qt::white);
    app.processEvents();

    // Inicjalizacja konfiguracji
    splash.showMessage("Ładowanie konfiguracji...", Qt::AlignBottom | Qt::AlignCenter, Qt::white);
    app.processEvents();

    if (!app.initializeConfiguration()) {
        splash.close();
        return -1;
    }

    // Inicjalizacja bazy danych
    splash.showMessage("Inicjalizacja bazy danych...", Qt::AlignBottom | Qt::AlignCenter, Qt::white);
    app.processEvents();

    if (!app.initializeDatabase()) {
        splash.close();
        return -1;
    }

    // Tworzenie głównego okna
    splash.showMessage("Uruchamianie interfejsu...", Qt::AlignBottom | Qt::AlignCenter, Qt::white);
    app.processEvents();

    MainWindow mainWindow;

    // Przekaż serwisy do głównego okna
    mainWindow.initializeServices(
        app.getUslugiKlienta(),
        app.getUslugiKarnetu(),
        app.getUslugiZajec(),
        app.getUslugiRaportow()
        );

    // Pokaż główne okno
    mainWindow.show();

    // Zamknij splash screen po 2 sekundach
    QTimer::singleShot(2000, &splash, &QWidget::close);

    qDebug() << "Aplikacja uruchomiona pomyślnie";

    int result = app.exec();

    qDebug() << "Aplikacja zakończona z kodem:" << result;
    g_app = nullptr;

    return result;
}

#include "main.moc"
