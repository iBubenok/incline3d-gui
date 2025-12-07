#include "ui/main_window.h"

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QCloseEvent>
#include <QDir>
#include <QDockWidget>
#include <QFileDialog>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QProgressBar>
#include <QStandardPaths>
#include <QStatusBar>
#include <QTabWidget>
#include <QToolBar>

#include "core/file_io.h"
#include "core/incline_process_runner.h"
#include "core/project_manager.h"
#include "core/settings.h"
#include "models/measurements_model.h"
#include "models/project_points_model.h"
#include "models/results_model.h"
#include "models/shot_points_model.h"
#include "models/well_table_model.h"
#include "ui/wells_dock.h"
#include "ui/project_points_dock.h"
#include "ui/shot_points_dock.h"
#include "ui/measurements_dock.h"
#include "ui/results_dock.h"
#include "ui/settings_dialog.h"
#include "ui/about_dialog.h"
#include "ui/process_dialog.h"
#include "ui/proximity_dialog.h"
#include "ui/offset_dialog.h"
#include "ui/export_image_dialog.h"
#include "ui/report_header_dialog.h"
#include "ui/view_options_dialog.h"
#include "ui/manual_input_dialog.h"
#include "ui/import_las_dialog.h"
#include "ui/import_zak_dialog.h"
#include "ui/conclusion_dialog.h"
#include "ui/vertical_settings_dialog.h"
#include "views/view3d_widget.h"
#include "views/plan_view.h"
#include "views/vertical_view.h"
#include "utils/logger.h"

namespace incline3d::ui {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {
    // Инициализация компонентов ядра
    project_manager_ = std::make_unique<core::ProjectManager>(this);
    process_runner_ = std::make_unique<core::InclineProcessRunner>(this);
    file_io_ = std::make_unique<core::FileIO>();

    // Инициализация моделей
    well_model_ = std::make_unique<models::WellTableModel>(this);
    project_points_model_ = std::make_unique<models::ProjectPointsModel>(this);
    shot_points_model_ = std::make_unique<models::ShotPointsModel>(this);
    measurements_model_ = std::make_unique<models::MeasurementsModel>(this);
    results_model_ = std::make_unique<models::ResultsModel>(this);

    setupUi();
    loadSettings();

    // Подключение сигналов проекта
    connect(project_manager_.get(), &core::ProjectManager::projectCreated,
            this, &MainWindow::updateWindowTitle);
    connect(project_manager_.get(), &core::ProjectManager::projectLoaded,
            this, &MainWindow::updateWindowTitle);
    connect(project_manager_.get(), &core::ProjectManager::projectSaved,
            this, &MainWindow::updateWindowTitle);
    connect(project_manager_.get(), &core::ProjectManager::dirtyChanged,
            this, &MainWindow::updateWindowTitle);
    connect(project_manager_.get(), &core::ProjectManager::wellsChanged,
            this, [this]() {
                // Синхронизация моделей с проектом
                well_model_->clear();
                for (const auto& well : project_manager_->wells()) {
                    well_model_->addWell(well);
                }
                if (view3d_) view3d_->update();
                if (plan_view_) plan_view_->update();
                if (vertical_view_) vertical_view_->update();
            });

    // Подключение сигналов процесса
    connect(process_runner_.get(), &core::InclineProcessRunner::processFinished,
            this, [this](const core::ProcessResult& result) {
                onProcessFinished(result.success, result.error_message);
            });

    // Автосохранение
    auto_save_timer_ = new QTimer(this);
    connect(auto_save_timer_, &QTimer::timeout, this, &MainWindow::onAutoSave);
    auto& settings = core::Settings::instance();
    if (settings.autoSaveEnabled()) {
        auto_save_timer_->start(settings.autoSaveIntervalMinutes() * 60 * 1000);
    }

    // Создание нового проекта
    project_manager_->newProject();

    // Проверка восстановления сессии после запуска окна
    QTimer::singleShot(100, this, &MainWindow::checkRecovery);

    LOG_INFO(tr("Приложение запущено"));
}

MainWindow::~MainWindow() {
    saveSettings();
}

void MainWindow::setupUi() {
    setWindowTitle(tr("Incline3D - Модуль инклинометрии"));
    setMinimumSize(1024, 768);

    createActions();
    createMenus();
    createToolBars();
    createDockWidgets();
    createCentralWidget();
    createStatusBar();

    // Начальная компоновка доков
    setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
    setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);
}

