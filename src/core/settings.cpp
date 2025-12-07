#include "core/settings.h"

#include <QCoreApplication>
#include <QDir>
#include <QSettings>
#include <QStandardPaths>

namespace incline3d::core {

Settings& Settings::instance() {
    static Settings instance;
    return instance;
}

void Settings::load() {
    QSettings s("PrimeGeo", "Incline3D");

    // Пути
    inclproc_path_ = s.value("paths/inclproc", "").toString();
    last_open_dir_ = s.value("paths/lastOpenDir",
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString();
    last_project_dir_ = s.value("paths/lastProjectDir", last_open_dir_).toString();
    recent_files_ = s.value("paths/recentFiles").toStringList();
    recent_projects_ = s.value("paths/recentProjects").toStringList();

    // Параметры расчёта
    s.beginGroup("calculation");
    default_params_.method = models::string_to_method(
        s.value("method", "mincurv").toString().toStdString());
    default_params_.magnetic_declination_deg = s.value("declination", 0.0).toDouble();
    default_params_.meridian_convergence_deg = s.value("meridian", 0.0).toDouble();
    default_params_.intensity_interval_m = s.value("intensityInterval", 30.0).toDouble();
    default_params_.error_depth_m = s.value("errorDepth", 0.1).toDouble();
    default_params_.error_inclination_deg = s.value("errorIncl", 0.1).toDouble();
    default_params_.error_azimuth_deg = s.value("errorAzim", 0.1).toDouble();
    default_params_.intensity_threshold_deg = s.value("intensityThreshold", 0.0).toDouble();
    default_params_.delta_depth_warning_m = s.value("deltaDepthWarning", 0.0).toDouble();
    default_params_.interpolation_step_m = s.value("interpStep", 0.0).toDouble();
    default_params_.use_last_azimuth = s.value("useLastAzimuth", true).toBool();
    default_params_.interpolate_missing_azimuths = s.value("interpAzimuths", true).toBool();
    default_params_.unwrap_azimuths = s.value("unwrapAzimuths", true).toBool();
    default_params_.smooth_intensity = s.value("smoothIntensity", false).toBool();
    default_params_.sngf_mode = s.value("sngfMode", false).toBool();
    default_params_.sngf_min_angle_deg = s.value("sngfMinAngle", 5.0).toDouble();
    default_params_.quality_check = s.value("qualityCheck", false).toBool();
    default_params_.max_angle_deviation_deg = s.value("maxAngleDeviation", 5.0).toDouble();
    default_params_.max_azimuth_deviation_deg = s.value("maxAzimuthDeviation", 10.0).toDouble();
    s.endGroup();

    // Визуализация
    s.beginGroup("visualization");
    default_well_color_ = s.value("defaultWellColor", QColor(Qt::blue)).value<QColor>();
    default_line_width_ = s.value("defaultLineWidth", 2).toInt();
    background_3d_ = s.value("background3D", QColor(Qt::white)).value<QColor>();
    grid_color_ = s.value("gridColor", QColor(Qt::lightGray)).value<QColor>();
    show_grid_ = s.value("showGrid", true).toBool();
    show_depth_labels_ = s.value("showDepthLabels", true).toBool();
    depth_label_step_ = s.value("depthLabelStep", 100.0).toDouble();
    s.endGroup();

    // Главное окно
    main_window_geometry_ = s.value("mainWindow/geometry").toByteArray();
    main_window_state_ = s.value("mainWindow/state").toByteArray();

    // Логирование
    QString default_log = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                          + "/incline3d.log";
    log_file_path_ = s.value("logging/filePath", default_log).toString();
    log_max_size_kb_ = s.value("logging/maxSizeKb", 1024).toInt();

    // Автосохранение
    auto_save_enabled_ = s.value("autoSave/enabled", true).toBool();
    auto_save_interval_minutes_ = s.value("autoSave/intervalMinutes", 5).toInt();

    // Единицы углов
    int unit = s.value("display/angleUnit", 0).toInt();
    angle_display_unit_ = static_cast<models::AngleUnit>(unit);

    // Восстановление сессии
    last_session_project_ = s.value("session/lastProject", "").toString();
    crash_recovery_enabled_ = s.value("session/crashRecoveryEnabled", true).toBool();
    recovery_project_path_ = s.value("session/recoveryProject", "").toString();
}

void Settings::save() {
    QSettings s("PrimeGeo", "Incline3D");

    // Пути
    s.setValue("paths/inclproc", inclproc_path_);
    s.setValue("paths/lastOpenDir", last_open_dir_);
    s.setValue("paths/lastProjectDir", last_project_dir_);
    s.setValue("paths/recentFiles", recent_files_);
    s.setValue("paths/recentProjects", recent_projects_);

    // Параметры расчёта
    s.beginGroup("calculation");
    s.setValue("method", QString::fromStdString(
        models::method_to_string(default_params_.method)));
    s.setValue("declination", default_params_.magnetic_declination_deg);
    s.setValue("meridian", default_params_.meridian_convergence_deg);
    s.setValue("intensityInterval", default_params_.intensity_interval_m);
    s.setValue("errorDepth", default_params_.error_depth_m);
    s.setValue("errorIncl", default_params_.error_inclination_deg);
    s.setValue("errorAzim", default_params_.error_azimuth_deg);
    s.setValue("intensityThreshold", default_params_.intensity_threshold_deg);
    s.setValue("deltaDepthWarning", default_params_.delta_depth_warning_m);
    s.setValue("interpStep", default_params_.interpolation_step_m);
    s.setValue("useLastAzimuth", default_params_.use_last_azimuth);
    s.setValue("interpAzimuths", default_params_.interpolate_missing_azimuths);
    s.setValue("unwrapAzimuths", default_params_.unwrap_azimuths);
    s.setValue("smoothIntensity", default_params_.smooth_intensity);
    s.setValue("sngfMode", default_params_.sngf_mode);
    s.setValue("sngfMinAngle", default_params_.sngf_min_angle_deg);
    s.setValue("qualityCheck", default_params_.quality_check);
    s.setValue("maxAngleDeviation", default_params_.max_angle_deviation_deg);
    s.setValue("maxAzimuthDeviation", default_params_.max_azimuth_deviation_deg);
    s.endGroup();

    // Визуализация
    s.beginGroup("visualization");
    s.setValue("defaultWellColor", default_well_color_);
    s.setValue("defaultLineWidth", default_line_width_);
    s.setValue("background3D", background_3d_);
    s.setValue("gridColor", grid_color_);
    s.setValue("showGrid", show_grid_);
    s.setValue("showDepthLabels", show_depth_labels_);
    s.setValue("depthLabelStep", depth_label_step_);
    s.endGroup();

    // Главное окно
    s.setValue("mainWindow/geometry", main_window_geometry_);
    s.setValue("mainWindow/state", main_window_state_);

    // Логирование
    s.setValue("logging/filePath", log_file_path_);
    s.setValue("logging/maxSizeKb", log_max_size_kb_);

    // Автосохранение
    s.setValue("autoSave/enabled", auto_save_enabled_);
    s.setValue("autoSave/intervalMinutes", auto_save_interval_minutes_);

    // Единицы углов
    s.setValue("display/angleUnit", static_cast<int>(angle_display_unit_));

    // Восстановление сессии
    s.setValue("session/lastProject", last_session_project_);
    s.setValue("session/crashRecoveryEnabled", crash_recovery_enabled_);
    s.setValue("session/recoveryProject", recovery_project_path_);
}

// Геттеры и сеттеры
QString Settings::inclprocPath() const { return inclproc_path_; }
void Settings::setInclprocPath(const QString& path) { inclproc_path_ = path; }

QString Settings::lastOpenDirectory() const { return last_open_dir_; }
void Settings::setLastOpenDirectory(const QString& dir) { last_open_dir_ = dir; }

QString Settings::lastProjectDirectory() const { return last_project_dir_; }
void Settings::setLastProjectDirectory(const QString& dir) { last_project_dir_ = dir; }

QStringList Settings::recentFiles() const { return recent_files_; }
void Settings::addRecentFile(const QString& path) {
    recent_files_.removeAll(path);
    recent_files_.prepend(path);
    while (recent_files_.size() > MAX_RECENT_FILES) {
        recent_files_.removeLast();
    }
}
void Settings::clearRecentFiles() { recent_files_.clear(); }

QStringList Settings::recentProjects() const { return recent_projects_; }
void Settings::addRecentProject(const QString& path) {
    recent_projects_.removeAll(path);
    recent_projects_.prepend(path);
    while (recent_projects_.size() > MAX_RECENT_FILES) {
        recent_projects_.removeLast();
    }
}
void Settings::clearRecentProjects() { recent_projects_.clear(); }

models::CalculationParams Settings::defaultCalculationParams() const { return default_params_; }
void Settings::setDefaultCalculationParams(const models::CalculationParams& params) {
    default_params_ = params;
}

QColor Settings::defaultWellColor() const { return default_well_color_; }
void Settings::setDefaultWellColor(const QColor& color) { default_well_color_ = color; }

int Settings::defaultLineWidth() const { return default_line_width_; }
void Settings::setDefaultLineWidth(int width) { default_line_width_ = width; }

QColor Settings::backgroundColor3D() const { return background_3d_; }
void Settings::setBackgroundColor3D(const QColor& color) { background_3d_ = color; }

QColor Settings::gridColor() const { return grid_color_; }
void Settings::setGridColor(const QColor& color) { grid_color_ = color; }

bool Settings::showGrid() const { return show_grid_; }
void Settings::setShowGrid(bool show) { show_grid_ = show; }

bool Settings::showDepthLabels() const { return show_depth_labels_; }
void Settings::setShowDepthLabels(bool show) { show_depth_labels_ = show; }

double Settings::depthLabelStep() const { return depth_label_step_; }
void Settings::setDepthLabelStep(double step) { depth_label_step_ = step; }

QByteArray Settings::mainWindowGeometry() const { return main_window_geometry_; }
void Settings::setMainWindowGeometry(const QByteArray& geometry) { main_window_geometry_ = geometry; }

QByteArray Settings::mainWindowState() const { return main_window_state_; }
void Settings::setMainWindowState(const QByteArray& state) { main_window_state_ = state; }

QString Settings::logFilePath() const { return log_file_path_; }
void Settings::setLogFilePath(const QString& path) { log_file_path_ = path; }

int Settings::logMaxSizeKb() const { return log_max_size_kb_; }
void Settings::setLogMaxSizeKb(int size) { log_max_size_kb_ = size; }

bool Settings::autoSaveEnabled() const { return auto_save_enabled_; }
void Settings::setAutoSaveEnabled(bool enabled) { auto_save_enabled_ = enabled; }

int Settings::autoSaveIntervalMinutes() const { return auto_save_interval_minutes_; }
void Settings::setAutoSaveIntervalMinutes(int minutes) { auto_save_interval_minutes_ = minutes; }

models::AngleUnit Settings::angleDisplayUnit() const { return angle_display_unit_; }
void Settings::setAngleDisplayUnit(models::AngleUnit unit) { angle_display_unit_ = unit; }

QString Settings::lastSessionProject() const { return last_session_project_; }
void Settings::setLastSessionProject(const QString& path) { last_session_project_ = path; }

bool Settings::crashRecoveryEnabled() const { return crash_recovery_enabled_; }
void Settings::setCrashRecoveryEnabled(bool enabled) { crash_recovery_enabled_ = enabled; }

QString Settings::recoveryProjectPath() const { return recovery_project_path_; }
void Settings::setRecoveryProjectPath(const QString& path) { recovery_project_path_ = path; }

void Settings::clearRecoveryData() {
    recovery_project_path_.clear();
    QSettings s("PrimeGeo", "Incline3D");
    s.remove("session/recoveryProject");
}

}  // namespace incline3d::core
