cmake_minimum_required(VERSION 3.20)
project(incline3d_gui LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

option(INCLINE3D_GUI_BUILD_TESTS "Собирать unit-тесты" ON)
option(INCLINE3D_GUI_USE_CORE_LIB "Использовать primeincl_core как библиотеку" OFF)
option(INCLINE3D_GUI_STATIC_QT "Использовать статическую сборку Qt" OFF)

# Поиск Qt 6
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    OpenGLWidgets
    Concurrent
    PrintSupport
)

# Определение статической линковки Qt
# Qt автоматически определяется как статический, если QT_STATIC_BUILD определён в Qt
get_target_property(QT_CORE_TYPE Qt6::Core TYPE)
if(QT_CORE_TYPE STREQUAL "STATIC_LIBRARY")
    set(INCLINE3D_QT_IS_STATIC TRUE)
    message(STATUS "Обнаружена статическая сборка Qt")
else()
    set(INCLINE3D_QT_IS_STATIC FALSE)
    message(STATUS "Обнаружена динамическая сборка Qt")
endif()

# Принудительная статическая сборка по опции
if(INCLINE3D_GUI_STATIC_QT AND NOT INCLINE3D_QT_IS_STATIC)
    message(WARNING "Запрошена статическая сборка Qt, но Qt собран динамически. "
                    "Для статической сборки необходимо использовать статически собранный Qt.")
endif()

# Путь к incline3d-cpp20 (ядро и CLI)
set(PRIMEINCL_CORE_DIR "${CMAKE_SOURCE_DIR}/../incline3d-cpp20" CACHE PATH
    "Путь к репозиторию incline3d-cpp20")
set(INCLPROC_EXECUTABLE "${PRIMEINCL_CORE_DIR}/build/inclproc" CACHE FILEPATH
    "Путь к исполняемому файлу inclproc")

# Если включена опция использования библиотеки ядра
if(INCLINE3D_GUI_USE_CORE_LIB AND EXISTS "${PRIMEINCL_CORE_DIR}/CMakeLists.txt")
    add_subdirectory(${PRIMEINCL_CORE_DIR} ${CMAKE_BINARY_DIR}/primeincl_core)
    set(USE_PRIMEINCL_CORE_LIB TRUE)
else()
    set(USE_PRIMEINCL_CORE_LIB FALSE)
endif()

# Исходные файлы моделей данных
set(MODEL_SOURCES
    src/models/well_data.cpp
    src/models/project_point.cpp
    src/models/shot_point.cpp
    src/models/well_table_model.cpp
    src/models/project_points_model.cpp
    src/models/shot_points_model.cpp
    src/models/measurements_model.cpp
    src/models/results_model.cpp
)

# Исходные файлы ядра GUI
set(CORE_SOURCES
    src/core/incline_process_runner.cpp
    src/core/project_manager.cpp
    src/core/file_io.cpp
    src/core/settings.cpp
)

# Исходные файлы UI
set(UI_SOURCES
    src/ui/main_window.cpp
    src/ui/wells_dock.cpp
    src/ui/project_points_dock.cpp
    src/ui/shot_points_dock.cpp
    src/ui/measurements_dock.cpp
    src/ui/results_dock.cpp
    src/ui/process_dialog.cpp
    src/ui/settings_dialog.cpp
    src/ui/about_dialog.cpp
    src/ui/proximity_dialog.cpp
    src/ui/offset_dialog.cpp
    src/ui/export_image_dialog.cpp
    src/ui/report_header_dialog.cpp
    src/ui/view_options_dialog.cpp
    src/ui/manual_input_dialog.cpp
    src/ui/import_las_dialog.cpp
    src/ui/import_zak_dialog.cpp
    src/ui/conclusion_dialog.cpp
    src/ui/vertical_settings_dialog.cpp
)

# Исходные файлы видов (визуализация)
set(VIEW_SOURCES
    src/views/view3d_widget.cpp
    src/views/plan_view.cpp
    src/views/vertical_view.cpp
    src/views/view_settings.cpp
)

# Исходные файлы утилит
set(UTIL_SOURCES
    src/utils/logger.cpp
    src/utils/angle_utils.cpp
)

# Основной исполняемый файл
add_executable(incline3d_gui
    src/app/main.cpp
    ${MODEL_SOURCES}
    ${CORE_SOURCES}
    ${UI_SOURCES}
    ${VIEW_SOURCES}
    ${UTIL_SOURCES}
    resources/resources.qrc
)