void MainWindow::createActions() {
    // Файл
    action_new_project_ = new QAction(tr("Новый проект"), this);
    action_new_project_->setShortcut(QKeySequence::New);
    connect(action_new_project_, &QAction::triggered, this, &MainWindow::onNewProject);

    action_open_project_ = new QAction(tr("Открыть проект..."), this);
    action_open_project_->setShortcut(QKeySequence::Open);
    connect(action_open_project_, &QAction::triggered, this, &MainWindow::onOpenProject);

    action_save_project_ = new QAction(tr("Сохранить проект"), this);
    action_save_project_->setShortcut(QKeySequence::Save);
    connect(action_save_project_, &QAction::triggered, this, &MainWindow::onSaveProject);

    action_save_project_as_ = new QAction(tr("Сохранить проект как..."), this);
    action_save_project_as_->setShortcut(QKeySequence::SaveAs);
    connect(action_save_project_as_, &QAction::triggered, this, &MainWindow::onSaveProjectAs);

    action_open_file_ = new QAction(tr("Открыть файл данных..."), this);
    action_open_file_->setShortcut(Qt::CTRL | Qt::SHIFT | Qt::Key_O);
    connect(action_open_file_, &QAction::triggered, this, &MainWindow::onOpenFile);

    action_save_file_ = new QAction(tr("Сохранить данные скважины..."), this);
    connect(action_save_file_, &QAction::triggered, this, &MainWindow::onSaveFile);

    action_export_project_ = new QAction(tr("Экспорт проекта..."), this);
    connect(action_export_project_, &QAction::triggered, this, &MainWindow::onExportProject);

    action_exit_ = new QAction(tr("Выход"), this);
    action_exit_->setShortcut(QKeySequence::Quit);
    connect(action_exit_, &QAction::triggered, this, &QMainWindow::close);

    // Редактирование
    action_add_well_ = new QAction(tr("Добавить скважину..."), this);
    connect(action_add_well_, &QAction::triggered, this, &MainWindow::onAddWell);

    action_remove_well_ = new QAction(tr("Удалить скважину"), this);
    action_remove_well_->setShortcut(Qt::Key_Delete);
    connect(action_remove_well_, &QAction::triggered, this, &MainWindow::onRemoveWell);

    action_add_project_point_ = new QAction(tr("Добавить проектную точку"), this);
    connect(action_add_project_point_, &QAction::triggered, this, &MainWindow::onAddProjectPoint);

    action_remove_project_point_ = new QAction(tr("Удалить проектную точку"), this);
    connect(action_remove_project_point_, &QAction::triggered, this, &MainWindow::onRemoveProjectPoint);

    action_add_shot_point_ = new QAction(tr("Добавить пункт возбуждения"), this);
    connect(action_add_shot_point_, &QAction::triggered, this, &MainWindow::onAddShotPoint);

    action_remove_shot_point_ = new QAction(tr("Удалить пункт возбуждения"), this);
    connect(action_remove_shot_point_, &QAction::triggered, this, &MainWindow::onRemoveShotPoint);

    // Обработка
    action_process_well_ = new QAction(tr("Обработать скважину..."), this);
    action_process_well_->setShortcut(Qt::Key_F5);
    connect(action_process_well_, &QAction::triggered, this, &MainWindow::onProcessWell);

    action_process_all_ = new QAction(tr("Обработать все скважины"), this);
    action_process_all_->setShortcut(Qt::SHIFT | Qt::Key_F5);
    connect(action_process_all_, &QAction::triggered, this, &MainWindow::onProcessAllWells);

    action_proximity_ = new QAction(tr("Анализ сближения..."), this);
    connect(action_proximity_, &QAction::triggered, this, &MainWindow::onProximityAnalysis);

    action_offset_ = new QAction(tr("Расчёт отхода..."), this);
    connect(action_offset_, &QAction::triggered, this, &MainWindow::onOffsetAnalysis);

    // Вид
    action_view_3d_ = new QAction(tr("3D аксонометрия"), this);
    action_view_3d_->setShortcut(Qt::Key_1);
    connect(action_view_3d_, &QAction::triggered, this, &MainWindow::onView3D);

    action_view_plan_ = new QAction(tr("План"), this);
    action_view_plan_->setShortcut(Qt::Key_2);
    connect(action_view_plan_, &QAction::triggered, this, &MainWindow::onViewPlan);

    action_view_vertical_ = new QAction(tr("Вертикальная проекция"), this);
    action_view_vertical_->setShortcut(Qt::Key_3);
    connect(action_view_vertical_, &QAction::triggered, this, &MainWindow::onViewVertical);

    action_reset_view_ = new QAction(tr("Сбросить вид"), this);
    action_reset_view_->setShortcut(Qt::Key_Home);
    connect(action_reset_view_, &QAction::triggered, this, &MainWindow::onResetView);

    action_view_options_ = new QAction(tr("Настройки отображения..."), this);
    connect(action_view_options_, &QAction::triggered, this, &MainWindow::onViewOptions);

    action_export_image_ = new QAction(tr("Экспорт изображения..."), this);
    action_export_image_->setShortcut(Qt::CTRL | Qt::Key_E);
    connect(action_export_image_, &QAction::triggered, this, &MainWindow::onExportImage);

    action_copy_to_clipboard_ = new QAction(tr("Копировать вид в буфер"), this);
    action_copy_to_clipboard_->setShortcut(Qt::CTRL | Qt::SHIFT | Qt::Key_C);
    connect(action_copy_to_clipboard_, &QAction::triggered, this, &MainWindow::onCopyToClipboard);

    // Исходные данные
    action_manual_input_ = new QAction(tr("Ручной ввод..."), this);
    action_manual_input_->setShortcut(Qt::CTRL | Qt::Key_M);
    connect(action_manual_input_, &QAction::triggered, this, &MainWindow::onManualInput);

    action_import_las_ = new QAction(tr("Импорт из LAS..."), this);
    connect(action_import_las_, &QAction::triggered, this, &MainWindow::onImportLas);

    action_import_zak_ = new QAction(tr("Импорт из ЗАК..."), this);
    connect(action_import_zak_, &QAction::triggered, this, &MainWindow::onImportZak);

    // Отчёты
    action_edit_report_header_ = new QAction(tr("Редактировать шапку..."), this);
    connect(action_edit_report_header_, &QAction::triggered, this, &MainWindow::onEditReportHeader);

    action_export_report_ = new QAction(tr("Экспорт отчёта..."), this);
    connect(action_export_report_, &QAction::triggered, this, &MainWindow::onExportReport);

    action_conclusion_ = new QAction(tr("Заключение..."), this);
    action_conclusion_->setShortcut(Qt::Key_F6);
    connect(action_conclusion_, &QAction::triggered, this, &MainWindow::onConclusion);

    // Вертикальная проекция
    action_vertical_settings_ = new QAction(tr("Настройки вертикальной проекции..."), this);
    connect(action_vertical_settings_, &QAction::triggered, this, &MainWindow::onVerticalSettings);

    // Настройки
    action_settings_ = new QAction(tr("Настройки..."), this);
    connect(action_settings_, &QAction::triggered, this, &MainWindow::onSettings);

    action_about_ = new QAction(tr("О программе..."), this);
    connect(action_about_, &QAction::triggered, this, &MainWindow::onAbout);
}

