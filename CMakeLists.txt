cmake_minimum_required(VERSION 3.16)

project(GymManagementQt VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Znajdź wymagane komponenty Qt
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Sql)

# Automatyczne narzędzia Qt
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Definicje preprocesora
add_definitions(-DUNICODE -D_UNICODE)

# Pliki źródłowe - GUI
set(GUI_SOURCES
    src/main.cpp
    src/gui/mainwindow.cpp
    src/gui/mainwindow.h
    src/gui/mainwindow.ui

    # Dialogi
    src/gui/dialogs/clientdialog.cpp
    src/gui/dialogs/clientdialog.h
    src/gui/dialogs/clientdialog.ui
    src/gui/dialogs/membershipdialog.cpp
    src/gui/dialogs/membershipdialog.h
    src/gui/dialogs/membershipdialog.ui
    src/gui/dialogs/classdialog.cpp
    src/gui/dialogs/classdialog.h
    src/gui/dialogs/classdialog.ui
    src/gui/dialogs/reservationdialog.cpp
    src/gui/dialogs/reservationdialog.h
    src/gui/dialogs/reservationdialog.ui

    # Widgety
    src/gui/widgets/clienttablewidget.cpp
    src/gui/widgets/clienttablewidget.h
    src/gui/widgets/membershiptablewidget.cpp
    src/gui/widgets/membershiptablewidget.h
    src/gui/widgets/classtablewidget.cpp
    src/gui/widgets/classtablewidget.h
    src/gui/widgets/dashboardwidget.cpp
    src/gui/widgets/dashboardwidget.h
    src/gui/widgets/dashboardwidget.ui

    # Raporty
    src/gui/reports/reportwindow.cpp
    src/gui/reports/reportwindow.h
    src/gui/reports/reportwindow.ui
    src/gui/reports/activityreportwidget.cpp
    src/gui/reports/activityreportwidget.h
    src/gui/reports/financialreportwidget.cpp
    src/gui/reports/financialreportwidget.h
    src/gui/reports/popularityreportwidget.cpp
    src/gui/reports/popularityreportwidget.h
)

# Pliki źródłowe - Models (istniejące)
set(MODEL_SOURCES
    src/models/klient.cpp
    src/models/klient.h
    src/models/karnet.cpp
    src/models/karnet.h
    src/models/zajecia.cpp
    src/models/zajecia.h
    src/models/rezerwacja.cpp
    src/models/rezerwacja.h
)

# Pliki źródłowe - Database (istniejące)
set(DATABASE_SOURCES
    src/database/menedzer_bd.cpp
    src/database/menedzer_bd.h
    src/database/dao/klient_dao.cpp
    src/database/dao/klient_dao.h
    src/database/dao/karnet_dao.cpp
    src/database/dao/karnet_dao.h
    src/database/dao/zajecia_dao.cpp
    src/database/dao/zajecia_dao.h
)

# Pliki źródłowe - Services (istniejące)
set(SERVICE_SOURCES
    src/services/uslugi_klienta.cpp
    src/services/uslugi_klienta.h
    src/services/uslugi_karnetu.cpp
    src/services/uslugi_karnetu.h
    src/services/uslugi_zajec.cpp
    src/services/uslugi_zajec.h
    src/services/uslugi_raportow.cpp
    src/services/uslugi_raportow.h
)

# Pliki źródłowe - Utils (istniejące)
set(UTILS_SOURCES
    src/utils/eksport_danych.cpp
    src/utils/eksport_danych.h
    src/utils/import_danych.cpp
    src/utils/import_danych.h
    src/utils/historia_zmian.cpp
    src/utils/historia_zmian.h
    src/utils/system_config.cpp
    src/config/system_config.h
)

# Zasoby Qt
set(RESOURCE_FILES
    resources/resources.qrc
)

# Wszystkie pliki źródłowe
set(ALL_SOURCES
    ${GUI_SOURCES}
    ${MODEL_SOURCES}
    ${DATABASE_SOURCES}
    ${SERVICE_SOURCES}
    ${UTILS_SOURCES}
    ${RESOURCE_FILES}
)

# Utwórz plik wykonywalny
add_executable(GymManagementQt ${ALL_SOURCES})

# Linkuj z bibliotekami Qt
target_link_libraries(GymManagementQt
    Qt6::Core
    Qt6::Widgets
    Qt6::Sql
)

# Ścieżki include
target_include_directories(GymManagementQt PRIVATE
    src/
    src/gui/
    src/models/
    src/database/
    src/services/
    src/utils/
    src/config/
)

# Kompilacja z SQLite (jeśli potrzebne)
if(WIN32)
    # Windows - dodaj sqlite3
    find_library(SQLITE3_LIBRARY sqlite3)
    if(SQLITE3_LIBRARY)
        target_link_libraries(GymManagementQt ${SQLITE3_LIBRARY})
    endif()
endif()

# Ustawienia kompilacji dla różnych platform
if(WIN32)
    set_target_properties(GymManagementQt PROPERTIES
        WIN32_EXECUTABLE TRUE
    )

    # Ustaw ikone aplikacji dla Windows
    set_target_properties(GymManagementQt PROPERTIES
        LINK_FLAGS "/SUBSYSTEM:WINDOWS"
    )
endif()

# Kopiuj pliki konfiguracyjne do folderu docelowego
configure_file(data/config.ini ${CMAKE_BINARY_DIR}/config.ini COPYONLY)

# Utwórz folder dla bazy danych
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/data/database)

# Instalacja (opcjonalna)
install(TARGETS GymManagementQt
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
)

# Pakowanie (CPack)
set(CPACK_PACKAGE_NAME "System Zarządzania Siłownią")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_VENDOR "GymSoft")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "System zarządzania klientami, karnetami i zajęciami w siłowni")

include(CPack)
