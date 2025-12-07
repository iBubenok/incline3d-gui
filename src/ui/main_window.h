#pragma once

#include <QMainWindow>
#include <QTimer>
#include <memory>

// Forward declarations
class QTabWidget;
class QDockWidget;
class QLabel;
class QProgressBar;
class QMenu;
class QAction;
class QToolBar;

namespace incline3d {

namespace core {
class ProjectManager;
class InclineProcessRunner;
class FileIO;
}  // namespace core

namespace models {
class WellTableModel;
class ProjectPointsModel;
class ShotPointsModel;
class MeasurementsModel;
class ResultsModel;
}  // namespace models

namespace views {
class View3DWidget;
class PlanView;
class VerticalView;
}  // namespace views

namespace ui {

class WellsDock;
class ProjectPointsDock;
class ShotPointsDock;
class MeasurementsDock;
class ResultsDock;

/// Главное окно приложения Incline3D
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

    /// Открыть проект из файла (для командной строки)
    void openProject(const QString& path);

    /// Открыть файл данных скважины (для командной строки)
    void openWellFile(const QString& path);

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    // Файл
    void onNewProject();
    void checkRecovery();
    void onOpenProject();
    void onSaveProject();
    void onSaveProjectAs();
    void onOpenFile();
    void onSaveFile();
    void onExportProject();
    void onRecentFileTriggered();
    void onRecentProjectTriggered();

    // Редактирование
    void onAddWell();
    void onRemoveWell();
    void onAddProjectPoint();
    void onRemoveProjectPoint();
    void onAddShotPoint();
    void onRemoveShotPoint();

    // Обработка
    void onProcessWell();
    void onProcessAllWells();
    void onProximityAnalysis();
    void onOffsetAnalysis();

    // Вид
    void onView3D();
    void onViewPlan();
    void onViewVertical();
    void onResetView();
    void onViewOptions();
    void onExportImage();
    void onCopyToClipboard();

    // Исходные данные
    void onManualInput();
    void onImportLas();
    void onImportZak();

    // Отчёты
    void onEditReportHeader();
    void onExportReport();
    void onConclusion();

    // Вертикальная проекция
    void onVerticalSettings();

    // Настройки
    void onSettings();
    void onAbout();

    // Внутренние
    void onWellSelected(int index);
    void onProcessFinished(bool success, const QString& message);
    void onAutoSave();
    void updateWindowTitle();
    void updateRecentFilesMenu();
    void updateRecentProjectsMenu();

private:
    void setupUi();
    void createActions();
    void createMenus();
    void createToolBars();
    void createDockWidgets();
    void createCentralWidget();
    void createStatusBar();

    void loadSettings();
    void saveSettings();

    bool maybeSave();
    void updateActions();

    // Компоненты ядра
    std::unique_ptr<core::ProjectManager> project_manager_;
    std::unique_ptr<core::InclineProcessRunner> process_runner_;
    std::unique_ptr<core::FileIO> file_io_;

    // Модели данных
    std::unique_ptr<models::WellTableModel> well_model_;
    std::unique_ptr<models::ProjectPointsModel> project_points_model_;
    std::unique_ptr<models::ShotPointsModel> shot_points_model_;
    std::unique_ptr<models::MeasurementsModel> measurements_model_;
    std::unique_ptr<models::ResultsModel> results_model_;

    // Виды визуализации
    views::View3DWidget* view3d_{nullptr};
    views::PlanView* plan_view_{nullptr};
    views::VerticalView* vertical_view_{nullptr};

    // Доки
    WellsDock* wells_dock_{nullptr};
    ProjectPointsDock* project_points_dock_{nullptr};
    ShotPointsDock* shot_points_dock_{nullptr};
    MeasurementsDock* measurements_dock_{nullptr};
    ResultsDock* results_dock_{nullptr};

    // Центральный виджет
    QTabWidget* central_tabs_{nullptr};

    // Статус-бар
    QLabel* status_label_{nullptr};
    QProgressBar* progress_bar_{nullptr};

    // Меню
    QMenu* file_menu_{nullptr};
    QMenu* recent_files_menu_{nullptr};
    QMenu* recent_projects_menu_{nullptr};
    QMenu* edit_menu_{nullptr};
    QMenu* data_menu_{nullptr};          // Меню "Исходные данные"
    QMenu* process_menu_{nullptr};
    QMenu* view_menu_{nullptr};
    QMenu* report_menu_{nullptr};
    QMenu* help_menu_{nullptr};

    // Тулбар
    QToolBar* main_toolbar_{nullptr};

    // Действия - Файл
    QAction* action_new_project_{nullptr};
    QAction* action_open_project_{nullptr};
    QAction* action_save_project_{nullptr};
    QAction* action_save_project_as_{nullptr};
    QAction* action_open_file_{nullptr};
    QAction* action_save_file_{nullptr};
    QAction* action_export_project_{nullptr};
    QAction* action_exit_{nullptr};

    // Действия - Редактирование
    QAction* action_add_well_{nullptr};
    QAction* action_remove_well_{nullptr};
    QAction* action_add_project_point_{nullptr};
    QAction* action_remove_project_point_{nullptr};
    QAction* action_add_shot_point_{nullptr};
    QAction* action_remove_shot_point_{nullptr};

    // Действия - Обработка
    QAction* action_process_well_{nullptr};
    QAction* action_process_all_{nullptr};
    QAction* action_proximity_{nullptr};
    QAction* action_offset_{nullptr};

    // Действия - Вид
    QAction* action_view_3d_{nullptr};
    QAction* action_view_plan_{nullptr};
    QAction* action_view_vertical_{nullptr};
    QAction* action_reset_view_{nullptr};
    QAction* action_view_options_{nullptr};
    QAction* action_export_image_{nullptr};
    QAction* action_copy_to_clipboard_{nullptr};

    // Действия - Исходные данные
    QAction* action_manual_input_{nullptr};
    QAction* action_import_las_{nullptr};
    QAction* action_import_zak_{nullptr};

    // Действия - Отчёты
    QAction* action_edit_report_header_{nullptr};
    QAction* action_export_report_{nullptr};
    QAction* action_conclusion_{nullptr};

    // Действия - Вид (дополнительные)
    QAction* action_vertical_settings_{nullptr};

    // Действия - Настройки
    QAction* action_settings_{nullptr};
    QAction* action_about_{nullptr};

    // Таймер автосохранения
    QTimer* auto_save_timer_{nullptr};

    // Текущая выбранная скважина
    int current_well_index_{-1};
};

}  // namespace ui
}  // namespace incline3d