void MainWindow::createMenus() {
    // Меню Файл
    file_menu_ = menuBar()->addMenu(tr("&Файл"));
    file_menu_->addAction(action_new_project_);
    file_menu_->addAction(action_open_project_);
    file_menu_->addAction(action_save_project_);
    file_menu_->addAction(action_save_project_as_);
    file_menu_->addSeparator();
    file_menu_->addAction(action_open_file_);
    file_menu_->addAction(action_save_file_);
    file_menu_->addAction(action_export_project_);
    file_menu_->addSeparator();

    recent_files_menu_ = file_menu_->addMenu(tr("Последние файлы"));
    recent_projects_menu_ = file_menu_->addMenu(tr("Последние проекты"));
    updateRecentFilesMenu();
    updateRecentProjectsMenu();

    file_menu_->addSeparator();
    file_menu_->addAction(action_exit_);

    // Меню Редактирование
    edit_menu_ = menuBar()->addMenu(tr("&Редактирование"));
    edit_menu_->addAction(action_add_well_);
    edit_menu_->addAction(action_remove_well_);
    edit_menu_->addSeparator();
    edit_menu_->addAction(action_add_project_point_);
    edit_menu_->addAction(action_remove_project_point_);
    edit_menu_->addSeparator();
    edit_menu_->addAction(action_add_shot_point_);
    edit_menu_->addAction(action_remove_shot_point_);

    // Меню Исходные данные
    data_menu_ = menuBar()->addMenu(tr("&Исходные данные"));
    data_menu_->addAction(action_manual_input_);
    data_menu_->addSeparator();
    data_menu_->addAction(action_import_las_);
    data_menu_->addAction(action_import_zak_);

    // Меню Обработка
    process_menu_ = menuBar()->addMenu(tr("&Обработка"));
    process_menu_->addAction(action_process_well_);
    process_menu_->addAction(action_process_all_);
    process_menu_->addSeparator();
    process_menu_->addAction(action_proximity_);
    process_menu_->addAction(action_offset_);

    // Меню Вид
    view_menu_ = menuBar()->addMenu(tr("&Вид"));
    view_menu_->addAction(action_view_3d_);
    view_menu_->addAction(action_view_plan_);
    view_menu_->addAction(action_view_vertical_);
    view_menu_->addSeparator();
    view_menu_->addAction(action_reset_view_);
    view_menu_->addAction(action_view_options_);
    view_menu_->addAction(action_vertical_settings_);
    view_menu_->addSeparator();
    view_menu_->addAction(action_export_image_);
    view_menu_->addAction(action_copy_to_clipboard_);

    // Меню Отчёты
    report_menu_ = menuBar()->addMenu(tr("&Отчёты"));
    report_menu_->addAction(action_edit_report_header_);
    report_menu_->addAction(action_export_report_);
    report_menu_->addSeparator();
    report_menu_->addAction(action_conclusion_);

    // Меню Справка
    help_menu_ = menuBar()->addMenu(tr("&Справка"));
    help_menu_->addAction(action_settings_);
    help_menu_->addSeparator();
    help_menu_->addAction(action_about_);
}

void MainWindow::createToolBars() {
    main_toolbar_ = addToolBar(tr("Основная панель"));
    main_toolbar_->setObjectName("MainToolBar");

    main_toolbar_->addAction(action_new_project_);
    main_toolbar_->addAction(action_open_project_);
    main_toolbar_->addAction(action_save_project_);
    main_toolbar_->addSeparator();
    main_toolbar_->addAction(action_open_file_);
    main_toolbar_->addSeparator();
    main_toolbar_->addAction(action_process_well_);
    main_toolbar_->addAction(action_process_all_);
    main_toolbar_->addSeparator();
    main_toolbar_->addAction(action_view_3d_);
    main_toolbar_->addAction(action_view_plan_);
    main_toolbar_->addAction(action_view_vertical_);
}

void MainWindow::createDockWidgets() {
    // Док скважин
    wells_dock_ = new WellsDock(well_model_.get(), this);
    wells_dock_->setObjectName("WellsDock");
    addDockWidget(Qt::LeftDockWidgetArea, wells_dock_);

    connect(wells_dock_, &WellsDock::wellSelected,
            this, &MainWindow::onWellSelected);

    // Док проектных точек
    project_points_dock_ = new ProjectPointsDock(project_points_model_.get(), this);
    project_points_dock_->setObjectName("ProjectPointsDock");
    addDockWidget(Qt::LeftDockWidgetArea, project_points_dock_);

    // Док пунктов возбуждения
    shot_points_dock_ = new ShotPointsDock(shot_points_model_.get(), this);
    shot_points_dock_->setObjectName("ShotPointsDock");
    addDockWidget(Qt::LeftDockWidgetArea, shot_points_dock_);

    // Таблификация левых доков
    tabifyDockWidget(wells_dock_, project_points_dock_);
    tabifyDockWidget(project_points_dock_, shot_points_dock_);
    wells_dock_->raise();

    // Док замеров
    measurements_dock_ = new MeasurementsDock(measurements_model_.get(), this);
    measurements_dock_->setObjectName("MeasurementsDock");
    addDockWidget(Qt::RightDockWidgetArea, measurements_dock_);

    // Док результатов
    results_dock_ = new ResultsDock(results_model_.get(), this);
    results_dock_->setObjectName("ResultsDock");
    addDockWidget(Qt::RightDockWidgetArea, results_dock_);

    // Таблификация правых доков
    tabifyDockWidget(measurements_dock_, results_dock_);
    measurements_dock_->raise();

    // Добавление в меню Вид
    view_menu_->addSeparator();
    view_menu_->addAction(wells_dock_->toggleViewAction());
    view_menu_->addAction(project_points_dock_->toggleViewAction());
    view_menu_->addAction(shot_points_dock_->toggleViewAction());
    view_menu_->addAction(measurements_dock_->toggleViewAction());
    view_menu_->addAction(results_dock_->toggleViewAction());
}

