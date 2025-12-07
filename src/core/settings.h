#pragma once

#include <QColor>
#include <QFont>
#include <QString>
#include <QStringList>

#include "models/well_data.h"

namespace incline3d::core {

/// Настройки приложения (сохраняются между сессиями)
class Settings {
public:
    static Settings& instance();

    /// Загрузить настройки
    void load();

    /// Сохранить настройки
    void save();

    // --- Пути ---
    QString inclprocPath() const;
    void setInclprocPath(const QString& path);

    QString lastOpenDirectory() const;
    void setLastOpenDirectory(const QString& dir);

    QString lastProjectDirectory() const;
    void setLastProjectDirectory(const QString& dir);

    QStringList recentFiles() const;
    void addRecentFile(const QString& path);
    void clearRecentFiles();

    QStringList recentProjects() const;
    void addRecentProject(const QString& path);
    void clearRecentProjects();

    // --- Параметры расчёта по умолчанию ---
    models::CalculationParams defaultCalculationParams() const;
    void setDefaultCalculationParams(const models::CalculationParams& params);

    // --- Визуализация ---
    QColor defaultWellColor() const;
    void setDefaultWellColor(const QColor& color);

    int defaultLineWidth() const;
    void setDefaultLineWidth(int width);

    QColor backgroundColor3D() const;
    void setBackgroundColor3D(const QColor& color);

    QColor gridColor() const;
    void setGridColor(const QColor& color);

    bool showGrid() const;
    void setShowGrid(bool show);

    bool showDepthLabels() const;
    void setShowDepthLabels(bool show);

    double depthLabelStep() const;
    void setDepthLabelStep(double step);

    // --- Главное окно ---
    QByteArray mainWindowGeometry() const;
    void setMainWindowGeometry(const QByteArray& geometry);

    QByteArray mainWindowState() const;
    void setMainWindowState(const QByteArray& state);

    // --- Логирование ---
    QString logFilePath() const;
    void setLogFilePath(const QString& path);

    int logMaxSizeKb() const;
    void setLogMaxSizeKb(int size);

    // --- Автосохранение ---
    bool autoSaveEnabled() const;
    void setAutoSaveEnabled(bool enabled);

    int autoSaveIntervalMinutes() const;
    void setAutoSaveIntervalMinutes(int minutes);

    // --- Единицы углов ---
    models::AngleUnit angleDisplayUnit() const;
    void setAngleDisplayUnit(models::AngleUnit unit);

    // --- Восстановление сессии ---
    QString lastSessionProject() const;
    void setLastSessionProject(const QString& path);

    bool crashRecoveryEnabled() const;
    void setCrashRecoveryEnabled(bool enabled);

    QString recoveryProjectPath() const;
    void setRecoveryProjectPath(const QString& path);

    void clearRecoveryData();

private:
    Settings() = default;
    Settings(const Settings&) = delete;
    Settings& operator=(const Settings&) = delete;

    static constexpr int MAX_RECENT_FILES = 10;

    QString inclproc_path_;
    QString last_open_dir_;
    QString last_project_dir_;
    QStringList recent_files_;
    QStringList recent_projects_;

    models::CalculationParams default_params_;

    QColor default_well_color_{Qt::blue};
    int default_line_width_{2};
    QColor background_3d_{Qt::white};
    QColor grid_color_{Qt::lightGray};
    bool show_grid_{true};
    bool show_depth_labels_{true};
    double depth_label_step_{100.0};

    QByteArray main_window_geometry_;
    QByteArray main_window_state_;

    QString log_file_path_;
    int log_max_size_kb_{1024};

    bool auto_save_enabled_{true};
    int auto_save_interval_minutes_{5};

    models::AngleUnit angle_display_unit_{models::AngleUnit::kDecimalDegrees};

    QString last_session_project_;
    bool crash_recovery_enabled_{true};
    QString recovery_project_path_;
};

}  // namespace incline3d::core