target_include_directories(incline3d_gui PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(incline3d_gui PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::OpenGLWidgets
    Qt6::Concurrent
    Qt6::PrintSupport
)

# Если доступна библиотека ядра, линкуем её
if(USE_PRIMEINCL_CORE_LIB)
    target_link_libraries(incline3d_gui PRIVATE primeincl_core)
    target_compile_definitions(incline3d_gui PRIVATE INCLINE3D_USE_CORE_LIB)
    target_include_directories(incline3d_gui PRIVATE ${PRIMEINCL_CORE_DIR}/cpp)
endif()

# Статическая сборка Qt: подключение плагинов и системных библиотек
if(INCLINE3D_QT_IS_STATIC)
    message(STATUS "Настройка статической линковки Qt...")

    # Определение для кода, чтобы инициализировать плагины
    target_compile_definitions(incline3d_gui PRIVATE INCLINE3D_STATIC_QT)

    # Поиск и подключение обязательных плагинов Qt для статической сборки
    # Плагин платформы (xcb для Linux, windows для Windows)
    if(WIN32)
        # Windows: плагин платформы qwindows
        find_package(Qt6 COMPONENTS QWindowsIntegrationPlugin QUIET)
        if(TARGET Qt6::QWindowsIntegrationPlugin)
            target_link_libraries(incline3d_gui PRIVATE Qt6::QWindowsIntegrationPlugin)
        endif()

        # Плагин стилей Windows
        find_package(Qt6 COMPONENTS QWindowsVistaStylePlugin QUIET)
        if(TARGET Qt6::QWindowsVistaStylePlugin)
            target_link_libraries(incline3d_gui PRIVATE Qt6::QWindowsVistaStylePlugin)
        endif()

        # Системные библиотеки Windows для OpenGL
        target_link_libraries(incline3d_gui PRIVATE
            opengl32
            gdi32
            user32
            dwmapi
            uxtheme
            ws2_32
            winmm
            imm32
            ole32
            oleaut32
            uuid
            advapi32
            shell32
            winspool
        )
    else()
        # Linux: плагин платформы xcb
        find_package(Qt6 COMPONENTS QXcbIntegrationPlugin QUIET)
        if(TARGET Qt6::QXcbIntegrationPlugin)
            target_link_libraries(incline3d_gui PRIVATE Qt6::QXcbIntegrationPlugin)
        endif()

        # Плагин XCB GL интеграции
        find_package(Qt6 COMPONENTS QXcbGlxIntegrationPlugin QUIET)
        if(TARGET Qt6::QXcbGlxIntegrationPlugin)
            target_link_libraries(incline3d_gui PRIVATE Qt6::QXcbGlxIntegrationPlugin)
        endif()

        # Поиск системных библиотек для Linux
        find_package(PkgConfig QUIET)
        if(PkgConfig_FOUND)
            pkg_check_modules(FONTCONFIG fontconfig)
            pkg_check_modules(FREETYPE freetype2)
            pkg_check_modules(XCB xcb xcb-xkb xcb-randr xcb-xfixes xcb-shape xcb-sync xcb-shm xcb-icccm xcb-keysyms xcb-image xcb-render-util xcb-render xcb-util xcb-cursor)
            pkg_check_modules(XKBCOMMON xkbcommon xkbcommon-x11)
            pkg_check_modules(X11 x11 x11-xcb)

            if(FONTCONFIG_FOUND)
                target_link_libraries(incline3d_gui PRIVATE ${FONTCONFIG_LIBRARIES})
                target_include_directories(incline3d_gui PRIVATE ${FONTCONFIG_INCLUDE_DIRS})
            endif()

            if(FREETYPE_FOUND)
                target_link_libraries(incline3d_gui PRIVATE ${FREETYPE_LIBRARIES})
                target_include_directories(incline3d_gui PRIVATE ${FREETYPE_INCLUDE_DIRS})
            endif()

            if(XCB_FOUND)
                target_link_libraries(incline3d_gui PRIVATE ${XCB_LIBRARIES})
                target_include_directories(incline3d_gui PRIVATE ${XCB_INCLUDE_DIRS})
            endif()

            if(XKBCOMMON_FOUND)
                target_link_libraries(incline3d_gui PRIVATE ${XKBCOMMON_LIBRARIES})
                target_include_directories(incline3d_gui PRIVATE ${XKBCOMMON_INCLUDE_DIRS})
            endif()

            if(X11_FOUND)
                target_link_libraries(incline3d_gui PRIVATE ${X11_LIBRARIES})
                target_include_directories(incline3d_gui PRIVATE ${X11_INCLUDE_DIRS})
            endif()
        endif()

        # OpenGL на Linux
        find_package(OpenGL REQUIRED)
        target_link_libraries(incline3d_gui PRIVATE OpenGL::GL)

        # Дополнительные системные библиотеки
        target_link_libraries(incline3d_gui PRIVATE
            pthread
            dl
        )
    endif()

    # Плагины форматов изображений (для экспорта PNG, JPEG и т.д.)
    find_package(Qt6 COMPONENTS QGifPlugin QICOPlugin QJpegPlugin QUIET)
    if(TARGET Qt6::QGifPlugin)
        target_link_libraries(incline3d_gui PRIVATE Qt6::QGifPlugin)
    endif()
    if(TARGET Qt6::QICOPlugin)
        target_link_libraries(incline3d_gui PRIVATE Qt6::QICOPlugin)
    endif()
    if(TARGET Qt6::QJpegPlugin)
        target_link_libraries(incline3d_gui PRIVATE Qt6::QJpegPlugin)
    endif()

    # Qt6 автоматически добавляет зависимости плагинов через qt_import_plugins
    # если используется qt_add_executable, но мы используем add_executable,
    # поэтому нужно вручную импортировать через Q_IMPORT_PLUGIN в коде

    message(STATUS "Статическая линковка Qt настроена")
endif()

# Определение пути к inclproc для использования в коде
target_compile_definitions(incline3d_gui PRIVATE
    INCLPROC_DEFAULT_PATH="${INCLPROC_EXECUTABLE}"
)

# Тесты
if(INCLINE3D_GUI_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Установка
install(TARGETS incline3d_gui
    RUNTIME DESTINATION bin
)