void MainWindow::createCentralWidget() {
    central_tabs_ = new QTabWidget(this);

    // 3D вид
    view3d_ = new views::View3DWidget(this);
    view3d_->setWellModel(well_model_.get());
    view3d_->setProjectPointsModel(project_points_model_.get());
    view3d_->setShotPointsModel(shot_points_model_.get());
    central_tabs_->addTab(view3d_, tr("3D Аксонометрия"));

    // План
    plan_view_ = new views::PlanView(this);
    plan_view_->setWellModel(well_model_.get());
    plan_view_->setProjectPointsModel(project_points_model_.get());
    plan_view_->setShotPointsModel(shot_points_model_.get());
    central_tabs_->addTab(plan_view_, tr("План"));

    // Вертикальная проекция
    vertical_view_ = new views::VerticalView(this);
    vertical_view_->setWellModel(well_model_.get());
    vertical_view_->setProjectPointsModel(project_points_model_.get());
    central_tabs_->addTab(vertical_view_, tr("Вертикальная проекция"));

    setCentralWidget(central_tabs_);
}

void MainWindow::createStatusBar() {
    status_label_ = new QLabel(tr("Готов"));
    statusBar()->addWidget(status_label_, 1);

    progress_bar_ = new QProgressBar();
    progress_bar_->setVisible(false);
    progress_bar_->setMaximumWidth(200);
    statusBar()->addPermanentWidget(progress_bar_);
}

void MainWindow::closeEvent(QCloseEvent* event) {
    if (maybeSave()) {
        // Очистка recovery-данных при нормальном закрытии
        core::Settings::instance().clearRecoveryData();
        saveSettings();
        event->accept();
    } else {
        event->ignore();
    }
}

