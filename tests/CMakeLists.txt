# Модульные тесты для incline3d-gui

find_package(Qt6 REQUIRED COMPONENTS Test)

enable_testing()

# Общие исходные файлы
set(COMMON_MODEL_SOURCES
    ${CMAKE_SOURCE_DIR}/src/models/well_data.cpp
    ${CMAKE_SOURCE_DIR}/src/models/project_point.cpp
    ${CMAKE_SOURCE_DIR}/src/models/shot_point.cpp
)

# Вспомогательная функция для добавления тестов
function(add_gui_test TEST_NAME)
    add_executable(${TEST_NAME} ${ARGN})
    target_link_libraries(${TEST_NAME} PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Test
    )
    target_include_directories(${TEST_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}/src
    )
    target_compile_definitions(${TEST_NAME} PRIVATE
        INCLPROC_DEFAULT_PATH="inclproc"
    )
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    set_tests_properties(${TEST_NAME} PROPERTIES
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endfunction()

# Тесты моделей данных
add_gui_test(test_well_data
    test_well_data.cpp
    ${CMAKE_SOURCE_DIR}/src/models/well_data.cpp
)

# Тесты утилит
add_gui_test(test_angle_utils
    test_angle_utils.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/angle_utils.cpp
)

# Тесты Qt-моделей
add_gui_test(test_well_table_model
    test_well_table_model.cpp
    ${CMAKE_SOURCE_DIR}/src/models/well_data.cpp
    ${CMAKE_SOURCE_DIR}/src/models/well_table_model.cpp
)

# Тесты ProjectManager
add_gui_test(test_project_manager
    test_project_manager.cpp
    ${COMMON_MODEL_SOURCES}
    ${CMAKE_SOURCE_DIR}/src/core/project_manager.cpp
    ${CMAKE_SOURCE_DIR}/src/core/file_io.cpp
    ${CMAKE_SOURCE_DIR}/src/core/settings.cpp
)

# Тесты InclineProcessRunner
add_gui_test(test_process_runner
    test_process_runner.cpp
    ${CMAKE_SOURCE_DIR}/src/models/well_data.cpp
    ${CMAKE_SOURCE_DIR}/src/core/incline_process_runner.cpp
)
