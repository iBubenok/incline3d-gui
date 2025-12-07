cmake_minimum_required(VERSION 3.20)
project(incline3d_gui LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

option(INCLINE3D_GUI_BUILD_TESTS "Собирать unit-тесты" ON)
option(INCLINE3D_GUI_USE_CORE_LIB "Использовать primeincl_core как библиотеку" OFF)

# Поиск Qt 6
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    OpenGLWidgets
    Concurrent
    PrintSupport
)

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