bool MainWindow::maybeSave() {
    if (!project_manager_->isDirty()) {
        return true;
    }

    QMessageBox::StandardButton ret = QMessageBox::warning(
        this, tr("Incline3D"),
        tr("Проект был изменён.\nСохранить изменения?"),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

    switch (ret) {
        case QMessageBox::Save:
            onSaveProject();
            return !project_manager_->isDirty();
        case QMessageBox::Discard:
            return true;
        case QMessageBox::Cancel:
        default:
            return false;
    }
}

void MainWindow::loadSettings() {
    auto& settings = core::Settings::instance();
    settings.load();

    // Восстановление геометрии окна
    if (!settings.mainWindowGeometry().isEmpty()) {
        restoreGeometry(settings.mainWindowGeometry());
    }
    if (!settings.mainWindowState().isEmpty()) {
        restoreState(settings.mainWindowState());
    }

    // Настройка путей
    if (!settings.inclprocPath().isEmpty()) {
        process_runner_->setInclprocPath(settings.inclprocPath());
    }
}

void MainWindow::saveSettings() {
    auto& settings = core::Settings::instance();

    settings.setMainWindowGeometry(saveGeometry());
    settings.setMainWindowState(saveState());

    settings.save();
}

void MainWindow::updateWindowTitle() {
    QString title = tr("Incline3D");

    QString project_name = project_manager_->projectData().name;
    if (project_name.isEmpty()) {
        project_name = tr("Новый проект");
    }
    title += " - " + project_name;

    if (project_manager_->isDirty()) {
        title += " *";
    }

    setWindowTitle(title);
}

void MainWindow::updateActions() {
    bool has_wells = well_model_->wellCount() > 0;
    bool has_selected_well = current_well_index_ >= 0;

    action_remove_well_->setEnabled(has_selected_well);
    action_save_file_->setEnabled(has_selected_well);
    action_process_well_->setEnabled(has_selected_well);
    action_process_all_->setEnabled(has_wells);
    action_proximity_->setEnabled(well_model_->wellCount() >= 2);
    action_offset_->setEnabled(well_model_->wellCount() >= 2);
}

// --- Слоты файлов ---

void MainWindow::onNewProject() {
    if (!maybeSave()) {
        return;
    }

    project_manager_->newProject();
    well_model_->clear();
    project_points_model_->clear();
    shot_points_model_->clear();
    measurements_model_->clearWell();
    results_model_->clearWell();
    current_well_index_ = -1;

    updateActions();
    status_label_->setText(tr("Создан новый проект"));
}

void MainWindow::onOpenProject() {
    if (!maybeSave()) {
        return;
    }

    auto& settings = core::Settings::instance();
    QString path = QFileDialog::getOpenFileName(
        this, tr("Открыть проект"),
        settings.lastProjectDirectory(),
        core::ProjectManager::getProjectFileFilter());

    if (path.isEmpty()) {
        return;
    }

    if (project_manager_->loadProject(path)) {
        settings.setLastProjectDirectory(QFileInfo(path).absolutePath());
        settings.addRecentProject(path);
        updateRecentProjectsMenu();

        // Синхронизация моделей
        project_points_model_->setPoints(project_manager_->projectData().project_points);
        shot_points_model_->setPoints(project_manager_->projectData().shot_points);

        current_well_index_ = -1;
        updateActions();
        status_label_->setText(tr("Проект загружен: %1").arg(path));
    } else {
        QMessageBox::critical(this, tr("Ошибка"),
                              tr("Не удалось загрузить проект"));
    }
}

void MainWindow::onSaveProject() {
    if (project_manager_->projectFilePath().isEmpty()) {
        onSaveProjectAs();
    } else {
        // Синхронизация данных
        project_manager_->projectData().project_points = project_points_model_->points();
        project_manager_->projectData().shot_points = shot_points_model_->points();

        if (project_manager_->saveProject()) {
            status_label_->setText(tr("Проект сохранён"));
        }
    }
}

void MainWindow::onSaveProjectAs() {
    auto& settings = core::Settings::instance();
    QString path = QFileDialog::getSaveFileName(
        this, tr("Сохранить проект как"),
        settings.lastProjectDirectory(),
        core::ProjectManager::getProjectFileFilter());

    if (path.isEmpty()) {
        return;
    }

    if (!path.endsWith(".inclproj", Qt::CaseInsensitive)) {
        path += ".inclproj";
    }

    // Синхронизация данных
    project_manager_->projectData().project_points = project_points_model_->points();
    project_manager_->projectData().shot_points = shot_points_model_->points();

    if (project_manager_->saveProject(path)) {
        settings.setLastProjectDirectory(QFileInfo(path).absolutePath());
        settings.addRecentProject(path);
        updateRecentProjectsMenu();
        status_label_->setText(tr("Проект сохранён: %1").arg(path));
    }
}

void MainWindow::onOpenFile() {
    auto& settings = core::Settings::instance();
    QString path = QFileDialog::getOpenFileName(
        this, tr("Открыть файл данных"),
        settings.lastOpenDirectory(),
        core::FileIO::getOpenFileFilter());

    if (path.isEmpty()) {
        return;
    }

    auto result = file_io_->loadWell(path);
    if (result.success && result.well) {
        result.well->display_color = settings.defaultWellColor();
        result.well->line_width = settings.defaultLineWidth();
        result.well->params = settings.defaultCalculationParams();

        project_manager_->addWell(result.well);
        well_model_->addWell(result.well);

        settings.setLastOpenDirectory(QFileInfo(path).absolutePath());
        settings.addRecentFile(path);
        updateRecentFilesMenu();

        status_label_->setText(tr("Загружена скважина: %1")
            .arg(QString::fromStdString(result.well->metadata.well_name)));

        updateActions();

        // Обновление видов
        if (view3d_) view3d_->update();
        if (plan_view_) plan_view_->update();
        if (vertical_view_) vertical_view_->update();
    } else {
        QMessageBox::critical(this, tr("Ошибка"),
                              tr("Не удалось загрузить файл:\n%1").arg(result.error_message));
    }
}

void MainWindow::onSaveFile() {
    if (current_well_index_ < 0) {
        return;
    }

    auto well = well_model_->wellAt(current_well_index_);
    if (!well) {
        return;
    }

    auto& settings = core::Settings::instance();
    QString path = QFileDialog::getSaveFileName(
        this, tr("Сохранить данные скважины"),
        settings.lastOpenDirectory(),
        core::FileIO::getSaveFileFilter());

    if (path.isEmpty()) {
        return;
    }

    auto result = file_io_->saveWell(path, *well);
    if (result.success) {
        well->source_file_path = path.toStdString();
        well->modified = false;
        well_model_->updateWell(current_well_index_);
        status_label_->setText(tr("Данные сохранены: %1").arg(path));
    } else {
        QMessageBox::critical(this, tr("Ошибка"),
                              tr("Не удалось сохранить файл:\n%1").arg(result.error_message));
    }
}

void MainWindow::onExportProject() {
    QString dir = QFileDialog::getExistingDirectory(
        this, tr("Выберите каталог для экспорта"),
        core::Settings::instance().lastProjectDirectory());

    if (dir.isEmpty()) {
        return;
    }

    if (project_manager_->exportProject(dir)) {
        status_label_->setText(tr("Проект экспортирован в: %1").arg(dir));
    } else {
        QMessageBox::warning(this, tr("Предупреждение"),
                             tr("Экспорт завершён с ошибками"));
    }
}

void MainWindow::onRecentFileTriggered() {
    QAction* action = qobject_cast<QAction*>(sender());
    if (!action) return;

    QString path = action->data().toString();
    auto result = file_io_->loadWell(path);
    if (result.success && result.well) {
        auto& settings = core::Settings::instance();
        result.well->display_color = settings.defaultWellColor();
        result.well->params = settings.defaultCalculationParams();

        project_manager_->addWell(result.well);
        well_model_->addWell(result.well);
        updateActions();

        if (view3d_) view3d_->update();
        if (plan_view_) plan_view_->update();
        if (vertical_view_) vertical_view_->update();
    }
}

void MainWindow::onRecentProjectTriggered() {
    QAction* action = qobject_cast<QAction*>(sender());
    if (!action) return;

    if (!maybeSave()) return;

    QString path = action->data().toString();
    if (project_manager_->loadProject(path)) {
        project_points_model_->setPoints(project_manager_->projectData().project_points);
        shot_points_model_->setPoints(project_manager_->projectData().shot_points);
        current_well_index_ = -1;
        updateActions();
    }
}

void MainWindow::updateRecentFilesMenu() {
    recent_files_menu_->clear();
    auto recent = core::Settings::instance().recentFiles();
    for (const auto& path : recent) {
        QAction* action = recent_files_menu_->addAction(path);
        action->setData(path);
        connect(action, &QAction::triggered, this, &MainWindow::onRecentFileTriggered);
    }
    recent_files_menu_->setEnabled(!recent.isEmpty());
}

void MainWindow::updateRecentProjectsMenu() {
    recent_projects_menu_->clear();
    auto recent = core::Settings::instance().recentProjects();
    for (const auto& path : recent) {
        QAction* action = recent_projects_menu_->addAction(path);
        action->setData(path);
        connect(action, &QAction::triggered, this, &MainWindow::onRecentProjectTriggered);
    }
    recent_projects_menu_->setEnabled(!recent.isEmpty());
}

// --- Слоты редактирования ---

void MainWindow::onAddWell() {
    onOpenFile();
}

void MainWindow::onRemoveWell() {
    if (current_well_index_ < 0) {
        return;
    }

    auto well = well_model_->wellAt(current_well_index_);
    if (!well) return;

    QMessageBox::StandardButton ret = QMessageBox::question(
        this, tr("Удаление скважины"),
        tr("Удалить скважину \"%1\" из проекта?")
            .arg(QString::fromStdString(well->metadata.well_name)),
        QMessageBox::Yes | QMessageBox::No);

    if (ret == QMessageBox::Yes) {
        project_manager_->removeWell(current_well_index_);
        well_model_->removeWell(current_well_index_);
        current_well_index_ = -1;
        measurements_model_->clearWell();
        results_model_->clearWell();
        updateActions();

        if (view3d_) view3d_->update();
        if (plan_view_) plan_view_->update();
        if (vertical_view_) vertical_view_->update();
    }
}

void MainWindow::onAddProjectPoint() {
    models::ProjectPoint pt;
    pt.name = tr("Новая точка").toStdString();
    project_points_model_->addPoint(pt);
    project_manager_->setDirty(true);

    if (view3d_) view3d_->update();
    if (plan_view_) plan_view_->update();
}

void MainWindow::onRemoveProjectPoint() {
    // TODO: получить выделенную строку из project_points_dock_
}

void MainWindow::onAddShotPoint() {
    models::ShotPoint pt;
    pt.name = tr("ПВ").toStdString();
    shot_points_model_->addPoint(pt);
    project_manager_->setDirty(true);

    if (view3d_) view3d_->update();
    if (plan_view_) plan_view_->update();
}

void MainWindow::onRemoveShotPoint() {
    // TODO: получить выделенную строку из shot_points_dock_
}

// --- Слоты обработки ---

void MainWindow::onProcessWell() {
    if (current_well_index_ < 0) {
        QMessageBox::information(this, tr("Обработка"),
                                 tr("Выберите скважину для обработки"));
        return;
    }

    auto well = well_model_->wellAt(current_well_index_);
    if (!well || well->measurements.empty()) {
        QMessageBox::warning(this, tr("Обработка"),
                             tr("У скважины нет исходных данных для обработки"));
        return;
    }

    ProcessDialog dialog(well, process_runner_.get(), this);
    if (dialog.exec() == QDialog::Accepted) {
        well_model_->updateWell(current_well_index_);
        results_model_->refresh();
        project_manager_->setDirty(true);

        if (view3d_) view3d_->update();
        if (plan_view_) plan_view_->update();
        if (vertical_view_) vertical_view_->update();

        status_label_->setText(tr("Обработка завершена"));
    }
}

void MainWindow::onProcessAllWells() {
    int processed = 0;
    int errors = 0;

    for (int i = 0; i < well_model_->wellCount(); ++i) {
        auto well = well_model_->wellAt(i);
        if (!well || well->measurements.empty()) {
            continue;
        }

        // TODO: реализовать пакетную обработку через inclproc
        ++processed;
    }

    status_label_->setText(tr("Обработано скважин: %1, ошибок: %2").arg(processed).arg(errors));
}

void MainWindow::onProximityAnalysis() {
    if (well_model_->wellCount() < 2) {
        QMessageBox::information(this, tr("Анализ сближения"),
                                 tr("Для анализа сближения необходимо минимум две скважины"));
        return;
    }

    ProximityDialog dialog(well_model_.get(), process_runner_.get(), this);
    dialog.exec();
}

void MainWindow::onOffsetAnalysis() {
    if (well_model_->wellCount() < 2) {
        QMessageBox::information(this, tr("Расчёт отхода"),
                                 tr("Для расчёта отхода необходимо минимум две скважины"));
        return;
    }

    OffsetDialog dialog(well_model_.get(), process_runner_.get(), this);
    dialog.exec();
}

// --- Слоты вида ---

void MainWindow::onView3D() {
    central_tabs_->setCurrentWidget(view3d_);
}

void MainWindow::onViewPlan() {
    central_tabs_->setCurrentWidget(plan_view_);
}

void MainWindow::onViewVertical() {
    central_tabs_->setCurrentWidget(vertical_view_);
}

void MainWindow::onResetView() {
    if (view3d_) view3d_->resetView();
    if (plan_view_) plan_view_->resetView();
    if (vertical_view_) vertical_view_->resetView();
}

void MainWindow::onViewOptions() {
    ViewOptionsDialog dialog(this);

    // Заполнение текущими настройками
    ViewOptions opts;
    opts.show_grid = view3d_ ? view3d_->showGrid() : true;
    opts.show_labels = view3d_ ? view3d_->showLabels() : true;
    // Можно расширить для других настроек
    dialog.setOptions(opts);

    connect(&dialog, &ViewOptionsDialog::optionsChanged,
            this, [this](const ViewOptions& opts) {
                // Применение настроек к видам
                if (view3d_) {
                    view3d_->setShowGrid(opts.show_grid);
                    view3d_->setShowLabels(opts.show_labels);
                    view3d_->update();
                }
                if (plan_view_) {
                    plan_view_->setShowGrid(opts.show_grid);
                    plan_view_->setShowLabels(opts.show_labels);
                    plan_view_->setGridStep(opts.grid_step);
                    plan_view_->refresh();
                }
                if (vertical_view_) {
                    vertical_view_->setShowGrid(opts.show_grid);
                    vertical_view_->setShowLabels(opts.show_labels);
                    vertical_view_->setGridStep(opts.grid_step);
                    vertical_view_->refresh();
                }
            });

    if (dialog.exec() == QDialog::Accepted) {
        status_label_->setText(tr("Настройки отображения применены"));
    }
}

void MainWindow::onExportImage() {
    QWidget* currentView = central_tabs_->currentWidget();
    if (!currentView) return;

    ExportImageDialog dialog(this);
    dialog.setCaptureWidget(currentView);

    if (dialog.exec() == QDialog::Accepted) {
        if (dialog.exportedToFile()) {
            status_label_->setText(tr("Изображение сохранено: %1").arg(dialog.selectedPath()));
        }
    }
}

void MainWindow::onCopyToClipboard() {
    QWidget* currentView = central_tabs_->currentWidget();
    if (!currentView) return;

    QImage image = currentView->grab().toImage();
    QApplication::clipboard()->setImage(image);
    status_label_->setText(tr("Изображение скопировано в буфер обмена"));
}

// --- Отчёты ---

void MainWindow::onEditReportHeader() {
    ReportHeaderDialog dialog(this);

    // Заполнение из проекта если есть данные
    ReportHeader header;
    if (!project_manager_->projectData().name.isEmpty()) {
        header.well_name = project_manager_->projectData().name;
    }

    // Если есть выбранная скважина, заполняем из неё
    if (current_well_index_ >= 0) {
        auto well = well_model_->wellAt(current_well_index_);
        if (well) {
            header.field_name = QString::fromStdString(well->metadata.field_name);
            header.well_pad = QString::fromStdString(well->metadata.well_pad);
            header.well_name = QString::fromStdString(well->metadata.well_name);
        }
    }

    dialog.setHeader(header);

    if (dialog.exec() == QDialog::Accepted) {
        // Сохранение шапки в настройки проекта
        // TODO: добавить поле для хранения шапки в ProjectData
        status_label_->setText(tr("Шапка отчёта обновлена"));
        project_manager_->setDirty(true);
    }
}

void MainWindow::onExportReport() {
    if (current_well_index_ < 0) {
        QMessageBox::information(this, tr("Экспорт отчёта"),
                                 tr("Выберите скважину для экспорта отчёта"));
        return;
    }

    auto well = well_model_->wellAt(current_well_index_);
    if (!well || well->results.empty()) {
        QMessageBox::warning(this, tr("Экспорт отчёта"),
                             tr("У выбранной скважины нет результатов обработки"));
        return;
    }

    QString path = QFileDialog::getSaveFileName(
        this, tr("Экспорт отчёта"),
        core::Settings::instance().lastOpenDirectory(),
        tr("CSV файлы (*.csv);;Текстовые файлы (*.txt)"));

    if (path.isEmpty()) return;

    // Экспорт результатов в CSV
    auto result = file_io_->saveWell(path, *well);
    if (result.success) {
        status_label_->setText(tr("Отчёт экспортирован: %1").arg(path));
    } else {
        QMessageBox::critical(this, tr("Ошибка"),
                              tr("Не удалось экспортировать отчёт:\n%1").arg(result.error_message));
    }
}

// --- Исходные данные ---

void MainWindow::onManualInput() {
    // Создание новой скважины с ручным вводом данных
    auto well = std::make_shared<models::WellData>();
    auto& settings = core::Settings::instance();
    well->display_color = settings.defaultWellColor();
    well->line_width = settings.defaultLineWidth();
    well->params = settings.defaultCalculationParams();

    ManualInputDialog dialog(well, this);
    if (dialog.exec() == QDialog::Accepted) {
        project_manager_->addWell(well);
        well_model_->addWell(well);
        project_manager_->setDirty(true);

        status_label_->setText(tr("Добавлена скважина: %1")
            .arg(QString::fromStdString(well->metadata.well_name)));

        updateActions();

        if (view3d_) view3d_->update();
        if (plan_view_) plan_view_->update();
        if (vertical_view_) vertical_view_->update();
    }
}

void MainWindow::onImportLas() {
    auto& settings = core::Settings::instance();
    QString path = QFileDialog::getOpenFileName(
        this, tr("Импорт из LAS"),
        settings.lastOpenDirectory(),
        tr("Файлы LAS (*.las *.LAS);;Все файлы (*)"));

    if (path.isEmpty()) {
        return;
    }

    ImportLasDialog dialog(path, this);
    if (dialog.exec() == QDialog::Accepted) {
        auto well = dialog.wellData();
        if (well) {
            well->display_color = settings.defaultWellColor();
            well->line_width = settings.defaultLineWidth();
            well->params = settings.defaultCalculationParams();
            well->source_file_path = path.toStdString();

            project_manager_->addWell(well);
            well_model_->addWell(well);
            project_manager_->setDirty(true);

            settings.setLastOpenDirectory(QFileInfo(path).absolutePath());
            settings.addRecentFile(path);
            updateRecentFilesMenu();

            status_label_->setText(tr("Импортирован LAS: %1")
                .arg(QString::fromStdString(well->metadata.well_name)));

            updateActions();

            if (view3d_) view3d_->update();
            if (plan_view_) plan_view_->update();
            if (vertical_view_) vertical_view_->update();
        }
    }
}

void MainWindow::onImportZak() {
    auto& settings = core::Settings::instance();
    QString path = QFileDialog::getOpenFileName(
        this, tr("Импорт из ЗАК"),
        settings.lastOpenDirectory(),
        tr("Файлы ЗАК (*.zak *.txt *.csv);;Все файлы (*)"));

    if (path.isEmpty()) {
        return;
    }

    ImportZakDialog dialog(path, this);
    if (dialog.exec() == QDialog::Accepted) {
        auto well = dialog.wellData();
        if (well) {
            well->display_color = settings.defaultWellColor();
            well->line_width = settings.defaultLineWidth();
            well->params = settings.defaultCalculationParams();
            well->source_file_path = path.toStdString();

            project_manager_->addWell(well);
            well_model_->addWell(well);
            project_manager_->setDirty(true);

            settings.setLastOpenDirectory(QFileInfo(path).absolutePath());
            settings.addRecentFile(path);
            updateRecentFilesMenu();

            status_label_->setText(tr("Импортирован ЗАК: %1")
                .arg(QString::fromStdString(well->metadata.well_name)));

            updateActions();

            if (view3d_) view3d_->update();
            if (plan_view_) plan_view_->update();
            if (vertical_view_) vertical_view_->update();
        }
    }
}

// --- Заключение ---

void MainWindow::onConclusion() {
    if (current_well_index_ < 0) {
        QMessageBox::information(this, tr("Заключение"),
                                 tr("Выберите скважину для формирования заключения"));
        return;
    }

    auto well = well_model_->wellAt(current_well_index_);
    if (!well) {
        return;
    }

    if (well->results.empty()) {
        QMessageBox::warning(this, tr("Заключение"),
                             tr("У скважины нет результатов обработки.\n"
                                "Сначала выполните обработку (F5)."));
        return;
    }

    // Получение проектных точек из модели
    std::vector<models::ProjectPoint> project_points;
    if (project_points_model_) {
        project_points = project_points_model_->points();
    }

    ConclusionDialog dialog(well, project_points, this);
    dialog.exec();
}

// --- Настройки вертикальной проекции ---

void MainWindow::onVerticalSettings() {
    VerticalSettingsDialog dialog(this);

    // Заполнение текущими настройками вертикальной проекции
    if (vertical_view_) {
        VerticalProjectionSettings settings;
        settings.show_grid = vertical_view_->showGrid();
        settings.show_depth_labels = vertical_view_->showLabels();
        settings.grid_step = vertical_view_->gridStep();
        // Можно расширить при необходимости
        dialog.setSettings(settings);
    }

    connect(&dialog, &VerticalSettingsDialog::settingsChanged,
            this, [this](const VerticalProjectionSettings& s) {
                if (vertical_view_) {
                    vertical_view_->setShowGrid(s.show_grid);
                    vertical_view_->setShowLabels(s.show_depth_labels);
                    vertical_view_->setGridStep(s.grid_step);
                    // Применение остальных настроек
                    vertical_view_->refresh();
                }
            });

    if (dialog.exec() == QDialog::Accepted) {
        status_label_->setText(tr("Настройки вертикальной проекции применены"));
    }
}

// --- Восстановление сессии ---

void MainWindow::checkRecovery() {
    auto& settings = core::Settings::instance();

    if (!settings.crashRecoveryEnabled()) {
        return;
    }

    QString recoveryPath = settings.recoveryProjectPath();
    if (!recoveryPath.isEmpty() && QFile::exists(recoveryPath)) {
        QMessageBox::StandardButton ret = QMessageBox::question(
            this, tr("Восстановление"),
            tr("Обнаружены несохранённые данные предыдущей сессии.\n"
               "Восстановить последнее состояние проекта?"),
            QMessageBox::Yes | QMessageBox::No);

        if (ret == QMessageBox::Yes) {
            if (project_manager_->loadProject(recoveryPath)) {
                project_points_model_->setPoints(project_manager_->projectData().project_points);
                shot_points_model_->setPoints(project_manager_->projectData().shot_points);
                status_label_->setText(tr("Сессия восстановлена"));
                LOG_INFO(tr("Сессия восстановлена из: %1").arg(recoveryPath));
            }
        }
    }

    settings.clearRecoveryData();
}

// --- Настройки и справка ---

void MainWindow::onSettings() {
    SettingsDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        // Применение настроек
        auto& settings = core::Settings::instance();
        process_runner_->setInclprocPath(settings.inclprocPath());

        if (settings.autoSaveEnabled()) {
            auto_save_timer_->start(settings.autoSaveIntervalMinutes() * 60 * 1000);
        } else {
            auto_save_timer_->stop();
        }
    }
}

void MainWindow::onAbout() {
    AboutDialog dialog(this);
    dialog.exec();
}

// --- Внутренние слоты ---

void MainWindow::onWellSelected(int index) {
    current_well_index_ = index;
    updateActions();

    auto well = well_model_->wellAt(index);
    if (well) {
        measurements_model_->setWell(well);
        results_model_->setWell(well);
        status_label_->setText(tr("Выбрана скважина: %1")
            .arg(QString::fromStdString(well->metadata.well_name)));
    } else {
        measurements_model_->clearWell();
        results_model_->clearWell();
    }
}

void MainWindow::onProcessFinished(bool success, const QString& message) {
    progress_bar_->setVisible(false);

    if (success) {
        status_label_->setText(tr("Обработка завершена успешно"));
    } else {
        status_label_->setText(tr("Ошибка обработки: %1").arg(message));
    }
}

void MainWindow::onAutoSave() {
    auto& settings = core::Settings::instance();

    // Синхронизация данных
    project_manager_->projectData().project_points = project_points_model_->points();
    project_manager_->projectData().shot_points = shot_points_model_->points();

    if (project_manager_->isDirty()) {
        if (!project_manager_->projectFilePath().isEmpty()) {
            // Сохранение в файл проекта
            if (project_manager_->saveProject()) {
                LOG_INFO(tr("Автосохранение выполнено"));
            }
        } else if (settings.crashRecoveryEnabled()) {
            // Сохранение во временный файл для восстановления
            QString recoveryDir = QStandardPaths::writableLocation(
                QStandardPaths::AppDataLocation);
            QDir().mkpath(recoveryDir);
            QString recoveryPath = recoveryDir + "/recovery.inclproj";

            if (project_manager_->saveProject(recoveryPath)) {
                settings.setRecoveryProjectPath(recoveryPath);
                settings.save();
                LOG_INFO(tr("Резервное сохранение для восстановления выполнено"));
            }
        }
    }
}

// --- Публичные методы для командной строки ---

void MainWindow::openProject(const QString& path) {
    if (path.isEmpty()) return;

    if (project_manager_->loadProject(path)) {
        auto& settings = core::Settings::instance();
        settings.setLastProjectDirectory(QFileInfo(path).absolutePath());
        settings.addRecentProject(path);
        updateRecentProjectsMenu();

        project_points_model_->setPoints(project_manager_->projectData().project_points);
        shot_points_model_->setPoints(project_manager_->projectData().shot_points);

        current_well_index_ = -1;
        updateActions();
        status_label_->setText(tr("Проект загружен: %1").arg(path));

        LOG_INFO(tr("Проект открыт из командной строки: %1").arg(path));
    } else {
        QMessageBox::critical(this, tr("Ошибка"),
                              tr("Не удалось загрузить проект:\n%1").arg(path));
    }
}

void MainWindow::openWellFile(const QString& path) {
    if (path.isEmpty()) return;

    auto result = file_io_->loadWell(path);
    if (result.success && result.well) {
        auto& settings = core::Settings::instance();
        result.well->display_color = settings.defaultWellColor();
        result.well->line_width = settings.defaultLineWidth();
        result.well->params = settings.defaultCalculationParams();

        project_manager_->addWell(result.well);
        well_model_->addWell(result.well);

        settings.setLastOpenDirectory(QFileInfo(path).absolutePath());
        settings.addRecentFile(path);
        updateRecentFilesMenu();

        status_label_->setText(tr("Загружена скважина: %1")
            .arg(QString::fromStdString(result.well->metadata.well_name)));

        updateActions();

        if (view3d_) view3d_->update();
        if (plan_view_) plan_view_->update();
        if (vertical_view_) vertical_view_->update();

        LOG_INFO(tr("Файл открыт из командной строки: %1").arg(path));
    } else {
        QMessageBox::critical(this, tr("Ошибка"),
                              tr("Не удалось загрузить файл:\n%1").arg(result.error_message));
    }
}

}  // namespace incline3d::ui
